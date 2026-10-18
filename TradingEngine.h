#pragma once

#include <vector>
#include "BlackScholes.h"
#include "ImpliedVolSolver.h"
#include "MarketDataLoader.h"
#include "Transaction.h"

// Per-trade result for reporting.
struct TradeResult {
    int    series_idx;
    int    entry_day;
    int    expiry_day;
    double strike;
    double entry_spot;
    double final_spot;
    double entry_option_price;
    double entry_iv;
    double pnl;
    double avg_delta;          // average BS delta over the life of the trade
};

// Daily PnL snapshot.
struct DailyPnL {
    int    day;
    double realized_pnl;
    double unrealized_pnl;
    double total_pnl;
    double spot;
    double option_qty;
    double stock_qty;
};

// Strategy:
//   - On the first day of every series, pick the option closest to ATM in the
//     front-month chain. Short `option_qty` contracts (negative number).
//     Hedge with delta * |option_qty| shares of underlying.
//   - On every subsequent day until (but not including) the expiry day:
//     re-extract implied vol from the current quote, recompute delta,
//     rebalance the stock hedge.
//   - On the expiry day: close at intrinsic, record PnL, start a new series.
class TradingEngine {
public:
    TradingEngine(const MarketDataLoader& md,
                  double r,
                  double option_qty,         // signed; negative = short
                  int    series_length,
                  double multiplier);        // shares per contract (e.g. 100)
    ~TradingEngine();

    // Run the strategy across all complete series in the data.
    // Populates trades + equity curve.
    void run();

    const std::vector<TradeResult>& trades() const;
    const std::vector<DailyPnL>&    dailyPnL() const;
    const std::vector<Transaction>& transactions() const;
private:
    const MarketDataLoader& m_md;
    double m_r;
    double m_option_qty;
    int    m_series_length;
    double m_multiplier;

    BlackScholes      m_bs;
    ImpliedVolSolver  m_iv;

    std::vector<TradeResult> m_trades;
    std::vector<DailyPnL>    m_pnl;
    std::vector<Transaction> m_txns;
};

