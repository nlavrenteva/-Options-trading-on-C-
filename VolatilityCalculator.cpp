#include "VolatilityCalculator.h"
#include <cmath>

VolatilityCalculator::VolatilityCalculator() {
    
}

VolatilityCalculator::VolatilityCalculator(const VolatilityCalculator& other) : m_values(other.m_values) {
    
}

VolatilityCalculator::~VolatilityCalculator() {
    
}

void VolatilityCalculator::addPrice(double v) {
    m_values.push_back(v);
}

double VolatilityCalculator::mean() const {
    if (m_values.empty()) return 0.0;
    double sum = 0.0;
    for (std::size_t i = 0; i < m_values.size(); ++i) sum += m_values[i];
    return sum / m_values.size();
}

double VolatilityCalculator::stdDev() const {
    if (m_values.size() < 2) return 0.0;
    const double m = mean();
    double s = 0.0;
    for (std::size_t i = 0; i < m_values.size(); ++i) {
        const double d = m_values[i] - m;
        s += d * d;
    }
    return std::sqrt(s / (m_values.size() - 1));
}

double VolatilityCalculator::annualizedVol(int periods_per_year) const {
    return stdDev() * std::sqrt(static_cast<double>(periods_per_year));
}

std::size_t VolatilityCalculator::size() const {
    return m_values.size();
}
