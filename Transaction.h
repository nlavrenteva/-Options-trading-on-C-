#pragma once

#include <string>

// One row of the per-operation audit log.
// cash_flow uses the sign convention: positive = cash received, negative = cash paid.
struct Transaction {
    int         day;
    std::string instrument;     // "STOCK" or "CALL_<strike>_E<expiry>"
    std::string side;           // "BUY" or "SELL"
    double      quantity;       // always non-negative; direction in `side`
    double      price;
    double      cash_flow;      // signed: + received, - paid
    std::string event;          // OPEN_OPTION / OPEN_HEDGE / REBAL_HEDGE / CLOSE_OPTION / CLOSE_HEDGE
};

