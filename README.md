# Delta-Hedged Options Backtester

A **C++17** backtesting application for a strategy that sells at-the-money (ATM) European call options and maintains a **dynamic delta hedge** using the underlying stock.

The project uses the Black–Scholes model to calculate option prices and Greeks, estimates implied volatility from market quotes, tracks position cash flows, and exports trade results, daily profit and loss (PnL), and individual transactions to CSV files.

## Features

- Load historical stock prices and option quotes from CSV files.
- Calculate European call prices, delta, and vega using the Black–Scholes model.
- Estimate implied volatility with the Newton–Raphson method.
- Calculate annualized realized volatility from daily log returns.
- Open short ATM call positions and establish a delta-neutral stock hedge.
- Rebalance the hedge daily and close positions at expiration.
- Generate CSV reports for completed trades, daily PnL, and transactions.

## Requirements

- A compiler supporting **C++17**, such as `g++`.
- `make` (if building with the included `Makefile`).

No third-party libraries are required.

## Quick Start

1. Extract the project and open a terminal in its directory.
2. Provide the input files `stock.csv` and `options.csv` (**neither is included in the supplied archive**).
3. Build and run the backtester:

```bash
make
./backtest stock.csv options.csv
```

The program prints realized volatility and a summary of trading results to the terminal. It also creates or overwrites three report files:

- `trades.csv` — results for each completed option series.
- `pnl.csv` — daily realized, unrealized, and total PnL.
- `transactions.csv` — a log of option and stock transactions.

If your input files are named `stock.csv` and `options.csv` and are located in the working directory, you can simply run:

```bash
./backtest
```

### Command-Line Arguments

All arguments are positional and optional:

```bash
./backtest [stock.csv] [options.csv] [trades.csv] [pnl.csv] [transactions.csv]
```

The first two arguments specify input files; the last three specify output report paths. Omitted arguments use the filenames shown above.

> **Important:** `make clean` removes not only the object files and the `backtest` executable, but also `trades.csv`, `pnl.csv`, and `transactions.csv`. Back up any reports you want to keep before cleaning the build.

## Input Data Formats

### `stock.csv`

A CSV file with a header row and the stock price in the **second column**. The parser ignores the first column. Trading days are determined by the order of data rows, starting at index `0`.

Example:

```csv
day,price
0,100.00
1,100.90
2,102.57
```

### `options.csv`

A CSV file with a header row and **five columns** in the following order:

| Column | Description |
|---|---|
| `day` | Zero-based trading day index. |
| `expiry_day` | Trading day index of the option's expiration. |
| `strike` | Option strike price. |
| `unused` | A field that is ignored by the current implementation. |
| `quoted_price` | Market option price per unit of the underlying stock. |

Example:

```csv
day,expiry_day,strike,unused,quoted_price
0,21,100.00,0,3.0015
1,21,100.00,0,3.1200
```

These examples illustrate the required file structure; they are **not** a complete dataset for a backtest. A realistic run requires stock prices and corresponding quotes for the option series over the relevant trading days. Day indices must be consistent across the two input files.

## Strategy Overview

The default parameters are defined in `main-2.cpp`:

| Parameter | Value | Purpose |
|---|---:|---|
| Risk-free rate `r` | `0.03` | Annual risk-free rate (3%) used by Black–Scholes. |
| `option_qty` | `-100` | Short position in 100 call contracts. |
| `series_length` | `21` | Number of trading days in each backtest series. |
| `multiplier` | `100` | Shares of underlying stock per option contract. |
| Trading days per year | `252` | Used for annualizing volatility and calculating time to expiration. |

### Trading Logic

1. **Select an option.** At the start of each 21-day period, select the nearest future expiration and the strike closest to the current stock price (ATM).
2. **Sell calls.** Open a short position in 100 call contracts and collect the option premium.
3. **Establish the initial hedge.** Calculate Black–Scholes delta and buy enough shares to offset the short option position's delta:

   ```text
   stock_qty = round(-option_qty × delta × multiplier)
   ```

