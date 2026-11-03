#include "CompsAnalyzer.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iostream>

static double medianOf(std::vector<double> v) {
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    size_t n = v.size();
    if (n % 2 == 1) return v[n / 2];
    return 0.5 * (v[n / 2 - 1] + v[n / 2]);
}

// Drops values whose modified z-score exceeds `cutoff` (both tails).
// Needs 3+ points and a nonzero MAD, otherwise returns the data unchanged.
static std::vector<double> filterOutliersMAD(const std::vector<double>& v,
                                             double cutoff = 3.5,
                                             size_t* removed = nullptr) {
    if (removed) *removed = 0;
    if (v.size() < 3) return v;

    double med = medianOf(v);
    std::vector<double> dev;
    dev.reserve(v.size());
    for (double x : v) dev.push_back(std::fabs(x - med));
    double mad = medianOf(dev);
    if (mad == 0.0) return v;  // can't score; skip filtering

    std::vector<double> kept;
    for (double x : v) {
        double z = 0.6745 * std::fabs(x - med) / mad;
        if (z <= cutoff) kept.push_back(x);
    }
    if (removed) *removed = v.size() - kept.size();
    return kept;
}

std::vector<Peer> loadPeersCsv(const std::string& path) {
    std::vector<Peer> peers;
    std::ifstream file(path);
    if (!file.is_open()) return peers;  // comps are optional

    std::string line;
    bool firstLine = true;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        if (firstLine) { firstLine = false; continue; }  // skip header

        std::stringstream ss(line);
        std::string field;
        Peer p;
        int col = 0;
        while (std::getline(ss, field, ',')) {
            try {
                if      (col == 0) p.ticker = field;
                else if (col == 1) p.enterpriseValue = std::stod(field);
                else if (col == 2) p.ebitda = std::stod(field);
                else if (col == 3) p.revenue = std::stod(field);
            } catch (const std::exception&) { /* skip bad cell */ }
            ++col;
        }
        if (!p.ticker.empty()) peers.push_back(p);
    }
    return peers;
}

CompsResult analyzeComps(const std::vector<Peer>& peers,
                         double targetEbitda, double targetRevenue) {
    CompsResult r;
    if (peers.empty()) return r;

    std::vector<double> evEbitda, evRevenue;
    for (const auto& p : peers) {
        if (p.ebitda  > 0.0) evEbitda.push_back(p.enterpriseValue / p.ebitda);
        if (p.revenue > 0.0) evRevenue.push_back(p.enterpriseValue / p.revenue);
    }

    // Medians before filtering (for before/after comparison)
    double rawMedianEbitda  = medianOf(evEbitda);
    double rawMedianRevenue = medianOf(evRevenue);

    // MAD-based outlier filter (modified z-score > 3.5, both tails)
    size_t droppedEbitda = 0, droppedRevenue = 0;
    evEbitda  = filterOutliersMAD(evEbitda,  3.5, &droppedEbitda);
    evRevenue = filterOutliersMAD(evRevenue, 3.5, &droppedRevenue);

    r.medianEvEbitda  = medianOf(evEbitda);
    r.medianEvRevenue = medianOf(evRevenue);
    r.impliedEvFromEbitda  = targetEbitda  * r.medianEvEbitda;
    r.impliedEvFromRevenue = targetRevenue * r.medianEvRevenue;
    r.valid = true;

    std::cout << "[Comps filter] EV/EBITDA: dropped " << droppedEbitda
              << ", median " << rawMedianEbitda << " -> " << r.medianEvEbitda << "\n"
              << "[Comps filter] EV/Revenue: dropped " << droppedRevenue
              << ", median " << rawMedianRevenue << " -> " << r.medianEvRevenue << "\n";

    return r;
}
