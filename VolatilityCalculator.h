#pragma once

#include <vector>

// Lecture-3 class with one addition: annualizedVol() for log-return inputs.
// Students should feed log-returns (log S_t / S_{t-1}) via addPrice() and then
// call annualizedVol(252) to get the realized volatility.
class VolatilityCalculator {
public:
    VolatilityCalculator();
    VolatilityCalculator(const VolatilityCalculator& other);
    ~VolatilityCalculator();

    void   addPrice(double v);                  // generic: adds any sample
    double mean()    const;
    double stdDev()  const;
    double annualizedVol(int periods_per_year) const;  // stdDev * sqrt(periods)
    std::size_t size() const;
private:
    std::vector<double> m_values;
};
