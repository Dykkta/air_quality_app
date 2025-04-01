//mainwindow.cpp
#include <iostream>
#include <algorithm>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "HttpClient.h"
#include "JsonHandler.h"
#include "Database.h"
#include "DataAnalyzer.h"
#include <QMessageBox>
#include <QPushButton>
#include <QComboBox>
#include <QVBoxLayout>
#include <QChartView>
#include <QLineSeries>
#include <QDateTime>
#include <QFileDialog>
#include <QInputDialog>
#include <exception>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    m_chartManager = new ChartManager(this);
    m_chartManager->connectParamCheckList(ui->paramCheckList);

    initializeUI();
    // Dodaj przycisk "Znajdź najbliższą stację" obok ComboBoxa ze stacjami
    QPushButton *findNearestButton = new QPushButton("Znajdź najbliższą stację", this);
    // Wstaw go do layoutu poziomego w stationGroupBox
    ui->horizontalLayout->addWidget(findNearestButton);

    // Połącz sygnał z nowym slotem
    connect(findNearestButton, &QPushButton::clicked, this, &MainWindow::onFindNearestStation);

    // Połączenie sygnałów z slotami
    //connect(ui->fetchButton, &QPushButton::clicked, this, &MainWindow::onFetchData);
    connect(ui->analyzeButton, &QPushButton::clicked, this, &MainWindow::onAnalyzeData);
    connect(ui->actionLoadData, &QAction::triggered, this, &MainWindow::onLoadHistoricalData);
    connect(ui->actionSaveData, &QAction::triggered, this, &MainWindow::onSaveData);
    connect(ui->actionRefreshStations, &QAction::triggered, this, &MainWindow::initializeStations);

    // Inicjalizacja dat w kontrolkach
    QDateTime currentDateTime = QDateTime::currentDateTime();
    ui->endDateTimeEdit->setDateTime(currentDateTime);
    ui->startDateTimeEdit->setDateTime(currentDateTime.addDays(-7)); // Domyślnie tydzień wstecz

    // Inicjalizacja listy stacji
    initializeStations();

    // Połączenie zmiany stacji z aktualizacją listy czujników
    connect(ui->stationComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onStationChanged);
}

void MainWindow::initializeUI() {
    // Ustawienie okna na pełny ekran
    showMaximized();

    // Dodanie placeholder tekstów dla mapy (można będzie usunąć po implementacji mapy)
    ui->mapPlaceholderLabel->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid #cccccc; }");

    // Ustawienie elastyczności paneli
    ui->leftPanel->setMinimumWidth(300); // Minimalna szerokość dla lewego panelu

    // Inicjalizacja dat
    QDateTime currentDateTime = QDateTime::currentDateTime();
    ui->startDateTimeEdit->setDateTime(currentDateTime.addDays(-7)); // Domyślnie ostatni tydzień
    ui->endDateTimeEdit->setDateTime(currentDateTime);

    // Połączenie kontrolek dat z funkcją odświeżania wykresu
    connect(ui->startDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &MainWindow::refreshChart);
    connect(ui->endDateTimeEdit, &QDateTimeEdit::dateTimeChanged, this, &MainWindow::refreshChart);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::initializeStations() {
    try {
        HttpClient client;
        std::string url = "https://api.gios.gov.pl/pjp-api/rest/station/findAll";
        std::string jsonData = client.getRequest(url);

        // Parsowanie JSON
        JsonHandler jsonHandler;
        nlohmann::json stations = jsonHandler.parseJson(jsonData);

        if (stations.empty() || !stations.is_array()) {
            throw std::runtime_error("Nie udało się pobrać listy stacji z serwera.");
        }

        // Zapisz pobrane dane do pliku
        Database::saveToFile("stations_data.json", stations);

        // Aktualizacja interfejsu użytkownika
        updateStationsComboBox(stations);

        ui->statusbar->showMessage("Pomyślnie pobrano listę stacji.", 5000);
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Błąd połączenia",
                             QString("Nie udało się pobrać danych ze serwera: %1\n\nCzy chcesz wczytać dane z lokalnej bazy?").arg(e.what()));

        // Próba wczytania danych offline
        tryLoadOfflineStations();
    }
}

