#pragma once
#include <string>
#include "DcfModel.hpp"

struct CompanyData {
    std::string ticker;
    std::string name;
    std::string sector;
    double currentPrice = 0.0;
    DcfInputs inputs;
};

// Loads a company from a simple "key,value" CSV file.
// Throws std::runtime_error if the file cannot be opened.
CompanyData loadCompanyCsv(const std::string& path);
