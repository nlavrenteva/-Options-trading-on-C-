#pragma once

#include "BlackScholes.h"

// Newton-Raphson implied volatility solver.
// Returns -1.0 if vega is too small (dead zone) or the iteration fails to converge.
class ImpliedVolSolver {
public:
    explicit ImpliedVolSolver(const BlackScholes& bs);
    ~ImpliedVolSolver();

    double solve(double market_price, double S, double K, double T,
                 double initial_guess = 0.20) const;
private:
    const BlackScholes& m_bs;
};