void MainWindow::tryLoadOfflineStations() {
    try {
        nlohmann::json stations = Database::loadFromFile("stations_data.json");

        if (stations.empty() || !stations.is_array()) {
            QMessageBox::critical(this, "Błąd", "Brak zapisanych danych o stacjach pomiarowych.");
            return;
        }

        updateStationsComboBox(stations);
        ui->statusbar->showMessage("Wczytano dane offline o stacjach pomiarowych.", 5000);
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Błąd",
                              QString("Nie udało się wczytać danych offline: %1").arg(e.what()));
    }
}

void MainWindow::updateStationsComboBox(const nlohmann::json& stations) {
    ui->stationComboBox->clear();

    // Tworzymy wektor par (nazwa stacji, id stacji)
    std::vector<std::pair<QString, int>> stationData;

    for (const auto& station : stations) {
        if (station.is_object() && station.contains("id") && station.contains("stationName")) {
            QString stationName = QString::fromStdString(station["stationName"]);
            int stationId = station["id"].get<int>();
            stationData.push_back({stationName, stationId});
        }
    }

    // Sortujemy wektor alfabetycznie według nazwy stacji
    std::sort(stationData.begin(), stationData.end(),
              [](const std::pair<QString, int>& a, const std::pair<QString, int>& b) {
                  return a.first < b.first;
              });

    // Dodajemy posortowane dane do comboBox
    for (const auto& [name, id] : stationData) {
        ui->stationComboBox->addItem(name, QVariant(id));
    }
}

void MainWindow::onStationChanged(int index) {
    if (index < 0) return;

    // Pobierz ID stacji z danych elementu comboBox
    int stationId = ui->stationComboBox->itemData(index).toInt();

    //reset listy parametrów
    ui->paramCheckList->clear();

    // Wyczyść dane w ChartManager
    m_chartManager->clearData();

    // Wyczyść obszar wykresu (jeśli istnieje)
    if (ui->chartLayout->count() > 0) {
        QLayoutItem *child;
        while ((child = ui->chartLayout->takeAt(0)) != nullptr) {
            if (child->widget()) {
                delete child->widget();
            }
            delete child;
        }
    }

    // Generuj nazwę pliku dla tej stacji
    QString stationFileName = QString("station_%1_sensors.json").arg(stationId);

    try {
        HttpClient client;
        std::string url = "https://api.gios.gov.pl/pjp-api/rest/station/sensors/" + std::to_string(stationId);
        std::string jsonData = client.getRequest(url);

        // Wydrukuj odpowiedź JSON do debugowania
        std::cout << "Odpowiedź API (sensors): " << jsonData << std::endl;

        // Parsowanie JSON
        JsonHandler jsonHandler;
        nlohmann::json data = jsonHandler.parseJson(jsonData);

        if (data.empty()) {
            throw std::runtime_error("Nie udało się pobrać danych z API.");
        }

        // Sprawdź, czy dane są tablicą
        if (!data.is_array()) {
            throw std::runtime_error("Otrzymane dane nie są w oczekiwanym formacie.");
        }

        // Zapis do lokalnej bazy danych z nazwą zawierającą ID stacji
        Database::saveToFile(stationFileName.toStdString(), data);

        // Aktualizacja UI (wybór czujników)
        updateSensorList(data);

        fetchDataForAllSensors();

        ui->statusbar->showMessage(QString("Pomyślnie pobrano dane o czujnikach dla stacji %1").arg(ui->stationComboBox->currentText()), 5000);

    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Błąd połączenia",
                             QString("Nie udało się pobrać danych o czujnikach: %1\n\nPróba wczytania danych z lokalnej bazy.").arg(e.what()));

        // Próba wczytania danych offline
        nlohmann::json offlineData = Database::loadFromFile(stationFileName.toStdString());

        if (!offlineData.empty() && offlineData.is_array()) {
            updateSensorList(offlineData);
            fetchDataForAllSensors(); //pobranie danych dla wszystkich czujnikow tez offline
            ui->statusbar->showMessage(QString("Wczytano offline dane o czujnikach dla stacji %1").arg(ui->stationComboBox->currentText()), 5000);
        } else {
            QMessageBox::critical(this, "Błąd", "Brak zapisanych danych o czujnikach dla wybranej stacji.");
        }
    }
}

