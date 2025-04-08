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

/**
 * @brief Konstruktor klasy MainWindow
 * @param parent - Wskaźnik do widgetu nadrzędnego
 *
 * Inicjalizuje główne okno aplikacji, tworzy menedżera wykresów,
 * konfiguruje interfejs użytkownika oraz łączy sygnały ze slotami.
 */
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
    connect(ui->analyzeButton, &QPushButton::clicked, this, &MainWindow::onAnalyzeData);
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

/**
 * @brief Inicjalizuje interfejs użytkownika
 *
 * Konfiguruje główne okno aplikacji, ustawia rozmiary paneli,
 * inicjalizuje kontrolki dat oraz łączy je z funkcją odświeżania wykresu.
 */
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

/**
 * @brief Destruktor klasy MainWindow
 *
 * Zwalnia zasoby alokowane podczas tworzenia obiektu klasy MainWindow.
 */
MainWindow::~MainWindow() {
    delete ui;
}

/**
 * @brief Inicjalizuje listę stacji pomiarowych
 *
 * Pobiera dane o stacjach pomiarowych z API GIOŚ lub wczytuje je z lokalnej bazy danych,
 * jeśli nie ma połączenia z internetem. Po pobraniu danych aktualizuje interfejs użytkownika.
 */
void MainWindow::initializeStations() {
    if (!HttpClient::isNetworkAvailable()) {
        QMessageBox::information(this, "Brak połączenia",
                                 "Brak połączenia internetowego. Wczytuję dane offline.");
        tryLoadOfflineStations();
        return;
    }

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
        QMessageBox::warning(this, "Błąd",
                             QString("Błąd połączenia: %1. Wczytuję dane offline.").arg(e.what()));
        tryLoadOfflineStations();
    }
}

/**
 * @brief Próbuje wczytać dane o stacjach z lokalnej bazy danych
 *
 * Funkcja wyświetla tylko te stacje, dla których dostępne są dane pomiarowe offline.
 * Jest wywoływana, gdy nie można pobrać danych z API.
 */
void MainWindow::tryLoadOfflineStations() {
    try {
        // Wczytaj listę wszystkich stacji z lokalnego pliku
        nlohmann::json allStations = Database::loadFromFile("stations_data.json");

        if (allStations.empty() || !allStations.is_array()) {
            throw std::runtime_error("Brak zapisanych danych o stacjach.");
        }

        // Utwórz nową listę stacji, która będzie zawierać tylko te z dostępnymi danymi
        nlohmann::json availableStations = nlohmann::json::array();

        // Sprawdź każdą stację, czy ma dane offline
        for (const auto& station : allStations) {
            // Sprawdź, czy stacja ma czujniki z danymi
            bool hasOfflineData = false;

            // Pobierz ID stacji
            int stationId = station.at("id").get<int>();

            // Wczytaj listę czujników dla tej stacji (jeśli plik istnieje)
            QString sensorsFileName = QString("station_%1_sensors.json").arg(stationId);

            // Sprawdź, czy plik z czujnikami istnieje
            if (QFile::exists(sensorsFileName)) {
                nlohmann::json sensors = Database::loadFromFile(sensorsFileName.toStdString());

                // Sprawdź, czy którykolwiek z czujników ma dane offline
                if (sensors.is_array()) {
                    for (const auto& sensor : sensors) {
                        int sensorId = sensor.at("id").get<int>();
                        QString dataFileName = QString("sensor_%1_data.json").arg(sensorId);

                        // Jeśli istnieje plik z danymi dla tego czujnika
                        if (QFile::exists(dataFileName)) {
                            hasOfflineData = true;
                            break;
                        }
                    }
                }
            }

            // Dodaj stację do listy dostępnych, jeśli ma dane offline
            if (hasOfflineData) {
                availableStations.push_back(station);
            }
        }

        // Jeśli nie ma dostępnych stacji z danymi offline
        if (availableStations.empty()) {
            QMessageBox::warning(this, "Brak danych",
                                 "Nie znaleziono stacji z danymi offline.");
            return;
        }

        // Aktualizuj interfejs użytkownika tylko dostępnymi stacjami
        updateStationsComboBox(availableStations);
        ui->statusbar->showMessage(QString("Wczytano %1 stacji z danymi offline.").arg(availableStations.size()), 5000);

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Błąd",
                              QString("Błąd podczas wczytywania danych offline: %1").arg(e.what()));
    }
}

/**
 * @brief Sprawdza czy dla danego czujnika istnieją dane offline
 * @param sensorId - Identyfikator czujnika
 * @return true jeśli dane istnieją, false w przeciwnym wypadku
 *
 * Funkcja sprawdza czy plik z danymi dla określonego czujnika istnieje
 * i czy zawiera poprawne dane w formacie JSON.
 */
