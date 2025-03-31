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

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

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

        ui->statusbar->showMessage(QString("Pomyślnie pobrano dane o czujnikach dla stacji %1").arg(ui->stationComboBox->currentText()), 5000);

    } catch (const std::exception& e) {
        QMessageBox::warning(this, "Błąd połączenia",
                             QString("Nie udało się pobrać danych o czujnikach: %1\n\nPróba wczytania danych z lokalnej bazy.").arg(e.what()));

        // Próba wczytania danych offline
        nlohmann::json offlineData = Database::loadFromFile(stationFileName.toStdString());

        if (!offlineData.empty() && offlineData.is_array()) {
            updateSensorList(offlineData);
            ui->statusbar->showMessage(QString("Wczytano offline dane o czujnikach dla stacji %1").arg(ui->stationComboBox->currentText()), 5000);
        } else {
            QMessageBox::critical(this, "Błąd", "Brak zapisanych danych o czujnikach dla wybranej stacji.");
        }
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
    // Zapisujemy wyniki w globalnej mapie danych
    m_allResults[paramName.toStdString()] = results;

    // Aktualizujemy listę dostępnych parametrów
    m_availableParams.insert(paramName.toStdString());

    // Inicjalizacja lub aktualizacja listy kontrolnej parametrów
    if (ui->paramCheckList->count() == 0) {
        // Pierwszy parametr - inicjujemy listę kontrolną
        for (const auto& param : m_availableParams) {
            QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(param), ui->paramCheckList);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked); // Domyślnie zaznaczamy nowy parametr
        }

        // Łączymy sygnał zmiany z aktualizacją wykresu (tylko raz)
        connect(ui->paramCheckList, &QListWidget::itemChanged, this, [this](QListWidgetItem* item) {
            // Wywołanie funkcji odświeżającej wykres
            displayMultiParamChart();
        });
    } else {
        // Sprawdzamy, czy ten parametr już istnieje na liście
        bool found = false;
        for (int i = 0; i < ui->paramCheckList->count(); ++i) {
            QListWidgetItem* item = ui->paramCheckList->item(i);
            if (item->text() == paramName) {
                found = true;
                item->setCheckState(Qt::Checked); // Zaznaczamy aktualnie analizowany parametr
                break;
            }
        }

        // Jeśli nie znaleziono, dodajemy nowy
        if (!found) {
            QListWidgetItem* item = new QListWidgetItem(paramName, ui->paramCheckList);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked); // Domyślnie zaznaczamy nowy parametr
        }
    }

    // Wyświetl wykres z wybranymi parametrami
    displayMultiParamChart();
}
void MainWindow::displayMultiParamChart() {
    // Czyścimy istniejący układ wykresu
    QLayoutItem *child;
    while ((child = ui->chartLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }

    // Filtracja danych według zakresu dat z kontrolek
    QDateTime startDate = ui->startDateTimeEdit->dateTime();
    QDateTime endDate = ui->endDateTimeEdit->dateTime();

    // Utworzenie wykresu
    QChart *chart = new QChart();
    chart->setTitle(QString("Pomiary parametrów"));
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    // Zmienne do śledzenia zakresu danych
    double minValue = std::numeric_limits<double>::max();
    double maxValue = std::numeric_limits<double>::lowest();
    QDateTime firstDate, lastDate;

    // Kolory dla różnych serii danych
    QList<QColor> colors = {Qt::red, Qt::blue, Qt::green, Qt::magenta, Qt::cyan,
                            Qt::yellow, Qt::darkRed, Qt::darkBlue, Qt::darkGreen,
                            Qt::darkMagenta, Qt::darkCyan};
    int colorIndex = 0;

    // Sprawdzamy które parametry są zaznaczone
    bool anySeriesAdded = false;

    for (int i = 0; i < ui->paramCheckList->count(); ++i) {
        QListWidgetItem* item = ui->paramCheckList->item(i);
        if (item->checkState() == Qt::Checked) {
            QString paramName = item->text();
            std::string paramNameStd = paramName.toStdString();

            // Sprawdzamy czy parametr istnieje w danych
            if (m_allResults.find(paramNameStd) == m_allResults.end()) {
                continue;
            }

            const auto& results = m_allResults.at(paramNameStd);

            QLineSeries *series = new QLineSeries();
            series->setName(paramName);

            // Ustawienie koloru dla serii
            QColor color = colors[colorIndex % colors.size()];
            series->setColor(color);
            colorIndex++;

            // Przygotowanie danych do wykresu
            std::vector<std::pair<QDateTime, double>> filteredData;

            for (const auto& [dateStr, value] : results) {
                QDateTime date = QDateTime::fromString(QString::fromStdString(dateStr), Qt::ISODate);

                // Sprawdź, czy data mieści się w wybranym zakresie
                if (date >= startDate && date <= endDate && value != 0.0) {  // Pomijamy wartości null (zamienione na 0.0)
                    filteredData.push_back({date, value});

                    // Aktualizacja zakresów
                    if (value < minValue) minValue = value;
                    if (value > maxValue) maxValue = value;

                    if (firstDate.isNull() || date < firstDate) firstDate = date;
                    if (lastDate.isNull() || date > lastDate) lastDate = date;
                }
            }

            // Sortujemy dane według czasu
            std::sort(filteredData.begin(), filteredData.end(),
                      [](const auto& a, const auto& b) { return a.first < b.first; });

            // Dodajemy posortowane dane do serii
            for (const auto& [date, value] : filteredData) {
                series->append(date.toMSecsSinceEpoch(), value);
            }

            chart->addSeries(series);
            anySeriesAdded = true;
        }
    }

    // Jeśli nie ma danych do wyświetlenia, zakończ
    if (!anySeriesAdded) {
        QLabel *noDataLabel = new QLabel("Brak danych do wyświetlenia w wybranym zakresie dat lub nie wybrano parametrów.");
        noDataLabel->setAlignment(Qt::AlignCenter);
        ui->chartLayout->addWidget(noDataLabel);
        delete chart;
        return;
    }

    // Utworzenie osi czasu
    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setFormat("dd.MM.yyyy\nHH:mm"); // Format z datą i godziną w orientacji pionowej
    axisX->setTitleText("Data pomiaru");
    axisX->setLabelsAngle(90); // Ustawienie pionowej orientacji etykiet

    // Zapewnienie lepszego formatowania osi czasu
    if (!firstDate.isNull() && !lastDate.isNull()) {
        // Dodaj małe marginesy do zakresu czasowego
        axisX->setRange(firstDate.addSecs(-3600), lastDate.addSecs(3600));

        // Ustaw odpowiednią liczbę znaczników na osi w zależności od zakresu czasu
        qint64 timeRangeHours = firstDate.secsTo(lastDate) / 3600;
        if (timeRangeHours <= 24) {
            axisX->setTickCount(qMin(int(timeRangeHours) + 1, 12)); // Co godzinę lub rzadziej dla krótkich zakresów
        } else if (timeRangeHours <= 72) {
            axisX->setTickCount(qMin(int(timeRangeHours / 3) + 1, 12)); // Co 3 godziny dla średnich zakresów
        } else {
            axisX->setTickCount(qMin(int(timeRangeHours / 6) + 1, 12)); // Co 6 godzin dla długich zakresów
        }
    }

    chart->addAxis(axisX, Qt::AlignBottom);

    // Utworzenie osi wartości z lepszym zakresem
    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Wartość");

    // Ustawienie zakresu wartości z marginesem
    if (minValue != std::numeric_limits<double>::max() && maxValue != std::numeric_limits<double>::lowest()) {
        double margin = (maxValue - minValue) * 0.1; // 10% marginesu
        if (margin < 0.1) margin = 1.0; // W przypadku małej różnicy

        axisY->setRange(minValue - margin, maxValue + margin);

        // Automatycznie dobierz liczbę znaczników na osi Y
        double range = maxValue - minValue;
        if (range <= 10) {
            axisY->setTickCount(qMin(int(range) + 2, 10));
        } else {
            axisY->setTickCount(10);
        }

        // Ustawienie formatu liczb, aby wyświetlać pełne wartości
        axisY->setLabelFormat("%.2f");
    }

    chart->addAxis(axisY, Qt::AlignLeft);

    // Podłączenie serii danych do osi
    for (QAbstractSeries *series : chart->series()) {
        series->attachAxis(axisX);
        series->attachAxis(axisY);
    }

    // Dodanie siatki dla lepszej czytelności
    axisX->setGridLineVisible(true);
    axisY->setGridLineVisible(true);

    // Utworzenie widoku wykresu
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Dodanie widoku do układu
    ui->chartLayout->addWidget(chartView);
}

// Funkcja do odświeżania wykresu (możemy ją podłączyć do przycisku lub innych kontrolek)
void MainWindow::refreshChart() {
    displayMultiParamChart();
}
// Funkcja do inicjalizacji listy kontrolnej parametrów
void MainWindow::initParamCheckList(const std::set<std::string>& availableParams) {
    ui->paramCheckList->clear();

    for (const auto& param : availableParams) {
        QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(param), ui->paramCheckList);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Unchecked);
    }

    // Połączenie sygnału zmiany stanu checkboxa z odświeżeniem wykresu
    connect(ui->paramCheckList, &QListWidget::itemChanged, this, [this](QListWidgetItem* item) {
        // Wywołanie funkcji odświeżającej wykres
        refreshChart();
    });
}