void MainWindow::fetchDataForAllSensors() {
    // Sprawdź czy lista czujników zawiera elementy
    if (ui->sensorComboBox->count() <= 0) {
        return;
    }

    // Dla każdego czujnika pobierz dane
    for (int i = 0; i < ui->sensorComboBox->count(); i++) {
        int sensorId = ui->sensorComboBox->itemData(i).toInt();
        QString paramName = ui->sensorComboBox->itemText(i);

        try {
            HttpClient client;
            std::string url = "https://api.gios.gov.pl/pjp-api/rest/data/getData/" + std::to_string(sensorId);
            std::string jsonData = client.getRequest(url);

            // Parsowanie JSON
            JsonHandler jsonHandler;
            nlohmann::json data = jsonHandler.parseJson(jsonData);

            if (data.empty() || !data.is_object()) {
                throw std::runtime_error("Nie udało się pobrać danych z API lub dane są w nieprawidłowym formacie.");
            }

            // Zapis do lokalnej bazy danych z nazwą zawierającą ID czujnika
            QString fileName = QString("sensor_%1_data.json").arg(sensorId);
            Database::saveToFile(fileName.toStdString(), data);

            // Przeprowadzenie analizy danych i dodanie do ChartManager
            DataAnalyzer analyzer;
            analyzer.analyzeData(data);
            const auto& results = analyzer.getResults();

            // Dodaj dane do ChartManager, aby były dostępne dla wykresów
            m_chartManager->addParameterData(paramName.toStdString(), results);

            // Aktualizuj listę dostępnych parametrów w UI
            m_chartManager->updateParamCheckList(ui->paramCheckList, paramName);

            // Status dla pierwszego parametru (opcjonalnie)
            if (i == 0) {
                ui->statusbar->showMessage(QString("Pobrano dane dla parametrów stacji %1").arg(ui->stationComboBox->currentText()), 5000);
            }

        } catch (const std::exception& e) {
            // W przypadku błędu spróbuj wczytać dane z lokalnej bazy
            QString fileName = QString("sensor_%1_data.json").arg(sensorId);
            nlohmann::json offlineData = Database::loadFromFile(fileName.toStdString());

            if (!offlineData.empty() && offlineData.is_object()) {
                // Przeprowadzenie analizy danych i dodanie do ChartManager
                DataAnalyzer analyzer;
                analyzer.analyzeData(offlineData);
                const auto& results = analyzer.getResults();

                // Dodaj dane do ChartManager, aby były dostępne dla wykresów
                m_chartManager->addParameterData(paramName.toStdString(), results);

                // Aktualizuj listę dostępnych parametrów w UI
                m_chartManager->updateParamCheckList(ui->paramCheckList, paramName);
            }
        }
    }

    // Po załadowaniu wszystkich danych, podłącz listę parametrów do ChartManager
    m_chartManager->connectParamCheckList(ui->paramCheckList);

    // Wyświetl wykres, jeśli mamy jakieś dane
    if (m_chartManager->getAvailableParams().size() > 0) {
        // Tutaj możesz dodać kod do automatycznego wyświetlenia wykresu
        // np. przy użyciu danych z pierwszego czujnika lub wszystkich czujników razem
        // W zależności od tego, jak zdefiniowana jest funkcja displayMultiParamChart
    }
}

