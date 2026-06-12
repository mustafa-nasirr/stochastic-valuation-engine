#include "Reporter.hpp"
#include "Statistics.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

static std::string money(double v) {
    std::ostringstream os;
    os << "$" << std::fixed << std::setprecision(2) << v;
    return os.str();
}

void printConsoleReport(const ReportData& r) {
    auto line = [] { std::cout << std::string(57, '=') << "\n"; };
    auto dash = [] { std::cout << std::string(57, '-') << "\n"; };

    line();
    std::cout << "  STOCHASTIC VALUATION SUMMARY  |  " << r.ticker
              << "  |  " << r.sims << " sims\n";
    line();
    std::cout << std::left << std::setw(36) << "  Current price"
              << std::right << std::setw(15) << money(r.currentPrice) << "\n";
    std::cout << std::left << std::setw(36) << "  Base-case intrinsic value"
              << std::right << std::setw(15) << money(r.baseValue) << "\n";
    dash();
    std::cout << std::left << std::setw(36) << "  Mean intrinsic value"
              << std::right << std::setw(15) << money(r.meanValue) << "\n";
    std::cout << std::left << std::setw(36) << "  Median intrinsic value"
              << std::right << std::setw(15) << money(r.medianValue) << "\n";
    std::cout << std::left << std::setw(36) << "  10th percentile"
              << std::right << std::setw(15) << money(r.p10) << "\n";
    std::cout << std::left << std::setw(36) << "  90th percentile"
              << std::right << std::setw(15) << money(r.p90) << "\n";
    dash();
    std::ostringstream prob;
    prob << std::fixed << std::setprecision(1) << (r.probUndervalued * 100.0) << " %";
    std::cout << std::left << std::setw(36) << "  Probability undervalued"
              << std::right << std::setw(15) << prob.str() << "\n";
    if (r.hasComps) {
        std::cout << std::left << std::setw(36) << "  Comps cross-check (per share)"
                  << std::right << std::setw(15) << money(r.compsImpliedPerShare) << "\n";
    }
    line();
}

void writeDistributionCsv(const std::string& path,
                          const std::vector<double>& values) {
    std::ofstream f(path);
    f << "intrinsic_value_per_share\n";
    for (double v : values) f << std::fixed << std::setprecision(4) << v << "\n";
}

// Maps a value within [lo, hi] to an x pixel within [x0, x1].
static double xmap(double v, double lo, double hi, double x0, double x1) {
    if (hi <= lo) return x0;
    double t = (v - lo) / (hi - lo);
    t = std::clamp(t, 0.0, 1.0);
    return x0 + t * (x1 - x0);
}

static std::string buildHistogramSvg(const ReportData& r,
                                     const std::vector<double>& sorted) {
    const int W = 720, H = 380;
    const double mL = 60, mR = 30, mT = 40, mB = 50;
    const double plotW = W - mL - mR;
    const double plotH = H - mT - mB;
    const int bins = 40;

    // Clip the axis to the 1st-99th percentile so a few extreme tail draws
    // do not squash the chart. This is standard practice for fat-tailed output.
    double lo = percentile(sorted, 0.01);
    double hi = percentile(sorted, 0.99);
    if (hi <= lo) hi = lo + 1.0;
    double binW = (hi - lo) / bins;

    std::vector<int> counts(bins, 0);
    for (double v : sorted) {
        if (v < lo || v > hi) continue;
        int b = static_cast<int>((v - lo) / binW);
        if (b == bins) b = bins - 1;
        if (b >= 0 && b < bins) counts[b]++;
    }
    int maxCount = 1;
    for (int c : counts) maxCount = std::max(maxCount, c);

    std::ostringstream s;
    s << "<svg viewBox=\"0 0 " << W << " " << H << "\" xmlns=\"http://www.w3.org/2000/svg\" "
      << "font-family=\"Segoe UI, Helvetica, Arial, sans-serif\">\n";

    // bars
    double barSlot = plotW / bins;
    for (int i = 0; i < bins; ++i) {
        double h = (static_cast<double>(counts[i]) / maxCount) * plotH;
        double x = mL + i * barSlot;
        double y = mT + (plotH - h);
        s << "<rect x=\"" << std::fixed << std::setprecision(1) << x + 0.5
          << "\" y=\"" << y << "\" width=\"" << (barSlot - 1.0)
          << "\" height=\"" << h << "\" fill=\"#2563eb\" opacity=\"0.85\"/>\n";
    }

    // axes
    s << "<line x1=\"" << mL << "\" y1=\"" << (mT + plotH) << "\" x2=\"" << (mL + plotW)
      << "\" y2=\"" << (mT + plotH) << "\" stroke=\"#888\" stroke-width=\"1\"/>\n";

    // current price reference line
    if (r.currentPrice >= lo && r.currentPrice <= hi) {
        double xp = xmap(r.currentPrice, lo, hi, mL, mL + plotW);
        s << "<line x1=\"" << xp << "\" y1=\"" << mT << "\" x2=\"" << xp << "\" y2=\""
          << (mT + plotH) << "\" stroke=\"#dc2626\" stroke-width=\"2\" stroke-dasharray=\"5,3\"/>\n";
        s << "<text x=\"" << (xp + 4) << "\" y=\"" << (mT + 14)
          << "\" fill=\"#dc2626\" font-size=\"12\">Price " << money(r.currentPrice) << "</text>\n";
    }

    // x-axis labels (lo, mid, hi)
    auto axisLabel = [&](double v, double x) {
        s << "<text x=\"" << x << "\" y=\"" << (mT + plotH + 20)
          << "\" fill=\"#444\" font-size=\"11\" text-anchor=\"middle\">" << money(v) << "</text>\n";
    };
    axisLabel(lo, mL);
    axisLabel((lo + hi) / 2.0, mL + plotW / 2.0);
    axisLabel(hi, mL + plotW);

    s << "<text x=\"" << (W / 2) << "\" y=\"22\" font-size=\"15\" font-weight=\"600\" "
      << "text-anchor=\"middle\" fill=\"#111\">Monte Carlo Intrinsic Value Distribution</text>\n";
    s << "<text x=\"" << (W / 2) << "\" y=\"" << (H - 12)
      << "\" font-size=\"12\" text-anchor=\"middle\" fill=\"#666\">Intrinsic value per share</text>\n";
    s << "</svg>\n";
    return s.str();
}

