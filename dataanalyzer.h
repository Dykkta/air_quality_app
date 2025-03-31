//dataanalyzer.h
#ifndef DATAANALYZER_H
#define DATAANALYZER_H

#include <nlohmann/json.hpp>
#include <map>
#include <string>
#include <tuple>

class DataAnalyzer {
public:
    void analyzeData(const nlohmann::json& data);
    const std::map<std::string, double>& getResults() const;
    const std::map<std::string, std::tuple<double, double, double>>& getAnalysisResults() const;

    const std::string& getMinDate() const { return minDate; }
    const std::string& getMaxDate() const { return maxDate; }

private:
    std::map<std::string, double> results;
    std::map<std::string, std::tuple<double, double, double>> analysisResults;

    std::string minDate;
    std::string maxDate;
};

#endif // DATAANALYZER_H