void MainWindow::onFetchData() {
    int currentIndex = ui->stationComboBox->currentIndex();
    if (currentIndex < 0) {
        QMessageBox::warning(this, "Błąd", "Nie wybrano stacji.");
        return;
    }

    // Pobierz ID stacji z danych elementu comboBox
    int stationId = ui->stationComboBox->itemData(currentIndex).toInt();
    QString stationName = ui->stationComboBox->currentText();

    try {
        HttpClient client;
        std::string url = "https://api.gios.gov.pl/pjp-api/rest/station/sensors/" + std::to_string(stationId);
        std::string jsonData = client.getRequest(url);

        // Wydrukuj odpowiedź JSON do debugowania
        std::cout << "Odpowiedź API (sensors): " << jsonData << std::endl;

        // Parsowanie JSON
        JsonHandler jsonHandler;
        nlohmann::json data = jsonHandler.parseJson(jsonData);

        if (data.empty()) {
            throw std::runtime_error("Nie udało się pobrać danych z API.");
        }

        // Sprawdź, czy dane są tablicą
        if (!data.is_array()) {
            throw std::runtime_error("Otrzymane dane nie są w oczekiwanym formacie.");
        }

        // Zapis do lokalnej bazy danych z nazwą zawierającą ID stacji
        QString fileName = QString("station_%1_sensors.json").arg(stationId);
        Database::saveToFile(fileName.toStdString(), data);

        // Aktualizacja UI (wybór czujników)
        updateSensorList(data);

        ui->statusbar->showMessage(QString("Pomyślnie pobrano dane o czujnikach dla stacji %1").arg(stationName), 5000);

    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Błąd połączenia",
                             QString("Nie udało się pobrać danych o czujnikach: %1\n\nCzy chcesz wczytać dane z lokalnej bazy?").arg(e.what()));

        // Próba wczytania danych offline
        QString fileName = QString("station_%1_sensors.json").arg(stationId);
        nlohmann::json offlineData = Database::loadFromFile(fileName.toStdString());

        if (!offlineData.empty() && offlineData.is_array()) {
            updateSensorList(offlineData);
            ui->statusbar->showMessage(QString("Wczytano offline dane o czujnikach dla stacji %1").arg(stationName), 5000);
        } else {
            QMessageBox::critical(this, "Błąd", "Brak zapisanych danych o czujnikach dla wybranej stacji.");
        }
    }
}

void MainWindow::onAnalyzeData() {
    int stationIndex = ui->stationComboBox->currentIndex();
    int sensorIndex = ui->sensorComboBox->currentIndex();

    if (stationIndex < 0) {
        QMessageBox::warning(this, "Błąd", "Nie wybrano stacji.");
        return;
    }

    if (sensorIndex < 0) {
        QMessageBox::warning(this, "Błąd", "Nie wybrano czujnika.");
        return;
    }

    // Pobierz ID czujnika z danych elementu comboBox
    int sensorId = ui->sensorComboBox->itemData(sensorIndex).toInt();
    QString paramName = ui->sensorComboBox->currentText();

    try {
        HttpClient client;
        std::string url = "https://api.gios.gov.pl/pjp-api/rest/data/getData/" + std::to_string(sensorId);
        std::string jsonData = client.getRequest(url);

        // Wydrukuj odpowiedź JSON do debugowania
        std::cout << "Odpowiedź API (data): " << jsonData << std::endl;

        // Parsowanie JSON
        JsonHandler jsonHandler;
        nlohmann::json data = jsonHandler.parseJson(jsonData);

        if (data.empty() || !data.is_object()) {
            throw std::runtime_error("Nie udało się pobrać danych z API lub dane są w nieprawidłowym formacie.");
        }

        // Zapis do lokalnej bazy danych z nazwą zawierającą ID czujnika
        QString fileName = QString("sensor_%1_data.json").arg(sensorId);
        Database::saveToFile(fileName.toStdString(), data);

        // Przeprowadzenie analizy i wyświetlenie wyników
        processAndDisplayData(data, paramName);

        ui->statusbar->showMessage(QString("Pomyślnie pobrano i przeanalizowano dane dla %1").arg(paramName), 5000);

    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Błąd połączenia",
                             QString("Nie udało się pobrać danych pomiarowych: %1\n\nCzy chcesz wczytać dane z lokalnej bazy?").arg(e.what()));

        // Próba wczytania danych offline
        QString fileName = QString("sensor_%1_data.json").arg(sensorId);
        nlohmann::json offlineData = Database::loadFromFile(fileName.toStdString());

        if (!offlineData.empty() && offlineData.is_object()) {
            processAndDisplayData(offlineData, paramName);
            ui->statusbar->showMessage(QString("Wczytano offline dane pomiarowe dla %1").arg(paramName), 5000);
        } else {
            QMessageBox::critical(this, "Błąd", "Brak zapisanych danych pomiarowych dla wybranego czujnika.");
        }
    }
}

