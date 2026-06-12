#include "DataLoader.hpp"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

static std::string trim(const std::string& s) {
    size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

CompanyData loadCompanyCsv(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open company file: " + path);
    }

    CompanyData c;
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        size_t comma = line.find(',');
        if (comma == std::string::npos) continue;

        std::string key = trim(line.substr(0, comma));
        std::string val = trim(line.substr(comma + 1));
        if (key.empty()) continue;

        try {
            if      (key == "ticker")             c.ticker = val;
            else if (key == "name")               c.name = val;
            else if (key == "sector")             c.sector = val;
            else if (key == "current_price")      c.currentPrice = std::stod(val);
            else if (key == "base_revenue")       c.inputs.baseRevenue = std::stod(val);
            else if (key == "projection_years")   c.inputs.projectionYears = std::stoi(val);
            else if (key == "revenue_growth")     c.inputs.revenueGrowth = std::stod(val);
            else if (key == "ebit_margin")        c.inputs.ebitMargin = std::stod(val);
            else if (key == "tax_rate")           c.inputs.taxRate = std::stod(val);
            else if (key == "da_pct_revenue")     c.inputs.daPctRevenue = std::stod(val);
            else if (key == "capex_pct_revenue")  c.inputs.capexPctRevenue = std::stod(val);
            else if (key == "nwc_pct_revenue")    c.inputs.nwcPctRevenue = std::stod(val);
            else if (key == "wacc")               c.inputs.wacc = std::stod(val);
            else if (key == "terminal_growth")    c.inputs.terminalGrowth = std::stod(val);
            else if (key == "net_debt")           c.inputs.netDebt = std::stod(val);
            else if (key == "shares_outstanding") c.inputs.sharesOutstanding = std::stod(val);
        } catch (const std::exception&) {
            throw std::runtime_error("Bad numeric value for key '" + key + "': " + val);
        }
    }

    if (c.inputs.sharesOutstanding <= 0.0) {
        throw std::runtime_error("shares_outstanding must be positive in " + path);
    }
    return c;
}
