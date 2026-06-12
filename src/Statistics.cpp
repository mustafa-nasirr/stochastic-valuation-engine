#include "Statistics.hpp"
#include <numeric>
#include <cmath>

double mean(const std::vector<double>& v) {
    if (v.empty()) return 0.0;
    return std::accumulate(v.begin(), v.end(), 0.0) / static_cast<double>(v.size());
}

double stddev(const std::vector<double>& v) {
    if (v.size() < 2) return 0.0;
    double m = mean(v);
    double acc = 0.0;
    for (double x : v) acc += (x - m) * (x - m);
    return std::sqrt(acc / static_cast<double>(v.size() - 1));
}

double percentile(const std::vector<double>& sortedValues, double p) {
    if (sortedValues.empty()) return 0.0;
    if (p <= 0.0) return sortedValues.front();
    if (p >= 1.0) return sortedValues.back();
    double idx = p * static_cast<double>(sortedValues.size() - 1);
    size_t lo  = static_cast<size_t>(std::floor(idx));
    size_t hi  = static_cast<size_t>(std::ceil(idx));
    double frac = idx - static_cast<double>(lo);
    return sortedValues[lo] * (1.0 - frac) + sortedValues[hi] * frac;
}

double probAbove(const std::vector<double>& values, double threshold) {
    if (values.empty()) return 0.0;
    size_t count = 0;
    for (double x : values) if (x > threshold) ++count;
    return static_cast<double>(count) / static_cast<double>(values.size());
}