void MainWindow::displayStatistics(const DataAnalyzer& analyzer) {
    const auto& results = analyzer.getAnalysisResults();

    // Znajdź minimalne i maksymalne wartości
    double minValue = 999999.0;
    double maxValue = -999999.0;
    std::string minDate, maxDate;
    double sum = 0.0;
    int count = 0;

    // Sprawdź, czy klasa DataAnalyzer zaimplementowała te metody
    if (!analyzer.getMinDate().empty() && !analyzer.getMaxDate().empty()) {
        // Jeśli metody są zaimplementowane, użyj ich
        ui->minValueEdit->setText(QString::number(std::get<0>(results.at(analyzer.getMinDate()))));
        ui->maxValueEdit->setText(QString::number(std::get<0>(results.at(analyzer.getMaxDate()))));
        ui->minDateEdit->setText(QString::fromStdString(analyzer.getMinDate()));
        ui->maxDateEdit->setText(QString::fromStdString(analyzer.getMaxDate()));
    } else {
        // W przeciwnym razie oblicz statystyki ręcznie
        for (const auto& [date, stats] : results) {
            double value = std::get<0>(stats);  // Pierwsza wartość to wartość pomiaru
            if (value < minValue) {
                minValue = value;
                minDate = date;
            }
            if (value > maxValue) {
                maxValue = value;
                maxDate = date;
            }
            sum += value;
            count++;
        }

        if (count > 0) {
            ui->minValueEdit->setText(QString::number(minValue));
            ui->maxValueEdit->setText(QString::number(maxValue));
            ui->minDateEdit->setText(QString::fromStdString(minDate));
            ui->maxDateEdit->setText(QString::fromStdString(maxDate));
            ui->avgValueEdit->setText(QString::number(sum / count));
        }
    }

    // Obliczenie trendu
    if (results.size() > 1) {
        // Pobierz pierwszy i ostatni pomiar
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
// Implementation of the distance calculation method
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

// Function to get location from IP using curl
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
