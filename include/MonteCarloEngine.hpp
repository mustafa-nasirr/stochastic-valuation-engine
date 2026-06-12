#pragma once
#include "DcfModel.hpp"
#include <vector>

// Runs the DCF many times, each time drawing the key assumptions from
// distributions centered on the base case. Returns the simulated
// intrinsic values per share, sorted ascending.
std::vector<double> runMonteCarlo(DcfInputs base, int sims, unsigned seed = 42);