void MainWindow::updateSensorList(const nlohmann::json& data) {
    ui->sensorComboBox->clear();

    for (const auto& sensor : data) {
        if (sensor.is_object() && sensor.contains("id") &&
            sensor.contains("param") && sensor["param"].is_object() &&
            sensor["param"].contains("paramName")) {

            QString paramName = QString::fromStdString(sensor["param"]["paramName"]);
            // Zapisz ID czujnika jako userData dla elementu comboBox
            ui->sensorComboBox->addItem(paramName, QVariant(sensor["id"].get<int>()));
        }
    }
}

void MainWindow::processAndDisplayData(const nlohmann::json& data, const QString& paramName) {
    // Najpierw czyścimy istniejący układ wykresu - poprawiona wersja
    QLayoutItem* item;
    while ((item = ui->chartLayout->takeAt(0)) != nullptr) {
        if (item->widget()) {
            delete item->widget();
        }
        delete item;
    }

    // Sprawdź, czy dane zawierają wartości
    if (!data.contains("values") || !data["values"].is_array()) {
        QMessageBox::warning(this, "Błąd", "Dane nie zawierają wartości pomiarowych.");
        return;
    }

    // Przeprowadzenie analizy danych
    DataAnalyzer analyzer;
    analyzer.analyzeData(data);

    // Pokazanie wyników analizy w UI
    const auto& results = analyzer.getResults();
    displayAnalysisResults(results, paramName);

    // Wyświetlanie statystyk
    displayStatistics(analyzer);
}
void MainWindow::displayAnalysisResults(const std::map<std::string, double>& results, const QString& paramName) {
    // Add the data to the ChartManager
    m_chartManager->addParameterData(paramName.toStdString(), results);

    // Update the parameter check list
    m_chartManager->updateParamCheckList(ui->paramCheckList, paramName);

    // Display the chart with the currently selected parameters
    m_chartManager->displayMultiParamChart(ui->chartLayout, ui->paramCheckList,
                                           ui->startDateTimeEdit->dateTime(),
                                           ui->endDateTimeEdit->dateTime());
}

void MainWindow::refreshChart() {
    // Just call the ChartManager to display the chart
    m_chartManager->displayMultiParamChart(ui->chartLayout, ui->paramCheckList,
                                           ui->startDateTimeEdit->dateTime(),
                                           ui->endDateTimeEdit->dateTime());
}


