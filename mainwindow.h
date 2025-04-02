#pragma once
#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QDateTime>
#include <QTimer>
#include <QListWidget>
#include <QLabel>
#include <nlohmann/json.hpp>

#include "DataAnalyzer.h"
#include "ChartManager.h"

#include <map>

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
    void fetchDataForAllSensors();
    void onAnalyzeData();    // Funkcja do analizy danych
    void initializeStations(); // Funkcja do inicjalizacji stacji
    void onStationChanged(int index); // Funkcja wywoływana po zmianie stacji
    void onFindNearestStation(); // slot do wyszukiwania najbliższej stacji
    void refreshChart(); // Funkcja do odświeżania wykresu po zmianie zaznaczonych parametrów

private:
    void updateStationsComboBox(const nlohmann::json& stations); // Aktualizowanie listy stacji
    void updateSensorList(const nlohmann::json& data);  // Aktualizowanie listy czujników
    void initializeUI();
    void displayAnalysisResults(const std::map<std::string, double>& results, const QString& paramName);
    void displayStatistics(const DataAnalyzer& analyzer); // Wyświetlanie statystyk
    void processAndDisplayData(const nlohmann::json& data, const QString& paramName); // Przetwarzanie i wyświetlanie danych
    GeoCoordinate getLocationFromIP(); // Pobieranie lokalizacji na podstawie IP
    void findNearestStationWithCoordinates(const GeoCoordinate &currentPosition, const nlohmann::json &stations);

    // Metody pomocnicze dla obsługi trybu offline
    void tryLoadOfflineStations();
    bool hasSensorOfflineData(int sensorId);
    void loadSensorsForStation(int stationId);
    bool loadOfflineData();
    bool hasOfflineData(const QString& fileName);

    bool m_forceOfflineMode = false;
    void loadOfflineSensorData(int stationId, const QString& stationFileName);
    Ui::MainWindow *ui;

    ChartManager* m_chartManager; //chartmanager
};
