#include "ImpliedVolSolver.h"
#include <cmath>

// Newton-Raphson hyperparameters. You do not need to change these.
static const double TOL      = 1e-7;    // converged when |price - market| < TOL
static const int    MAX_ITER = 100;     // max iterations
static const double MIN_VEGA = 1e-6;    // below this, sigma is indistinguishable
static const double SIGMA_LO = 1e-4;    // lower clamp on sigma (avoid div by zero)
static const double SIGMA_HI = 5.0;     // upper clamp (500% vol — obvious outlier)

ImpliedVolSolver::ImpliedVolSolver(const BlackScholes& bs) : m_bs(bs) {}

ImpliedVolSolver::~ImpliedVolSolver() {}

// Newton-Raphson algorithm:
//   1. Start with sigma = initial_guess
//   2. On each iteration:
//      a. Compute BS price f(sigma) at the current sigma
//      b. diff = f(sigma) - market_price
//      c. If |diff| < TOL — converged, return sigma
//      d. Compute vega(sigma)
//      e. If vega < MIN_VEGA — point is insensitive to sigma, return -1.0
//      f. Update: sigma -= diff / vega
//      g. Clamp sigma to [SIGMA_LO, SIGMA_HI]
//   3. If MAX_ITER iterations didn't converge — return -1.0
//
// Return value of -1.0 is a sentinel meaning "couldn't extract IV".
// The caller must check for this and skip such quotes.
double ImpliedVolSolver::solve(double market_price, double S, double K, double T,
                               double initial_guess) const {
    double sigma = initial_guess;
    for (int i = 0; i < MAX_ITER; ++i) {
        const double diff = m_bs.price(S, K, T, sigma) - market_price;
        if (std::fabs(diff) < TOL) return sigma;
        const double v = m_bs.vega(S, K, T, sigma);
        if (v < MIN_VEGA) return -1.0;
        sigma -= diff / v;
        if (sigma < SIGMA_LO) sigma = SIGMA_LO;
        if (sigma > SIGMA_HI) sigma = SIGMA_HI;
    }
    return -1.0;
}
