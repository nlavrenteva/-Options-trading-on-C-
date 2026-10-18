#include "Position.h"
#include <algorithm>

// =============================================================================
//  WARNING: this file contains the trickiest parts of the assignment.
//  Sign errors in cash flows cause 80% of all bugs. Re-read Position.h carefully.
//
//  Sign convention:
//    option_qty < 0  -> short the option (we receive premium, cash goes up)
//    stock_qty  > 0  -> long the underlying (we pay, cash goes down)
//
//  Cash flow on any trade:  cash -= signed_qty * price * scale
//    Sanity-check yourself: if signed_qty > 0 (buying), cash must DECREASE.
//    Hence the minus sign.
// =============================================================================

Position::Position()
    : m_option_qty(0.0),
      m_stock_qty(0.0),
      m_cash(0.0),
      m_strike(0.0),
      m_expiry_day(0),
      m_multiplier(1.0) {}

Position::~Position() {}

// Open a position: trade `option_qty` contracts at `option_price`,
// simultaneously hedge with `initial_stock_qty` shares at `stock_price`.
// `multiplier` is the number of shares represented by 1 contract
// (US equity options: 100).
//
// Cash flows:
//   - Option leg: cash -= option_qty * option_price * multiplier
//   - Hedge leg:  cash -= initial_stock_qty * stock_price  (no multiplier — shares are direct)
//
// Save strike, expiry_day, and multiplier into member fields — closeAtExpiry will need them.
void Position::open(double option_qty, double option_price,
                    double strike, int expiry_day,
                    double initial_stock_qty, double stock_price,
                    double multiplier) {
    m_option_qty = option_qty;
    m_stock_qty  = initial_stock_qty;
    m_strike     = strike;
    m_expiry_day = expiry_day;
    m_multiplier = multiplier;
    m_cash -= option_qty * option_price * multiplier;
    m_cash -= initial_stock_qty * stock_price;
}

// Rebalance the hedge: adjust stock holdings to `new_stock_qty`.
// diff = new_stock_qty - m_stock_qty   (can be positive or negative)
// Cash: cash -= diff * stock_price
// Update m_stock_qty.
void Position::rebalanceHedge(double new_stock_qty, double stock_price) {
    const double diff = new_stock_qty - m_stock_qty;
    m_cash -= diff * stock_price;
    m_stock_qty = new_stock_qty;
}

// Close the position at expiry.
//
// Option leg:
//   intrinsic = max(0, S_T - strike)
//   "Closing" the short means buying it back at intrinsic value.
//   Position change: m_option_qty -> 0, which means buying -m_option_qty units
//   (if m_option_qty was -100, we buy 100 to close).
//   Cash flow from closing the option: cash += m_option_qty * intrinsic * multiplier
//   (This is NOT a typo — think about the signs!  m_option_qty is negative,
//   intrinsic is positive, so the product is negative, and cash DECREASES.
//   Common antipattern: writing `cash -= m_option_qty * intrinsic * multiplier`
//   flips the sign and inflates PnL on ITM expiries.)
//
// Stock leg:
//   Sell all m_stock_qty at spot.
//   Cash flow: cash += m_stock_qty * stock_price
//
// After closing, zero both quantities (m_option_qty = 0, m_stock_qty = 0).
// Return the realized PnL — this is the total cash accumulated during the series.
// Convenient pattern: return m_cash and reset it to 0 so the next series
// starts clean.
double Position::closeAtExpiry(double stock_price) {
    const double intrinsic = std::max(0.0, stock_price - m_strike);
    m_cash += m_option_qty * intrinsic * m_multiplier;
    m_cash += m_stock_qty * stock_price;
    m_option_qty = 0.0;
    m_stock_qty  = 0.0;
    const double pnl = m_cash;
    m_cash = 0.0;
    return pnl;
}

// Mark-to-market value of the position at given market prices.
//   value = cash + option_qty * option_price * multiplier + stock_qty * stock_price
double Position::markToMarket(double stock_price, double option_price) const {
    return m_cash + m_option_qty * option_price * m_multiplier + m_stock_qty * stock_price;
}

// Trivial getters — one-liners, implement by analogy.
bool   Position::isOpen()    const {
    return m_option_qty != 0.0;
}
double Position::optionQty() const {
    return m_option_qty;
}
double Position::stockQty()  const {
    return m_stock_qty;
}
double Position::cash()      const {
    return m_cash;
}
double Position::strike()    const {
    return m_strike;
}
int    Position::expiryDay() const {
    return m_expiry_day;
}
double Position::multiplier()const {
    return m_multiplier;
}