bool MainWindow::hasSensorOfflineData(int sensorId) {
    QString fileName = QString("sensor_%1_data.json").arg(sensorId);
    QFile file(fileName);

    if (file.exists()) {
        // Spróbuj otworzyć i sprawdzić zawartość
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray data = file.readAll();
            file.close();

            // Sprawdź, czy dane są poprawnym JSON i nie są puste
            try {
                JsonHandler jsonHandler;
                nlohmann::json jsonData = jsonHandler.parseJson(data.toStdString());

                // Sprawdź, czy dane zawierają odpowiednie wartości
                if (jsonData.contains("values") && jsonData["values"].is_array() && !jsonData["values"].empty()) {
                    return true;
                }
            } catch (...) {
                // Jeśli wystąpił błąd parsowania, to dane są uszkodzone
                return false;
            }
        }
    }

    return false;
}

/**
 * @brief Wczytuje i wyświetla czujniki dla wybranej stacji
 * @param stationId - Identyfikator stacji
 *
 * Funkcja wczytuje dane o czujnikach dla określonej stacji z pliku
 * i aktualizuje listę czujników w interfejsie użytkownika.
 * W trybie offline pokazuje tylko czujniki z dostępnymi danymi.
 */
void MainWindow::loadSensorsForStation(int stationId) {
    ui->sensorComboBox->clear();

    QString fileName = QString("station_%1_sensors.json").arg(stationId);

    if (!QFile::exists(fileName)) {
        return;  // Brak danych o czujnikach dla tej stacji
    }

    try {
        nlohmann::json sensors = Database::loadFromFile(fileName.toStdString());

        if (!sensors.is_array()) {
            return;
        }

        bool isOffline = !HttpClient::isNetworkAvailable();

        for (const auto& sensor : sensors) {
            int sensorId = sensor.at("id").get<int>();
            std::string paramName = sensor.at("param").at("paramName").get<std::string>();

            // W trybie offline dodaj tylko czujniki z dostępnymi danymi
            if (!isOffline || hasSensorOfflineData(sensorId)) {
                ui->sensorComboBox->addItem(QString::fromStdString(paramName), sensorId);
            }
        }
    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Błąd",
                             QString("Nie udało się wczytać danych czujników: %1").arg(e.what()));
    }
}

/**
 * @brief Aktualizuje kontrolkę ComboBox ze stacjami
 * @param stations - Obiekt JSON zawierający dane o stacjach
 *
 * Funkcja czyści obecną listę stacji, sortuje je alfabetycznie według nazwy
 * i dodaje do kontrolki ComboBox wraz z identyfikatorami jako dane użytkownika.
 */
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

/**
 * @brief Obsługuje zmianę wybranej stacji
 * @param index - Indeks nowo wybranej stacji w ComboBox
 *
 * Funkcja pobiera dane o czujnikach dla wybranej stacji z API lub
 * z lokalnej bazy danych, aktualizuje listę czujników i czyści dane wykresu.
 */
