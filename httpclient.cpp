/**
 * @file HttpClient.cpp
 * @brief Implementacja klasy HttpClient służącej do wykonywania żądań HTTP
 * @details Zawiera funkcjonalność do pobierania danych z API o jakości powietrza
 */

#include "HttpClient.h"
#include <curl/curl.h>
#include <iostream>

/**
 * @brief Funkcja callback dla biblioteki libcurl do odbierania danych HTTP
 * @param contents Wskaźnik do odebranych danych
 * @param size Rozmiar pojedynczego elementu
 * @param nmemb Liczba elementów
 * @param output Wskaźnik do stringa, do którego zapisywane są dane
 * @return Całkowity rozmiar odebranych danych
 * @details Funkcja jest używana wewnętrznie przez libcurl do przechwytywania odpowiedzi HTTP
 */
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

/**
 * @brief Wykonuje żądanie HTTP GET
 * @param url Adres URL, do którego ma zostać wysłane żądanie
 * @return Odpowiedź serwera jako string
 * @details Wykorzystuje bibliotekę libcurl do wykonania żądania HTTP GET
 */
std::string HttpClient::getRequest(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "Curl request failed: " << curl_easy_strerror(res) << std::endl;
        }
        curl_easy_cleanup(curl);
    }
    return response;
}

/**
 * @brief Sprawdza dostępność połączenia internetowego
 * @return true jeśli połączenie internetowe jest dostępne, false w przeciwnym przypadku
 * @details Próbuje połączyć się z wiarygodnym serwerem Google z krótkim limitem czasu oczekiwania
 */
bool HttpClient::isNetworkAvailable() {
    CURL* curl = curl_easy_init();
    if (!curl) {
        return false;
    }
    // Try to connect to a reliable server with a short timeout
    curl_easy_setopt(curl, CURLOPT_URL, "https://www.google.com");
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);       // HEAD request only (no body)
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 3L);      // 3 second timeout
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3L); // 3 second connect timeout
    // Don't follow redirects
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);
    // Create a no-op write function to avoid printing response to stdout
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                     [](void*, size_t size, size_t nmemb, void*) -> size_t { return size * nmemb; });
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    return (res == CURLE_OK);
}
