#pragma once

#include <string>
#include <vector>
#include "TradingEngine.h"

class Reporter {
public:
    Reporter();
    ~Reporter();

    bool writeTrades(const std::string& path,
                     const std::vector<TradeResult>& trades) const;

    bool writePnL(const std::string& path,
                  const std::vector<DailyPnL>& pnl) const;

    bool writeTransactions(const std::string& path,
                           const std::vector<Transaction>& txns) const;

    void printSummary(const std::vector<TradeResult>& trades) const;
};