void MainWindow::displayStatistics(const DataAnalyzer& analyzer) {
    const auto& results = analyzer.getAnalysisResults();
    double sum = 0.0;
    int count = 0;

    // Calculate sum and count regardless of which path we take
    for (const auto& [date, stats] : results) {
        double value = std::get<0>(stats);
        sum += value;
        count++;
    }

    // Calculate and display average if we have data
    if (count > 0) {
        ui->avgValueEdit->setText(QString::number(sum / count));
    } else {
        ui->avgValueEdit->setText("Brak danych");
    }

    // Now handle min/max values
    if (!analyzer.getMinDate().empty() && !analyzer.getMaxDate().empty()) {
        // Use the analyzer's methods
        ui->minValueEdit->setText(QString::number(std::get<0>(results.at(analyzer.getMinDate()))));
        ui->maxValueEdit->setText(QString::number(std::get<0>(results.at(analyzer.getMaxDate()))));
        ui->minDateEdit->setText(QString::fromStdString(analyzer.getMinDate()));
        ui->maxDateEdit->setText(QString::fromStdString(analyzer.getMaxDate()));
    } else {
        // Calculate min/max manually
        double minValue = 999999.0;
        double maxValue = -999999.0;
        std::string minDate, maxDate;

        for (const auto& [date, stats] : results) {
            double value = std::get<0>(stats);
            if (value < minValue) {
                minValue = value;
                minDate = date;
            }
            if (value > maxValue) {
                maxValue = value;
                maxDate = date;
            }
        }

        if (count > 0) {
            ui->minValueEdit->setText(QString::number(minValue));
            ui->maxValueEdit->setText(QString::number(maxValue));
            ui->minDateEdit->setText(QString::fromStdString(minDate));
            ui->maxDateEdit->setText(QString::fromStdString(maxDate));
        }
    }

    // Calculate trend
    if (results.size() > 1) {
        auto firstIt = results.begin();
        auto lastIt = std::prev(results.end());
        double firstValue = std::get<0>(firstIt->second);
        double lastValue = std::get<0>(lastIt->second);

        if (lastValue > firstValue) {
            ui->trendEdit->setText("Wzrostowy");
        } else if (lastValue < firstValue) {
            ui->trendEdit->setText("Malejący");
        } else {
            ui->trendEdit->setText("Stały");
        }
    } else {
        ui->trendEdit->setText("Brak danych");
    }
}

void MainWindow::onLoadHistoricalData() {
    QString fileName = QFileDialog::getOpenFileName(this, "Wczytaj dane historyczne", "", "Pliki JSON (*.json)");
    if (fileName.isEmpty()) {
        return;
    }

    try {
        nlohmann::json data = Database::loadFromFile(fileName.toStdString());

        // Sprawdzamy rodzaj danych w pliku
        if (data.is_array() && !data.empty() && data[0].contains("stationName")) {
            // To są dane o stacjach
            updateStationsComboBox(data);
            ui->statusbar->showMessage("Wczytano historyczne dane o stacjach pomiarowych.", 5000);
        } else if (data.is_array() && !data.empty() && data[0].contains("param")) {
            // To są dane o czujnikach
            updateSensorList(data);
            ui->statusbar->showMessage("Wczytano historyczne dane o czujnikach.", 5000);
        } else if (data.is_object() && data.contains("values")) {
            // To są dane pomiarowe
            bool ok;
            QString paramName = QInputDialog::getText(this, "Nazwa parametru",
                                                      "Podaj nazwę parametru dla wykresu:", QLineEdit::Normal, "", &ok);
            if (ok && !paramName.isEmpty()) {
                processAndDisplayData(data, paramName);
                ui->statusbar->showMessage("Wczytano historyczne dane pomiarowe.", 5000);
            }
        } else {
            QMessageBox::warning(this, "Błąd", "Nierozpoznany format danych.");
        }
    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Błąd",
                              QString("Nie udało się wczytać danych z pliku: %1").arg(e.what()));
    }
}

void MainWindow::onSaveData() {
    QString fileName = QFileDialog::getSaveFileName(this, "Zapisz dane", "", "Pliki JSON (*.json)");
    if (fileName.isEmpty()) {
        return;
    }

    int stationIndex = ui->stationComboBox->currentIndex();
    int sensorIndex = ui->sensorComboBox->currentIndex();

    if (stationIndex < 0) {
        QMessageBox::warning(this, "Błąd", "Nie wybrano stacji.");
        return;
    }

    if (sensorIndex < 0) {
        // Zapisz dane o stacji
        int stationId = ui->stationComboBox->itemData(stationIndex).toInt();
        QString stationFileName = QString("station_%1_sensors.json").arg(stationId);
        nlohmann::json stationData = Database::loadFromFile(stationFileName.toStdString());

        if (!stationData.empty()) {
            Database::saveToFile(fileName.toStdString(), stationData);
            ui->statusbar->showMessage(QString("Zapisano dane o czujnikach dla stacji %1").arg(ui->stationComboBox->currentText()), 5000);
        } else {
            QMessageBox::warning(this, "Błąd", "Brak danych do zapisania.");
        }
    } else {
        // Zapisz dane pomiarowe
        int sensorId = ui->sensorComboBox->itemData(sensorIndex).toInt();
        QString dataFileName = QString("sensor_%1_data.json").arg(sensorId);
        nlohmann::json sensorData = Database::loadFromFile(dataFileName.toStdString());

        if (!sensorData.empty()) {
            Database::saveToFile(fileName.toStdString(), sensorData);
            ui->statusbar->showMessage(QString("Zapisano dane pomiarowe dla %1").arg(ui->sensorComboBox->currentText()), 5000);
        } else {
            QMessageBox::warning(this, "Błąd", "Brak danych do zapisania.");
        }
    }
}

