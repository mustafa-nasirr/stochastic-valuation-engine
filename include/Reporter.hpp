#pragma once
#include <string>
#include <vector>

struct ReportData {
    std::string ticker;
    std::string name;
    int    sims = 0;
    double currentPrice = 0.0;
    double baseValue = 0.0;
    double meanValue = 0.0;
    double medianValue = 0.0;
    double p10 = 0.0;
    double p90 = 0.0;
    double probUndervalued = 0.0;   // fraction in [0,1]
    bool   hasComps = false;
    double compsImpliedPerShare = 0.0;
};

void printConsoleReport(const ReportData& r);

// Writes a self-contained HTML report with an inline SVG histogram and
// football field. sortedValues must be sorted ascending.
void writeHtmlReport(const std::string& path,
                     const ReportData& r,
                     const std::vector<double>& sortedValues);

// Dumps every simulated value, one per line, for use in Excel or Python.
void writeDistributionCsv(const std::string& path,
                          const std::vector<double>& values);
