#include "DataLoader.hpp"
#include "DcfModel.hpp"
#include "MonteCarloEngine.hpp"
#include "Statistics.hpp"
#include "CompsAnalyzer.hpp"
#include "Reporter.hpp"

#include <iostream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

static std::string argValue(int argc, char** argv, const std::string& flag,
                            const std::string& fallback) {
    for (int i = 1; i < argc - 1; ++i)
        if (flag == argv[i]) return argv[i + 1];
    return fallback;
}

int main(int argc, char** argv) {
    std::string csvPath   = argValue(argc, argv, "--csv",   "data/sample_company.csv");
    std::string peersPath = argValue(argc, argv, "--peers", "data/peers.csv");
    std::string outDir    = argValue(argc, argv, "--out",   "output");
    int sims = std::stoi(argValue(argc, argv, "--sims", "100000"));

    try {
        CompanyData company = loadCompanyCsv(csvPath);

        // Base case and simulation
        double baseValue = intrinsicValuePerShare(company.inputs);
        std::vector<double> values = runMonteCarlo(company.inputs, sims);

        ReportData r;
        r.ticker          = company.ticker;
        r.name            = company.name;
        r.sims            = sims;
        r.currentPrice    = company.currentPrice;
        r.baseValue       = baseValue;
        r.meanValue       = mean(values);
        r.medianValue     = percentile(values, 0.50);
        r.p10             = percentile(values, 0.10);
        r.p90             = percentile(values, 0.90);
        r.probUndervalued = probAbove(values, company.currentPrice);

        // Comparable company cross-check (optional)
        double targetRevenue = company.inputs.baseRevenue;
        double targetEbitda  = company.inputs.baseRevenue * company.inputs.ebitMargin;
        auto peers = loadPeersCsv(peersPath);
        CompsResult comps = analyzeComps(peers, targetEbitda, targetRevenue);
        if (comps.valid) {
            double impliedEv  = 0.5 * (comps.impliedEvFromEbitda + comps.impliedEvFromRevenue);
            double impliedEq  = impliedEv - company.inputs.netDebt;
            r.hasComps             = true;
            r.compsImpliedPerShare = impliedEq / company.inputs.sharesOutstanding;
        }

        // Output
        fs::create_directories(outDir);
        printConsoleReport(r);
        writeDistributionCsv(outDir + "/distribution.csv", values);
        writeHtmlReport(outDir + "/report.html", r, values);

        std::cout << "\nWrote " << outDir << "/report.html and "
                  << outDir << "/distribution.csv\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