// Obliczanie odległości
double GeoCoordinate::distanceTo(const GeoCoordinate& other) const {
    // Earth radius in meters
    const double R = 6371000.0;

    // Convert to radians
    double lat1 = latitude * M_PI / 180.0;
    double lon1 = longitude * M_PI / 180.0;
    double lat2 = other.latitude * M_PI / 180.0;
    double lon2 = other.longitude * M_PI / 180.0;

    // Haversine formula
    double dLat = lat2 - lat1;
    double dLon = lon2 - lon1;
    double a = std::sin(dLat/2) * std::sin(dLat/2) +
               std::cos(lat1) * std::cos(lat2) *
                   std::sin(dLon/2) * std::sin(dLon/2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1-a));
    double distance = R * c;

    return distance;
}

GeoCoordinate MainWindow::getLocationFromIP() {
    GeoCoordinate result = {0.0, 0.0};

    try {
        // Create a temporary file to store curl response
        QString tempFileName = QDir::tempPath() + "/ip_location.json";

        // Execute curl to fetch geolocation data from ipinfo.io or similar service
        QString command = QString("curl -s http://ip-api.com/json/ > %1").arg(tempFileName);
        int exitCode = system(command.toStdString().c_str());

        if (exitCode != 0) {
            throw std::runtime_error("Błąd podczas wykonywania polecenia curl.");
        }

        // Read the response from the temporary file
        QFile file(tempFileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            throw std::runtime_error("Nie można otworzyć pliku z danymi lokalizacji.");
        }

        QString responseData = file.readAll();
        file.close();

        // Parse JSON response
        JsonHandler jsonHandler;
        nlohmann::json data = jsonHandler.parseJson(responseData.toStdString());

        // Check if we have valid location data
        if (data.contains("lat") && data.contains("lon") &&
            !data["lat"].is_null() && !data["lon"].is_null()) {

            result.latitude = data["lat"].get<double>();
            result.longitude = data["lon"].get<double>();
        } else {
            throw std::runtime_error("Brak danych o lokalizacji w odpowiedzi.");
        }

        // Clean up the temporary file
        QFile::remove(tempFileName);

    } catch (const std::exception& e) {
        QMessageBox::warning(nullptr, "Błąd lokalizacji",
                             QString("Nie udało się określić lokalizacji: %1").arg(e.what()));
    }

    return result;
}
void MainWindow::onFindNearestStation() {
    try {
        // First try to load station data
        nlohmann::json stations;

        // Try to get data from server or load from local database
        try {
            HttpClient client;
            std::string url = "https://api.gios.gov.pl/pjp-api/rest/station/findAll";
            std::string jsonData = client.getRequest(url);
            JsonHandler jsonHandler;
            stations = jsonHandler.parseJson(jsonData);

            if (stations.empty() || !stations.is_array()) {
                throw std::runtime_error("Nie udało się pobrać listy stacji z serwera.");
            }

            // Save the retrieved data to a file
            Database::saveToFile("stations_data.json", stations);
        }
        catch (const std::exception& e) {
            // If failed to get data from server, load from local database
            stations = Database::loadFromFile("stations_data.json");

            if (stations.empty() || !stations.is_array()) {
                throw std::runtime_error("Brak zapisanych danych o stacjach.");
            }
        }

        // Get location from IP
        GeoCoordinate currentPosition = getLocationFromIP();

        // If location determination was successful
        if (currentPosition.latitude != 0.0 || currentPosition.longitude != 0.0) {
            findNearestStationWithCoordinates(currentPosition, stations);
        } else {
            // If automatic location failed, ask the user to input location manually
            bool ok;
            QString locationInput = QInputDialog::getText(this, "Podaj lokalizację",
                                                          "Nie można uzyskać lokalizacji automatycznie.\nPodaj współrzędne (szerokość,długość):",
                                                          QLineEdit::Normal, "52.2297,21.0122", &ok);

            if (ok && !locationInput.isEmpty()) {
                QStringList coords = locationInput.split(",");
                if (coords.size() == 2) {
                    bool latOk, lonOk;
                    double lat = coords[0].trimmed().toDouble(&latOk);
                    double lon = coords[1].trimmed().toDouble(&lonOk);

                    if (latOk && lonOk) {
                        GeoCoordinate manualLocation = {lat, lon};
                        findNearestStationWithCoordinates(manualLocation, stations);
                    }
                }
            }
        }

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Błąd",
                              QString("Wystąpił błąd podczas wyszukiwania najbliższej stacji: %1").arg(e.what()));
    }
}