static std::string buildFootballFieldSvg(const ReportData& r) {
    const int W = 720, H = 220;
    const double mL = 130, mR = 40, mT = 40, mB = 40;
    const double plotW = W - mL - mR;

    // Build the value range to plot across all methods.
    double lo = std::min({r.p10, r.currentPrice, r.baseValue});
    double hi = std::max({r.p90, r.currentPrice, r.baseValue});
    if (r.hasComps) { lo = std::min(lo, r.compsImpliedPerShare); hi = std::max(hi, r.compsImpliedPerShare); }
    double pad = (hi - lo) * 0.08 + 1.0;
    lo -= pad; hi += pad;

    std::ostringstream s;
    s << "<svg viewBox=\"0 0 " << W << " " << H << "\" xmlns=\"http://www.w3.org/2000/svg\" "
      << "font-family=\"Segoe UI, Helvetica, Arial, sans-serif\">\n";
    s << "<text x=\"" << (W / 2) << "\" y=\"22\" font-size=\"15\" font-weight=\"600\" "
      << "text-anchor=\"middle\" fill=\"#111\">Valuation Football Field</text>\n";

    auto rowY = [&](int i) { return mT + i * 42.0; };

    // DCF Monte Carlo range bar (p10 to p90)
    {
        double x1 = xmap(r.p10, lo, hi, mL, mL + plotW);
        double x2 = xmap(r.p90, lo, hi, mL, mL + plotW);
        double y = rowY(0);
        s << "<text x=\"" << (mL - 10) << "\" y=\"" << (y + 14)
          << "\" font-size=\"12\" text-anchor=\"end\" fill=\"#333\">DCF (P10-P90)</text>\n";
        s << "<rect x=\"" << x1 << "\" y=\"" << y << "\" width=\"" << (x2 - x1)
          << "\" height=\"20\" rx=\"3\" fill=\"#2563eb\" opacity=\"0.85\"/>\n";
    }
    // Comps implied (single marker drawn as a short band)
    if (r.hasComps) {
        double xc = xmap(r.compsImpliedPerShare, lo, hi, mL, mL + plotW);
        double y = rowY(1);
        s << "<text x=\"" << (mL - 10) << "\" y=\"" << (y + 14)
          << "\" font-size=\"12\" text-anchor=\"end\" fill=\"#333\">Comps implied</text>\n";
        s << "<rect x=\"" << (xc - 14) << "\" y=\"" << y << "\" width=\"28\" height=\"20\" rx=\"3\" "
          << "fill=\"#16a34a\" opacity=\"0.85\"/>\n";
    }

    // current price line across the chart
    double xp = xmap(r.currentPrice, lo, hi, mL, mL + plotW);
    s << "<line x1=\"" << xp << "\" y1=\"" << (mT - 8) << "\" x2=\"" << xp << "\" y2=\""
      << (H - mB + 6) << "\" stroke=\"#dc2626\" stroke-width=\"2\" stroke-dasharray=\"5,3\"/>\n";
    s << "<text x=\"" << xp << "\" y=\"" << (H - mB + 22)
      << "\" font-size=\"12\" text-anchor=\"middle\" fill=\"#dc2626\">Price "
      << money(r.currentPrice) << "</text>\n";

    s << "</svg>\n";
    return s.str();
}

