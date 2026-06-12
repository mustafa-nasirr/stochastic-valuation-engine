#include "CompsAnalyzer.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

static double medianOf(std::vector<double> v) {
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    size_t n = v.size();
    if (n % 2 == 1) return v[n / 2];
    return 0.5 * (v[n / 2 - 1] + v[n / 2]);
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

    r.medianEvEbitda  = medianOf(evEbitda);
    r.medianEvRevenue = medianOf(evRevenue);
    r.impliedEvFromEbitda  = targetEbitda  * r.medianEvEbitda;
    r.impliedEvFromRevenue = targetRevenue * r.medianEvRevenue;
    r.valid = true;
    return r;
}
