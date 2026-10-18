#include <cmath>
#include <iostream>
#include <string>

#include "MarketDataLoader.h"
#include "Reporter.h"
#include "TradingEngine.h"
#include "VolatilityCalculator.h"

int main(int argc, const char* argv[]) {
    // Input / output paths. Default to files in the working directory.
    const std::string stock_path   = (argc > 1) ? argv[1] : "stock.csv";
    const std::string options_path = (argc > 2) ? argv[2] : "options.csv";
    const std::string trades_path  = (argc > 3) ? argv[3] : "trades.csv";
    const std::string pnl_path     = (argc > 4) ? argv[4] : "pnl.csv";
    const std::string txns_path    = (argc > 5) ? argv[5] : "transactions.csv";

    // -------------------------------------------------------------------------
    //  Step 1 — Load market data
    // -------------------------------------------------------------------------
    MarketDataLoader md;
    if (!md.loadStock(stock_path))   return 1;
    if (!md.loadOptions(options_path)) return 1;

    // -------------------------------------------------------------------------
    //  Step 2 — Compute realized volatility (sanity output)
    // -------------------------------------------------------------------------
    VolatilityCalculator vol;
    for (int d = 1; d < md.numDays(); ++d) {
        vol.addPrice(std::log(md.stockPrice(d) / md.stockPrice(d - 1)));
    }
    std::cout << "Realized vol: " << vol.annualizedVol(252) << "\n";

    // -------------------------------------------------------------------------
    //  Step 3 — Run the strategy
    // -------------------------------------------------------------------------
    const double r             = 0.03;
    const double option_qty    = -100.0;
    const int    series_length = 21;
    const double multiplier    = 100.0;

    TradingEngine engine(md, r, option_qty, series_length, multiplier);
    engine.run();

    // -------------------------------------------------------------------------
    //  Step 4 — Write results
    // -------------------------------------------------------------------------
    Reporter rep;
    rep.writeTrades(trades_path, engine.trades());
    rep.writePnL(pnl_path, engine.dailyPnL());
    rep.writeTransactions(txns_path, engine.transactions());
    rep.printSummary(engine.trades());

    return 0;
}