4. **Rebalance daily.** On each trading day before expiration, retrieve the option quote, recalculate implied volatility and delta, and buy or sell stock to adjust the hedge.
5. **Close at expiration.** Settle the option at intrinsic value, `max(0, S_T − K)`, liquidate the stock hedge, and record the trade PnL.
6. **Write reports.** Record the trade summary, daily portfolio valuations, and individual option and stock transactions.

The PnL calculation incorporates option premiums, cash paid or received when trading stock, and the cost of closing the option. Purchases decrease the cash balance; sales increase it.

**Implementation detail:** The engine uses its configured `series_length` to determine each position's closing day. The expiration day stored in the selected option quote is used to identify the quote series. For consistent results, the input option expirations should align with the configured backtest periods.

## Project Structure

```text
.
├── main-2.cpp                  # Entry point, parameters, and backtest execution
├── MarketDataLoader.h/.cpp     # Stock and option market data loading
├── BlackScholes.h/.cpp         # European call pricing, delta, and vega
├── ImpliedVolSolver.h/.cpp     # Implied volatility estimation
├── VolatilityCalculator.h/.cpp # Realized volatility calculation
├── Position.h/.cpp             # Positions, hedges, cash flows, and PnL
├── TradingEngine.h/.cpp        # Trading strategy and rebalancing logic
├── Transaction.h               # Transaction data structure
├── Reporter.h/.cpp             # CSV report generation and console summary
├── Makefile                    # Build configuration
├── REPORT.md                   # Implementation notes (in Russian)
├── trades.csv                  # Sample backtest trade results
├── pnl.csv                     # Sample daily PnL
└── transactions.csv            # Sample transaction log
```

## Output Reports

| File | Main columns |
|---|---|
| `trades.csv` | `series`, `entry_day`, `expiry_day`, `strike`, `entry_spot`, `final_spot`, `entry_option_price`, `entry_iv`, `pnl`, `avg_delta` |
| `pnl.csv` | `day`, `realized_pnl`, `unrealized_pnl`, `total_pnl`, `spot`, `option_qty`, `stock_qty` |
| `transactions.csv` | `day`, `instrument`, `side`, `quantity`, `price`, `cash_flow`, `event` |

In `transactions.csv`, a positive `cash_flow` represents money received, while a negative value represents money spent. The recorded event types are:

- `OPEN_OPTION` — open the short call position.
- `OPEN_HEDGE` — establish the initial stock hedge.
- `REBAL_HEDGE` — adjust the stock hedge.
- `CLOSE_OPTION` — settle the option at expiration.
- `CLOSE_HEDGE` — close the stock hedge.

## Results from the Included Reports

The CSV reports supplied with the project contain the following results:

| Metric | Value |
|---|---:|
| Completed option series | 12 |
| Profitable series | 11 out of 12 |
| Series length | 21 trading days |
| Total PnL | **55,052.9250** |

These metrics were taken from the **existing CSV reports**, not reproduced by rerunning the backtester: the original market-data files (`stock.csv` and `options.csv`) are not included in the archive. The currency of the PnL depends on the currency used for the input prices.

## Model Assumptions and Limitations

- Only **European call options** are supported; dividends are not modeled.
- Strategy parameters are hard-coded in `main-2.cpp`; there is no separate configuration file.
- The strategy processes complete 21-day periods and rebalances daily, with no intraday trading.
- Commissions, slippage, bid–ask spreads, hedge financing costs, and other real-world trading expenses are not included.
- If a quote for the active option is unavailable on a given day, the entry quote is used as a fallback. If implied volatility cannot be recalculated, the last valid implied volatility is reused.
- The option data is expected to be compatible with the configured series length and expiration schedule.
- This project is intended for research and demonstration. Historical results are not a forecast of future performance.

## Building Without `make`

You can compile the project directly:

```bash
g++ -std=c++17 -O2 -Wall -Wextra *.cpp -o backtest
```

---

**Build status:** The project has been built successfully with `make`. Reproducing the backtest requires the missing input market-data CSV files.
