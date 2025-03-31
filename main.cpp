//main.cpp
#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include "HttpClient.h"
#include "JsonHandler.h"
#include "Database.h"
#include "DataAnalyzer.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Inicjalizacja komponentów
    HttpClient httpClient;
    JsonHandler jsonHandler;
    Database database;
    DataAnalyzer dataAnalyzer;

    // Przykład pobrania danych z API i zapisania do pliku
    std::string url = "https://api.gios.gov.pl/pjp-api/rest/station/findAll";  // URL API do pobrania danych o stacjach
    std::string jsonResponse = httpClient.getRequest(url);
    if (jsonResponse.empty()) {
        QMessageBox::critical(nullptr, "Błąd", "Nie udało się pobrać danych z API!");
        return -1;  // Zakończenie aplikacji w przypadku błędu
    }

    // Parsowanie odpowiedzi JSON
    nlohmann::json data = jsonHandler.parseJson(jsonResponse);
    if (data.empty()) {
        QMessageBox::critical(nullptr, "Błąd", "Błąd parsowania danych JSON!");
        return -1;  // Zakończenie aplikacji w przypadku błędu
    }

    // Zapisz dane do lokalnej bazy danych
    database.saveToFile("stations_data.json", data);

    // Tworzymy okno główne aplikacji
    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();  // Uruchomienie głównej pętli aplikacji
}
