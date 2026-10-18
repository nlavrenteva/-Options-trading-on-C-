#include "BlackScholes.h"
#include <algorithm>
#include <cmath>

BlackScholes::BlackScholes(double r) : m_r(r) {}

BlackScholes::~BlackScholes() {}

// Cumulative distribution function of the standard normal.
// Hint: use std::erf from <cmath>:
//   N(x) = 0.5 * (1 + erf(x / sqrt(2)))
double BlackScholes::normCdf(double x) {
    return 0.5 * (1.0 + std::erf(x / std::sqrt(2.0)));
}

// Probability density function of the standard normal.
//   n(x) = exp(-x^2 / 2) / sqrt(2*pi)
double BlackScholes::normPdf(double x) {
    return std::exp(-x * x / 2.0) / std::sqrt(2.0 * M_PI);
}

// Black-Scholes price of a European call.
// C = S * N(d1) - K * exp(-r*T) * N(d2)
// where:
//   d1 = (ln(S/K) + (r + sigma^2/2) * T) / (sigma * sqrt(T))
//   d2 = d1 - sigma * sqrt(T)
//
// Edge case: if T <= 0 or sigma <= 0, return max(0, S - K) (intrinsic value).
double BlackScholes::price(double S, double K, double T, double sigma) const {
    if (T <= 0.0 || sigma <= 0.0) return std::max(0.0, S - K);
    const double d1 = (std::log(S / K) + (m_r + sigma * sigma / 2.0) * T) / (sigma * std::sqrt(T));
    const double d2 = d1 - sigma * std::sqrt(T);
    return S * normCdf(d1) - K * std::exp(-m_r * T) * normCdf(d2);
}

// Delta of a call: dC/dS = N(d1).
// Edge case: if T <= 0 or sigma <= 0, return 1.0 if S > K else 0.0.
double BlackScholes::delta(double S, double K, double T, double sigma) const {
    if (T <= 0.0 || sigma <= 0.0) return (S > K) ? 1.0 : 0.0;
    const double d1 = (std::log(S / K) + (m_r + sigma * sigma / 2.0) * T) / (sigma * std::sqrt(T));
    return normCdf(d1);
}

// Vega: dC/dsigma = S * n(d1) * sqrt(T).
// Edge case: if T <= 0 or sigma <= 0, return 0.
double BlackScholes::vega(double S, double K, double T, double sigma) const {
    if (T <= 0.0 || sigma <= 0.0) return 0.0;
    const double d1 = (std::log(S / K) + (m_r + sigma * sigma / 2.0) * T) / (sigma * std::sqrt(T));
    return S * normPdf(d1) * std::sqrt(T);
}
