#pragma once
#include <string>
#include <nlohmann/json.hpp>

/**
 * @brief Klasa odpowiedzialna za operacje na plikach bazy danych
 *
 * Klasa Database zapewnia funkcjonalność do zapisywania i wczytywania
 * danych w formacie JSON do/z plików. Wszystkie metody są statyczne,
 * co umożliwia korzystanie z nich bez konieczności tworzenia instancji klasy.
 */
class Database {
public:
    /**
     * @brief Zapisuje dane JSON do pliku
     * @param filename Nazwa pliku docelowego
     * @param data Dane w formacie JSON do zapisania
     * @throw std::runtime_error W przypadku błędu zapisu do pliku
     *
     * Metoda serializuje obiekt JSON i zapisuje go do określonego pliku.
     * Jeśli plik istnieje, zostanie nadpisany.
     */
    static void saveToFile(const std::string& filename, const nlohmann::json& data);

    /**
     * @brief Wczytuje dane JSON z pliku
     * @param filename Nazwa pliku źródłowego
     * @return Dane wczytane w formacie JSON
     * @throw std::runtime_error W przypadku błędu odczytu pliku lub parsowania JSON
     * @throw nlohmann::json::parse_error W przypadku niepoprawnego formatu JSON
     *
     * Metoda wczytuje zawartość pliku i parsuje ją do obiektu JSON.
     * Jeśli plik nie istnieje lub jest pusty, zostanie rzucony wyjątek.
     */
    static nlohmann::json loadFromFile(const std::string& filename);

    /**
     * @brief Sprawdza istnienie pliku
     * @param filename Nazwa pliku do sprawdzenia
     * @return true jeśli plik istnieje, false w przeciwnym przypadku
     *
     * Metoda weryfikuje czy plik o podanej nazwie istnieje w systemie plików
     * i czy jest dostępny do odczytu.
     */
    bool fileExists(const std::string& filename);
};
