#include "MonteCarloEngine.hpp"
#include <random>
#include <algorithm>

std::vector<double> runMonteCarlo(DcfInputs base, int sims, unsigned seed) {
    std::mt19937 rng(seed);

    // Each driver becomes a distribution centered on the base case.
    // The standard deviations encode how uncertain you are about each input.
    // Tune these as you learn the company.
    std::normal_distribution<double> growthDist(base.revenueGrowth, 0.02);
    std::normal_distribution<double> marginDist(base.ebitMargin,   0.03);
    std::normal_distribution<double> waccDist(base.wacc,           0.01);
    std::normal_distribution<double> tgDist(base.terminalGrowth,   0.005);

    std::vector<double> values;
    values.reserve(static_cast<size_t>(sims));

    for (int i = 0; i < sims; ++i) {
        DcfInputs in = base;
        in.revenueGrowth  = growthDist(rng);
        in.ebitMargin     = std::clamp(marginDist(rng), 0.0, 0.9);
        in.terminalGrowth = tgDist(rng);
        // Keep WACC comfortably above terminal growth or the terminal value
        // formula blows up toward infinity.
        in.wacc = std::max(waccDist(rng), in.terminalGrowth + 0.02);

        values.push_back(intrinsicValuePerShare(in));
    }

    std::sort(values.begin(), values.end());
    return values;
}
