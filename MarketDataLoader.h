#pragma once

#include <map>
#include <string>
#include <vector>

struct OptionQuote {
    int    day;
    int    expiry_day;
    double strike;
    double quoted_price;
};

class MarketDataLoader {
public:
    MarketDataLoader();
    ~MarketDataLoader();

    bool loadStock  (const std::string& path);
    bool loadOptions(const std::string& path);

    int    numDays()           const;
    double stockPrice(int day) const;

    const std::vector<OptionQuote>& optionsOnDay(int day) const;
    OptionQuote nearestAtmOption(int day, double target_strike) const;
private:
    std::vector<double>                       m_stockPrices;
    std::map<int, std::vector<OptionQuote> >  m_optionsByDay;
    std::vector<OptionQuote>                  m_empty;
};

