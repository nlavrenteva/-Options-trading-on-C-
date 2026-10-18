#pragma once

// A delta-hedged option position. Quantities are floating-point to avoid
// integer rounding in the hedge (1 option = 1 unit of underlying).
//
// Conventions:
//   option_qty < 0  => short the option (premium received as positive cash)
//   stock_qty  > 0  => long the underlying
class Position {
public:
    Position();
    ~Position();

    // Open: trade option_qty contracts at option_price, hedge with initial_stock_qty shares.
    // multiplier scales option payoffs: 1 contract represents `multiplier` shares.
    void open(double option_qty, double option_price,
              double strike, int expiry_day,
              double initial_stock_qty, double stock_price,
              double multiplier);

    // Adjust stock holdings to a new target quantity.
    // Cash is updated for the buy/sell at the given stock price.
    void rebalanceHedge(double new_stock_qty, double stock_price);

    // Settle at expiry: buy back the option at intrinsic, sell all stock at S_T.
    // Returns the realized PnL of this trade (= cash after closing).
    double closeAtExpiry(double stock_price);

    // Mark-to-market value: cash + option_qty * option_price + stock_qty * stock_price.
    double markToMarket(double stock_price, double option_price) const;

    bool   isOpen()      const;
    double optionQty()   const;
    double stockQty()    const;
    double cash()        const;
    double strike()      const;
    int    expiryDay()   const;
    double multiplier()  const;
private:
    double m_option_qty;
    double m_stock_qty;
    double m_cash;
    double m_strike;
    int    m_expiry_day;
    double m_multiplier;
};
