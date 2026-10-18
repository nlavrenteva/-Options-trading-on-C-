#include "Reporter.h"
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>

Reporter::Reporter() {
    
}
Reporter::~Reporter() {
    
}

bool Reporter::writeTrades(const std::string& path,
                           const std::vector<TradeResult>& trades) const {
    std::ofstream out(path, std::ios::trunc);
    if (!out) {
        std::cerr << "Reporter: cannot open '" << path << "' for writing\n";
        return false;
    }
    out << "series,entry_day,expiry_day,strike,entry_spot,final_spot,"
           "entry_option_price,entry_iv,pnl,avg_delta\n";
    out << std::fixed << std::setprecision(4);
    for (std::size_t i = 0; i < trades.size(); ++i) {
        const TradeResult& t = trades[i];
        out << t.series_idx        << ','
            << t.entry_day         << ','
            << t.expiry_day        << ','
            << t.strike            << ','
            << t.entry_spot        << ','
            << t.final_spot        << ','
            << t.entry_option_price<< ','
            << t.entry_iv          << ','
            << t.pnl               << ','
            << t.avg_delta         << '\n';
    }
    return true;
}

bool Reporter::writeTransactions(const std::string& path,
                                 const std::vector<Transaction>& txns) const {
    std::ofstream out(path, std::ios::trunc);
    if (!out) {
        std::cerr << "Reporter: cannot open '" << path << "' for writing\n";
        return false;
    }
    out << "day,instrument,side,quantity,price,cash_flow,event\n";
    out << std::fixed << std::setprecision(4);
    for (std::size_t i = 0; i < txns.size(); ++i) {
        const Transaction& t = txns[i];
        out << t.day        << ','
            << t.instrument << ','
            << t.side       << ','
            << t.quantity   << ','
            << t.price      << ','
            << t.cash_flow  << ','
            << t.event      << '\n';
    }
    return true;
}

bool Reporter::writePnL(const std::string& path,
                        const std::vector<DailyPnL>& pnl) const {
    std::ofstream out(path, std::ios::trunc);
    if (!out) {
        std::cerr << "Reporter: cannot open '" << path << "' for writing\n";
        return false;
    }
    out << "day,realized_pnl,unrealized_pnl,total_pnl,spot,option_qty,stock_qty\n";
    out << std::fixed << std::setprecision(4);
    for (std::size_t i = 0; i < pnl.size(); ++i) {
        const DailyPnL& p = pnl[i];
        out << p.day            << ','
            << p.realized_pnl   << ','
            << p.unrealized_pnl << ','
            << p.total_pnl      << ','
            << p.spot           << ','
            << p.option_qty     << ','
            << p.stock_qty      << '\n';
    }
    return true;
}

void Reporter::printSummary(const std::vector<TradeResult>& trades) const {
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "==== Per-trade PnL ====\n";
    std::cout << "series  entry  expiry  K       spot_in  spot_out  iv_in   pnl\n";
    double total = 0.0;
    double total_abs = 0.0;
    int wins = 0;
    for (std::size_t i = 0; i < trades.size(); ++i) {
        const TradeResult& t = trades[i];
        std::cout << "  " << std::setw(3) << t.series_idx
                  << "    " << std::setw(3) << t.entry_day
                  << "    " << std::setw(3) << t.expiry_day
                  << "  "   << std::setw(6) << t.strike
                  << "   "  << std::setw(7) << t.entry_spot
                  << "   "  << std::setw(7) << t.final_spot
                  << "  "   << std::setw(6) << t.entry_iv
                  << "   "  << std::setw(8) << t.pnl
                  << "\n";
        total     += t.pnl;
        total_abs += std::fabs(t.pnl);
        if (t.pnl > 0.0) ++wins;
    }
    std::cout << "==== Summary ====\n";
    std::cout << "trades        : " << trades.size() << "\n";
    std::cout << "winning trades: " << wins << " / " << trades.size() << "\n";
    std::cout << "total PnL     : " << total << "\n";
    std::cout << "avg PnL       : " << (trades.empty() ? 0.0 : total / trades.size()) << "\n";
}
