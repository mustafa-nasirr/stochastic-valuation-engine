#pragma once
#include <string>
#include <vector>

struct Peer {
    std::string ticker;
    double enterpriseValue = 0.0;
    double ebitda = 0.0;
    double revenue = 0.0;
};

struct CompsResult {
    double medianEvEbitda = 0.0;
    double medianEvRevenue = 0.0;
    double impliedEvFromEbitda = 0.0;   // target EBITDA * peer median EV/EBITDA
    double impliedEvFromRevenue = 0.0;  // target revenue * peer median EV/Revenue
    bool   valid = false;
};

// Loads peers from a header CSV: ticker,enterprise_value,ebitda,revenue
std::vector<Peer> loadPeersCsv(const std::string& path);

CompsResult analyzeComps(const std::vector<Peer>& peers,
                         double targetEbitda, double targetRevenue);