void MainWindow::onStationChanged(int index) {
    if (index < 0) return;

    // Pobierz ID stacji z danych elementu comboBox
    int stationId = ui->stationComboBox->itemData(index).toInt();

    // Wczytaj dane czujników dla wybranej stacji
    loadSensorsForStation(stationId);

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

    if (!HttpClient::isNetworkAvailable()) {
        // Wczytaj dane offline
        QString fileName = QString("station_%1_sensors.json").arg(stationId);
        nlohmann::json offlineData = Database::loadFromFile(fileName.toStdString());
        updateSensorList(offlineData);
        fetchDataForAllSensors();
        return;
    }

    try {
        HttpClient client;
        std::string url = "https://api.gios.gov.pl/pjp-api/rest/station/sensors/" + std::to_string(stationId);
        std::string jsonData = client.getRequest(url);

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

/**
 * @brief Pobiera dane dla wszystkich czujników wybranej stacji
 *
 * Funkcja pobiera dane pomiarowe dla każdego czujnika z API lub
 * wczytuje je z lokalnej bazy danych, analizuje je i aktualizuje
 * menedżera wykresów oraz listę dostępnych parametrów.
 */
void MainWindow::fetchDataForAllSensors() {
    bool isOnline = HttpClient::isNetworkAvailable();

    //Sprawdź czy lista czujników zawiera elementy
    if (ui->sensorComboBox->count() <= 0) {
        return;
    }

    // Dla każdego czujnika pobierz dane
    for (int i = 0; i < ui->sensorComboBox->count(); i++) {
        int sensorId = ui->sensorComboBox->itemData(i).toInt();
        QString paramName = ui->sensorComboBox->itemText(i);

        try {
            if(isOnline){
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
            } else{
                throw std::runtime_error("Offline mode");
            }

        } catch (const std::exception& e) {
            // Wczytaj dane z pliku
            QString fileName = QString("sensor_%1_data.json").arg(sensorId);
            nlohmann::json offlineData = Database::loadFromFile(fileName.toStdString());
            // ... przetwarzanie danych offline ..

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

/**
 * @brief Obsługuje żądanie analizy danych
 *
 * Funkcja jest wywoływana po kliknięciu przycisku "Analizuj".
 * Pobiera dane dla wybranego czujnika z API lub z lokalnej bazy danych,
 * analizuje je i wyświetla wyniki analizy oraz wykres.
 */
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


    if (!HttpClient::isNetworkAvailable()) {
        // Wczytaj dane z lokalnego pliku
        QString fileName = QString("sensor_%1_data.json").arg(sensorId);
        nlohmann::json offlineData = Database::loadFromFile(fileName.toStdString());
        processAndDisplayData(offlineData, paramName);
        return;
    }

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

/**
 * @brief Aktualizuje listę czujników
 * @param data - Obiekt JSON zawierający dane o czujnikach
 *
 * Funkcja czyści obecną listę czujników i dodaje nowe dane
 * z obiektu JSON do kontrolki ComboBox.
 */
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

/**
 * @brief Przetwarza i wyświetla dane pomiarowe
 * @param data - Obiekt JSON zawierający dane pomiarowe
 * @param paramName - Nazwa parametru pomiarowego
 *
 * Funkcja analizuje dane pomiarowe, wyświetla je na wykresie
 * oraz pokazuje statystyki takie jak wartość średnia, minimalna i maksymalna.
 */
void MainWindow::processAndDisplayData(const nlohmann::json& data, const QString& paramName) {
    // Najpierw czyścimy istniejący układ wykresu
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

/**
 * @brief Wyświetla wyniki analizy danych
 * @param results - Mapa zawierająca wyniki analizy
 * @param paramName - Nazwa parametru pomiarowego
 *
 * Funkcja dodaje dane do menedżera wykresów, aktualizuje listę parametrów
 * i wyświetla wykres dla wybranych parametrów.
 */
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

/**
 * @brief Odświeża wykres
 *
 * Funkcja wyświetla wykres z aktualnymi danymi dla wybranych parametrów
 * w wybranym zakresie czasowym. Jest wywoływana po zmianie zakresu dat.
 */
void MainWindow::refreshChart() {
    // Just call the ChartManager to display the chart
    m_chartManager->displayMultiParamChart(ui->chartLayout, ui->paramCheckList,
                                           ui->startDateTimeEdit->dateTime(),
                                           ui->endDateTimeEdit->dateTime());
}


/**
 * @brief Wyświetla statystyki analizy danych
 *
 * Funkcja oblicza i wyświetla w interfejsie użytkownika statystyki na podstawie danych
 * z analizatora, takie jak: wartość średnia, wartości minimalne i maksymalne,
 * daty wystąpienia wartości minimalnych i maksymalnych oraz trend zmian wartości.
 *
 * @param analyzer Obiekt analizatora danych zawierający wyniki analizy
 */
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
        ui->avgValueEdit->setText(QString::number(sum / count, 'f', 1));
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

/**
 * @brief Oblicza odległość między dwoma współrzędnymi geograficznymi
 *
 * Funkcja wykorzystuje formułę Haversine do obliczenia odległości między dwoma
 * punktami na powierzchni Ziemi określonymi przez ich współrzędne geograficzne.
 *
 * @param other Współrzędne punktu docelowego
 * @return Odległość w metrach między bieżącym punktem a punktem docelowym
 */
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

/**
 * @brief Pobiera położenie geograficzne na podstawie adresu IP
 *
 * Funkcja używa zewnętrznego API GIOS do określenia położenia geograficznego
 * użytkownika na podstawie jego adresu IP. Wyniki są pobierane za pomocą polecenia curl
 * i zapisywane w pliku tymczasowym, który następnie jest przetwarzany.
 *
 * @return Obiekt GeoCoordinate zawierający współrzędne geograficzne (szerokość i długość)
 */
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

/**
 * @brief Obsługuje żądanie znalezienia najbliższej stacji pomiarowej
 *
 * Funkcja wywoływana po kliknięciu przycisku wyszukiwania najbliższej stacji.
 * Sprawdza dostępność sieci, pobiera aktualną lokalizację użytkownika,
 * a następnie przeszukuje listę stacji pomiarowych, aby znaleźć najbliższą.
 * Jeśli automatyczne określenie lokalizacji nie jest możliwe, prosi użytkownika
 * o ręczne wprowadzenie współrzędnych.
 */
void MainWindow::onFindNearestStation() {

    if (!HttpClient::isNetworkAvailable()) {
        QMessageBox::information(this, "Offline",
                                 "Wyszukiwanie stacji w trybie offline jest niemożliwe. \nSpróbuj wybrać stację ręcznie.");
        return;
    }
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

/**
 * @brief Znajduje najbliższą stację pomiarową na podstawie współrzędnych geograficznych
 *
 * Funkcja analizuje listę stacji pomiarowych i znajduje tę, która jest najbliżej
 * podanej lokalizacji. Odległość jest oblicq   zana za pomocą metody distanceTo klasy
 * GeoCoordinate. Po znalezieniu najbliższej stacji, jest ona wybierana w interfejsie
 * użytkownika.
 *
 * @param currentPosition Aktualna pozycja geograficzna, względem której szukamy najbliższej stacji
 * @param stations Lista dostępnych stacji pomiarowych w formacie JSON
 */
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


/**
 * @brief Zamyka aplikację
 *
 *
 */
void MainWindow::on_actionExit_triggered() {
    QApplication::quit();
}
