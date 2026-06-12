#include "DcfModel.hpp"
#include <cmath>

double intrinsicValuePerShare(const DcfInputs& in) {
    double revenue = in.baseRevenue;
    double enterpriseValue = 0.0;
    double lastFcf = 0.0;

    for (int t = 1; t <= in.projectionYears; ++t) {
        double prevRevenue = revenue;
        revenue *= (1.0 + in.revenueGrowth);

        double ebit     = revenue * in.ebitMargin;
        double nopat    = ebit * (1.0 - in.taxRate);          // net operating profit after tax
        double da       = revenue * in.daPctRevenue;
        double capex    = revenue * in.capexPctRevenue;
        double deltaNwc = (revenue - prevRevenue) * in.nwcPctRevenue;

        double fcf = nopat + da - capex - deltaNwc;           // unlevered free cash flow
        enterpriseValue += fcf / std::pow(1.0 + in.wacc, t);
        lastFcf = fcf;
    }

    // Gordon growth terminal value, then discount it back to today.
    double terminalValue = lastFcf * (1.0 + in.terminalGrowth)
                         / (in.wacc - in.terminalGrowth);
    enterpriseValue += terminalValue
                     / std::pow(1.0 + in.wacc, in.projectionYears);

    // Simplified bridge: equity = enterprise value - net debt.
    // A full bridge also adjusts for minority interest, preferred, and investments.
    double equityValue = enterpriseValue - in.netDebt;
    return equityValue / in.sharesOutstanding;
}
