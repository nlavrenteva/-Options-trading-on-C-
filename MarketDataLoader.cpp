#include "MarketDataLoader.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

MarketDataLoader::MarketDataLoader() {}
MarketDataLoader::~MarketDataLoader() {}

bool MarketDataLoader::loadStock(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        std::cerr << "MarketDataLoader: cannot open '" << path << "'\n";
        return false;
    }
    std::string line;
    int lineno = 0;
    while (std::getline(in, line)) {
        ++lineno;
        if (lineno == 1 || line.empty()) continue;
        std::stringstream ss(line);
        std::string field;
        std::getline(ss, field, ',');
        std::getline(ss, field, ',');
        m_stockPrices.push_back(std::stod(field));
    }
    std::cerr << "MarketDataLoader: loaded " << m_stockPrices.size() << " stock prices\n";
    return !m_stockPrices.empty();
}

bool MarketDataLoader::loadOptions(const std::string& path) {
    std::ifstream in(path);
    if (!in) {
        std::cerr << "MarketDataLoader: cannot open '" << path << "'\n";
        return false;
    }
    std::string line;
    int lineno = 0;
    int total  = 0;
    while (std::getline(in, line)) {
        ++lineno;
        if (lineno == 1 || line.empty()) continue;
        std::stringstream ss(line);
        std::string field;
        std::vector<std::string> row;
        while (std::getline(ss, field, ',')) row.push_back(field);
        if (row.size() != 5) continue;

        OptionQuote q;
        q.day          = std::stoi(row[0]);
        q.expiry_day   = std::stoi(row[1]);
        q.strike       = std::stod(row[2]);
        q.quoted_price = std::stod(row[4]);
        m_optionsByDay[q.day].push_back(q);
        ++total;
    }
    std::cerr << "MarketDataLoader: loaded " << total << " option quotes\n";
    return total > 0;
}

int MarketDataLoader::numDays() const {
    return static_cast<int>(m_stockPrices.size());
}

double MarketDataLoader::stockPrice(int day) const {
    return m_stockPrices.at(day);
}

const std::vector<OptionQuote>& MarketDataLoader::optionsOnDay(int day) const {
    std::map<int, std::vector<OptionQuote> >::const_iterator it = m_optionsByDay.find(day);
    if (it == m_optionsByDay.end()) {
        return m_empty;
    }
    return it->second;
}

OptionQuote MarketDataLoader::nearestAtmOption(int day, double target_strike) const {
    const std::vector<OptionQuote>& all = optionsOnDay(day);

    int front_expiry = 1 << 30;
    for (std::size_t i = 0; i < all.size(); ++i) {
        if (all[i].expiry_day > day && all[i].expiry_day < front_expiry) {
            front_expiry = all[i].expiry_day;
        }
    }

    OptionQuote best;
    best.day = -1;
    double best_dist = 1e18;
    for (std::size_t i = 0; i < all.size(); ++i) {
        if (all[i].expiry_day != front_expiry) continue;
        const double d = std::fabs(all[i].strike - target_strike);
        if (d < best_dist) {
            best_dist = d;
            best      = all[i];
        }
    }
    return best;
}