void writeHtmlReport(const std::string& path,
                     const ReportData& r,
                     const std::vector<double>& sorted) {
    std::ostringstream prob;
    prob << std::fixed << std::setprecision(1) << (r.probUndervalued * 100.0);

    std::string verdict = (r.medianValue > r.currentPrice)
        ? "the model implies the stock screens cheap versus its intrinsic value"
        : "the model implies the stock screens rich versus its intrinsic value";

    std::ofstream f(path);
    f << "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"utf-8\">\n";
    f << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n";
    f << "<title>" << r.ticker << " Valuation</title>\n";
    f << "<style>"
         "body{font-family:Segoe UI,Helvetica,Arial,sans-serif;color:#1a1a1a;"
         "max-width:820px;margin:32px auto;padding:0 20px;line-height:1.55;}"
         "h1{font-size:24px;margin-bottom:2px;}"
         ".sub{color:#666;margin-top:0;font-size:14px;}"
         "table{border-collapse:collapse;width:100%;margin:18px 0;font-size:14px;}"
         "td{padding:8px 10px;border-bottom:1px solid #eee;}"
         "td.k{color:#555;}td.v{text-align:right;font-variant-numeric:tabular-nums;font-weight:600;}"
         ".card{border:1px solid #eee;border-radius:10px;padding:14px 18px;margin:18px 0;}"
         ".tag{display:inline-block;background:#eff6ff;color:#2563eb;border-radius:6px;"
         "padding:2px 8px;font-size:12px;font-weight:600;}"
         "footer{color:#888;font-size:12px;margin-top:28px;}"
         "</style></head><body>\n";

    f << "<span class=\"tag\">STOCHASTIC VALUATION</span>\n";
    f << "<h1>" << (r.name.empty() ? r.ticker : r.name) << " (" << r.ticker << ")</h1>\n";
    f << "<p class=\"sub\">" << r.sims << " Monte Carlo paths &middot; DCF with simulated assumptions</p>\n";

    f << "<div class=\"card\">" << buildHistogramSvg(r, sorted) << "</div>\n";

    f << "<table>\n";
    f << "<tr><td class=\"k\">Current price</td><td class=\"v\">" << money(r.currentPrice) << "</td></tr>\n";
    f << "<tr><td class=\"k\">Base-case intrinsic value</td><td class=\"v\">" << money(r.baseValue) << "</td></tr>\n";
    f << "<tr><td class=\"k\">Mean intrinsic value</td><td class=\"v\">" << money(r.meanValue) << "</td></tr>\n";
    f << "<tr><td class=\"k\">Median intrinsic value</td><td class=\"v\">" << money(r.medianValue) << "</td></tr>\n";
    f << "<tr><td class=\"k\">10th - 90th percentile</td><td class=\"v\">" << money(r.p10)
      << " to " << money(r.p90) << "</td></tr>\n";
    f << "<tr><td class=\"k\">Probability undervalued</td><td class=\"v\">" << prob.str() << " %</td></tr>\n";
    if (r.hasComps)
        f << "<tr><td class=\"k\">Comps cross-check (per share)</td><td class=\"v\">"
          << money(r.compsImpliedPerShare) << "</td></tr>\n";
    f << "</table>\n";

    f << "<div class=\"card\">" << buildFootballFieldSvg(r) << "</div>\n";

    f << "<p>Across " << r.sims << " simulated scenarios, the median intrinsic value is "
      << money(r.medianValue) << " against a market price of " << money(r.currentPrice)
      << ", so " << verdict << ". The 10th to 90th percentile range of "
      << money(r.p10) << " to " << money(r.p90)
      << " is the honest way to read the output: a range, not a single target.</p>\n";

    f << "<footer>Generated by the Stochastic Valuation Engine. Assumptions are illustrative "
         "and this is not investment advice.</footer>\n";
    f << "</body></html>\n";
}
