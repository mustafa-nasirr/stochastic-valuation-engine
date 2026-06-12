#pragma once
#include <vector>

double mean(const std::vector<double>& v);
double stddev(const std::vector<double>& v);

// Expects a sorted vector. p is in [0, 1], e.g. 0.10 for the 10th percentile.
double percentile(const std::vector<double>& sortedValues, double p);

// Fraction of values strictly greater than the threshold. Works on any order.
double probAbove(const std::vector<double>& values, double threshold);
