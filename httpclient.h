#pragma once
#include <string>

/**
 * @brief Klasa do obsługi zapytań HTTP
 *
 * Klasa HttpClient umożliwia wykonywanie podstawowych operacji sieciowych,
 * w tym wysyłanie żądań GET oraz sprawdzanie dostępności połączenia sieciowego.
 * Wszystkie metody są statyczne, co pozwala na ich użycie bez tworzenia instancji klasy.
 */
class HttpClient {
public:
    /**
     * @brief Wykonuje żądanie HTTP GET
     * @param url Adres URL do którego ma zostać wysłane żądanie
     * @return Odpowiedź serwera jako string
     * @throw std::runtime_error W przypadku błędu połączenia
     * @throw std::invalid_argument W przypadku nieprawidłowego URL
     *
     * Metoda wysyła synchroniczne żądanie GET do określonego URL
     * i zwraca otrzymaną odpowiedź. Obsługuje protokoły HTTP i HTTPS.
     * Przykład użycia:
     * @code
     * std::string response = HttpClient::getRequest("https://api.example.com/data");
     * @endcode
     */
    static std::string getRequest(const std::string& url);

    /**
     * @brief Sprawdza dostępność połączenia sieciowego
     * @return true jeśli połączenie jest dostępne, false w przeciwnym przypadku
     *
     * Metoda sprawdza czy urządzenie ma aktywny dostęp do internetu.
     * Weryfikuje zarówno połączenie sieciowe jak i dostępność zdalnych serwerów.
     * Przykład użycia:
     * @code
     * if(HttpClient::isNetworkAvailable()) {
     *     // wykonaj operacje sieciowe
     * }
     * @endcode
     */
    static bool isNetworkAvailable();
};
