//mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QChartView>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QDateTime>
#include <QTimer>
#include <QListWidget>
#include <QLabel>
#include <nlohmann/json.hpp>
#include "DataAnalyzer.h"

// Dodane nagłówki dla kontenerów STL
#include <map>
#include <set>
#include <string>
#include <vector>
#include <limits>
#include <algorithm>

struct GeoCoordinate {
    double latitude;
    double longitude;
    double distanceTo(const GeoCoordinate& other) const;
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onFetchData();      // Funkcja do pobierania danych
    void onAnalyzeData();    // Funkcja do analizy danych
    void initializeStations(); // Funkcja do inicjalizacji stacji
    void onStationChanged(int index); // Funkcja wywoływana po zmianie stacji
    void onLoadHistoricalData(); // Funkcja do wczytywania danych historycznych
    void onSaveData(); // Funkcja do zapisywania danych
    void onFindNearestStation(); // slot do wyszukiwania najbliższej stacji
    void refreshChart(); // Funkcja do odświeżania wykresu po zmianie zaznaczonych parametrów

private:
    void updateStationsComboBox(const nlohmann::json& stations); // Aktualizowanie listy stacji
    void updateSensorList(const nlohmann::json& data);  // Aktualizowanie listy czujników

    // Funkcja do wyświetlania wykresu z wieloma parametrami
    void displayMultiParamChart();

    void initializeUI();
    // Istniejąca funkcja (zachowujemy dla kompatybilności)
    void displayAnalysisResults(const std::map<std::string, double>& results, const QString& paramName);

    void initParamCheckList(const std::set<std::string>& availableParams);

    void displayStatistics(const DataAnalyzer& analyzer); // Wyświetlanie statystyk
    void processAndDisplayData(const nlohmann::json& data, const QString& paramName); // Przetwarzanie i wyświetlanie danych
    void tryLoadOfflineStations(); // Próba wczytania stacji z lokalnej bazy
    GeoCoordinate getLocationFromIP(); // Pobieranie lokalizacji na podstawie IP
    void findNearestStationWithCoordinates(const GeoCoordinate &currentPosition, const nlohmann::json &stations);

    Ui::MainWindow *ui;

    // Zmienne do przechowywania danych pomiarowych dla różnych parametrów
    std::map<std::string, std::map<std::string, double>> m_allResults;

    // Zmienna do przechowywania dostępnych parametrów
    std::set<std::string> m_availableParams;
};

#endif // MAINWINDOW_H
