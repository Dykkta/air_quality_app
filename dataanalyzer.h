#pragma once
#include <nlohmann/json.hpp>
#include <map>
#include <string>
#include <tuple>

/**
 * @brief Klasa odpowiedzialna za analizę danych pomiarowych
 *
 * Klasa DataAnalyzer wykonuje obliczenia statystyczne na danych pomiarowych
 * w formacie JSON. Oblicza wartości średnie, minimalne i maksymalne dla parametrów
 * oraz określa zakres dat, w których zostały zebrane dane.
 */
class DataAnalyzer {
public:
    /**
     * @brief Analizuje przekazane dane JSON
     * @param data Dane pomiarowe w formacie JSON do analizy
     *
     * Metoda przetwarza dane wejściowe, obliczając statystyki dla każdego parametru
     * i określając zakres dat pomiarów.
     */
    void analyzeData(const nlohmann::json& data);

    /**
     * @brief Pobiera wyniki analizy
     * @return Mapa wyników (nazwa parametru -> wartość średnia)
     */
    const std::map<std::string, double>& getResults() const;

    /**
     * @brief Pobiera szczegółowe wyniki analizy
     * @return Mapa wyników (nazwa parametru -> krotka (średnia, minimum, maksimum))
     */
    const std::map<std::string, std::tuple<double, double, double>>& getAnalysisResults() const;

    /**
     * @brief Pobiera najwcześniejszą datę pomiaru
     * @return Data w formacie tekstowym
     */
    const std::string& getMinDate() const { return minDate; }

    /**
     * @brief Pobiera najpóźniejszą datę pomiaru
     * @return Data w formacie tekstowym
     */
    const std::string& getMaxDate() const { return maxDate; }

private:
    /// Mapa przechowująca średnie wartości dla każdego parametru
    std::map<std::string, double> results;

    /// Mapa przechowująca statystyki (średnia, min, max) dla każdego parametru
    std::map<std::string, std::tuple<double, double, double>> analysisResults;

    /// Najwcześniejsza data w analizowanych danych
    std::string minDate;

    /// Najpóźniejsza data w analizowanych danych
    std::string maxDate;
};
