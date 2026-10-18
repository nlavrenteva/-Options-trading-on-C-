#include "TradingEngine.h"
#include "Position.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>

TradingEngine::TradingEngine(const MarketDataLoader& md,
                             double r,
                             double option_qty,
                             int    series_length,
                             double multiplier)
    : m_md(md),
      m_r(r),
      m_option_qty(option_qty),
      m_series_length(series_length),
      m_multiplier(multiplier),
      m_bs(r),
      m_iv(m_bs) {}

TradingEngine::~TradingEngine() {}

// =============================================================================
//  Helpers — feel free to add your own.
// =============================================================================

// Find a specific quote in the chain by (strike, expiry). Returns an OptionQuote
// with day = -1 if not found (sentinel).
static OptionQuote findQuote(const std::vector<OptionQuote>& quotes,
                             double strike, int expiry_day) {
    OptionQuote bad;
    bad.day = -1;
    for (std::size_t i = 0; i < quotes.size(); ++i) {
        if (quotes[i].strike == strike && quotes[i].expiry_day == expiry_day) {
            return quotes[i];
        }
    }
    return bad;
}

// Format an instrument name for the transaction log: "CALL_<strike>_E<expiry_day>".
// Use std::ostringstream + std::fixed + setprecision(2).
static std::string optionName(double strike, int expiry_day) {
    std::ostringstream os;
    os << std::fixed << std::setprecision(2);
    os << "CALL_" << strike << "_E" << expiry_day;
    return os.str();
}

// Append a single transaction to the log. signed_qty is signed (positive = BUY).
// cash_flow = -signed_qty * price * multiplier  (same sign rule as in Position!)
static void recordTxn(std::vector<Transaction>& log,
                      int day, const std::string& instr,
                      double signed_qty, double price, double multiplier,
                      const std::string& event) {
    if (signed_qty == 0.0) return;
    Transaction t;
    t.day        = day;
    t.instrument = instr;
    t.side       = (signed_qty > 0.0) ? "BUY" : "SELL";
    t.quantity   = std::fabs(signed_qty);
    t.price      = price;
    t.cash_flow  = -signed_qty * price * multiplier;
    t.event      = event;
    log.push_back(t);
}

// =============================================================================
//  Main strategy loop.
// =============================================================================
void TradingEngine::run() {
    m_trades.clear();
    m_pnl.clear();
    m_txns.clear();

    const int total_days = m_md.numDays();
    const int num_series = (total_days - 1) / m_series_length;
    double cumulative_pnl = 0.0;
    Position pos;

    for (int s = 0; s < num_series; ++s) {
        const int entry_day  = s * m_series_length;
        const int expiry_day = entry_day + m_series_length;

        // --- ENTRY ---
        const double S0 = m_md.stockPrice(entry_day);
        OptionQuote q = m_md.nearestAtmOption(entry_day, S0);
        const double strike = q.strike;
        const int    exp    = q.expiry_day;
        const double T0 = (expiry_day - entry_day) / 252.0;

        const double iv0 = m_iv.solve(q.quoted_price, S0, strike, T0);
        if (iv0 < 0.0) continue;  // couldn't extract IV — skip the series

        double last_iv = iv0;
        const double delta0 = m_bs.delta(S0, strike, T0, iv0);
        const double hedge0 = std::round(-m_option_qty * delta0 * m_multiplier);

        pos.open(m_option_qty, q.quoted_price, strike, exp,
                 hedge0, S0, m_multiplier);

        const std::string instr = optionName(strike, exp);
        recordTxn(m_txns, entry_day, instr, m_option_qty, q.quoted_price,
                  m_multiplier, "OPEN_OPTION");
        recordTxn(m_txns, entry_day, "STOCK", hedge0, S0, 1.0, "OPEN_HEDGE");

        double delta_sum = delta0;
        int    delta_cnt = 1;

        {
            DailyPnL p;
            p.day            = entry_day;
            p.spot           = S0;
            p.realized_pnl   = cumulative_pnl;
            p.unrealized_pnl = pos.markToMarket(S0, q.quoted_price);
            p.total_pnl      = p.realized_pnl + p.unrealized_pnl;
            p.option_qty     = pos.optionQty();
            p.stock_qty      = pos.stockQty();
            m_pnl.push_back(p);
        }

        // --- HOLD ---
        for (int d = entry_day + 1; d < expiry_day; ++d) {
            const double S = m_md.stockPrice(d);
            const std::vector<OptionQuote>& chain = m_md.optionsOnDay(d);
            OptionQuote cur = findQuote(chain, strike, exp);
            const double price = (cur.day == -1) ? q.quoted_price : cur.quoted_price;
            const double T = (expiry_day - d) / 252.0;

            double iv = m_iv.solve(price, S, strike, T, last_iv);
            if (iv < 0.0) iv = last_iv;
            last_iv = iv;

            const double delta = m_bs.delta(S, strike, T, iv);
            const double target = std::round(-m_option_qty * delta * m_multiplier);

            const double prev_stock = pos.stockQty();
            pos.rebalanceHedge(target, S);
            recordTxn(m_txns, d, "STOCK", target - prev_stock, S, 1.0, "REBAL_HEDGE");

            delta_sum += delta;
            ++delta_cnt;

            DailyPnL p;
            p.day            = d;
            p.spot           = S;
            p.realized_pnl   = cumulative_pnl;
            p.unrealized_pnl = pos.markToMarket(S, price);
            p.total_pnl      = p.realized_pnl + p.unrealized_pnl;
            p.option_qty     = pos.optionQty();
            p.stock_qty      = pos.stockQty();
            m_pnl.push_back(p);
        }

        // --- EXPIRY ---
        const double final_spot = m_md.stockPrice(expiry_day);
        const double intrinsic  = std::max(0.0, final_spot - strike);
        recordTxn(m_txns, expiry_day, instr, -pos.optionQty(), intrinsic,
                  m_multiplier, "CLOSE_OPTION");
        recordTxn(m_txns, expiry_day, "STOCK", -pos.stockQty(), final_spot,
                  1.0, "CLOSE_HEDGE");

        const double trade_pnl = pos.closeAtExpiry(final_spot);
        cumulative_pnl += trade_pnl;

        TradeResult tr;
        tr.series_idx         = s;
        tr.entry_day          = entry_day;
        tr.expiry_day         = expiry_day;
        tr.strike             = strike;
        tr.entry_spot         = S0;
        tr.final_spot         = final_spot;
        tr.entry_option_price = q.quoted_price;
        tr.entry_iv           = iv0;
        tr.pnl                = trade_pnl;
        tr.avg_delta          = delta_sum / delta_cnt;
        m_trades.push_back(tr);

        if (s == num_series - 1) {
            DailyPnL p;
            p.day            = expiry_day;
            p.spot           = final_spot;
            p.realized_pnl   = cumulative_pnl;
            p.unrealized_pnl = pos.markToMarket(final_spot, intrinsic);
            p.total_pnl      = p.realized_pnl + p.unrealized_pnl;
            p.option_qty     = pos.optionQty();
            p.stock_qty      = pos.stockQty();
            m_pnl.push_back(p);
        }
    }
}

// Getters — trivial one-liners.
const std::vector<TradeResult>& TradingEngine::trades() const {
    return m_trades;
}

const std::vector<DailyPnL>& TradingEngine::dailyPnL() const {
    return m_pnl;
}

const std::vector<Transaction>& TradingEngine::transactions() const {
    return m_txns;
}
