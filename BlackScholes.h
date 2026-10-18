#pragma once

// European call pricing under Black-Scholes.
// Risk-free rate is fixed at construction; spot/strike/T/sigma vary per call.
// No dividends.
class BlackScholes {
public:
    explicit BlackScholes(double r);
    ~BlackScholes();

    double price(double S, double K, double T, double sigma) const;
    double delta(double S, double K, double T, double sigma) const;
    double vega (double S, double K, double T, double sigma) const;
private:
    double m_r;
    static double normCdf(double x);
    static double normPdf(double x);
};