void MainWindow::findNearestStationWithCoordinates(const GeoCoordinate &currentPosition, const nlohmann::json &stations) {
    // Variable to store the nearest station
    int closestStationId = -1;
    QString closestStationName;
    double closestDistance = std::numeric_limits<double>::max();

    // Search all stations to find the nearest one
    for (const auto& station : stations) {
        if (station.contains("gegrLat") && station.contains("gegrLon") &&
            !station["gegrLat"].is_null() && !station["gegrLon"].is_null()) {

            double lat = 0.0;
            double lon = 0.0;

            // Check if the values are strings and convert them if needed
            if (station["gegrLat"].is_string()) {
                try {
                    lat = std::stod(station["gegrLat"].get<std::string>());
                } catch (const std::exception& e) {
                    continue; // Skip this station if conversion fails
                }
            } else if (station["gegrLat"].is_number()) {
                lat = station["gegrLat"].get<double>();
            } else {
                continue; // Skip if not a valid type
            }

            if (station["gegrLon"].is_string()) {
                try {
                    lon = std::stod(station["gegrLon"].get<std::string>());
                } catch (const std::exception& e) {
                    continue; // Skip this station if conversion fails
                }
            } else if (station["gegrLon"].is_number()) {
                lon = station["gegrLon"].get<double>();
            } else {
                continue; // Skip if not a valid type
            }

            GeoCoordinate stationPosition = {lat, lon};
            double distance = currentPosition.distanceTo(stationPosition);

            if (distance < closestDistance) {
                closestDistance = distance;
                closestStationId = station["id"].get<int>();
                closestStationName = QString::fromStdString(station["stationName"].get<std::string>());
            }
        }
    }

    // Rest of the function remains the same
    if (closestStationId != -1) {
        // Find the index in the comboBox for the given station
        int index = -1;
        for (int i = 0; i < ui->stationComboBox->count(); i++) {
            if (ui->stationComboBox->itemData(i).toInt() == closestStationId) {
                index = i;
                break;
            }
        }

        if (index != -1) {
            ui->stationComboBox->setCurrentIndex(index);
            ui->statusbar->showMessage(QString("Wybrano najbliższą stację: %1 (odległość: %2 km)")
                                           .arg(closestStationName)
                                           .arg(closestDistance / 1000.0, 0, 'f', 2), 5000);
        } else {
            QMessageBox::warning(this, "Błąd",
                                 "Znaleziono najbliższą stację, ale nie ma jej na liście.");
        }
    } else {
        QMessageBox::warning(this, "Informacja",
                             "Nie znaleziono stacji z danymi geograficznymi.");
    }
}
