#pragma once

// All monetary inputs are in the same units (millions works well).
// Per-share output then comes out in dollars when shares are in millions.
struct DcfInputs {
    double baseRevenue = 0.0;        // most recent annual revenue
    int    projectionYears = 5;
    double revenueGrowth = 0.0;      // annual, e.g. 0.06
    double ebitMargin = 0.0;         // operating margin, e.g. 0.315
    double taxRate = 0.21;
    double daPctRevenue = 0.05;      // depreciation & amortization as % of revenue
    double capexPctRevenue = 0.05;   // capital expenditure as % of revenue
    double nwcPctRevenue = 0.02;     // change in net working capital as % of revenue growth
    double wacc = 0.09;              // weighted average cost of capital
    double terminalGrowth = 0.025;   // perpetual growth rate
    double netDebt = 0.0;            // negative means net cash
    double sharesOutstanding = 1.0;
};

// Returns the intrinsic value per share under a single set of assumptions.
double intrinsicValuePerShare(const DcfInputs& in);
