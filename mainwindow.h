#pragma once
#include <QMainWindow>
#include <nlohmann/json.hpp>
#include <map>
#include <string>
#include "ChartManager.h"
#include "DataAnalyzer.h"

/**
 * @struct GeoCoordinate
 * @brief Struktura przechowująca współrzędne geograficzne
 *
 * Struktura zawiera pola dla szerokości i długości geograficznej
 * oraz metodę do obliczania odległości między dwoma punktami.
 */
struct GeoCoordinate {
    double latitude;  ///< Szerokość geograficzna w stopniach
    double longitude; ///< Długość geograficzna w stopniach

    /**
     * @brief Oblicza odległość między dwoma współrzędnymi geograficznymi
     * @param other Współrzędne punktu docelowego
     * @return Odległość w metrach między bieżącym punktem a punktem docelowym
     */
    double distanceTo(const GeoCoordinate& other) const;
};

namespace Ui {
class MainWindow;
}

/**
 * @class MainWindow
 * @brief Główne okno aplikacji
 *
 * Klasa odpowiedzialna za wyświetlanie interfejsu użytkownika,
 * pobieranie danych z API GIOŚ oraz ich wizualizację.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Konstruktor klasy MainWindow
     * @param parent - Wskaźnik do widgetu nadrzędnego
     *
     * Inicjalizuje główne okno aplikacji, tworzy menedżera wykresów,
     * konfiguruje interfejs użytkownika oraz łączy sygnały ze slotami.
     */
    explicit MainWindow(QWidget *parent = nullptr);

    /**
     * @brief Destruktor klasy MainWindow
     *
     * Zwalnia zasoby alokowane podczas tworzenia obiektu klasy MainWindow.
     */
    ~MainWindow();

private slots:
    /**
     * @brief Inicjalizuje interfejs użytkownika
     *
     * Konfiguruje główne okno aplikacji, ustawia rozmiary paneli,
     * inicjalizuje kontrolki dat oraz łączy je z funkcją odświeżania wykresu.
     */
    void initializeUI();

    /**
     * @brief Inicjalizuje listę stacji pomiarowych
     *
     * Pobiera dane o stacjach pomiarowych z API GIOŚ lub wczytuje je z lokalnej bazy danych,
     * jeśli nie ma połączenia z internetem. Po pobraniu danych aktualizuje interfejs użytkownika.
     */
    void initializeStations();

    /**
     * @brief Obsługuje zmianę wybranej stacji
     * @param index - Indeks nowo wybranej stacji w ComboBox
     *
     * Funkcja pobiera dane o czujnikach dla wybranej stacji z API lub
     * z lokalnej bazy danych, aktualizuje listę czujników i czyści dane wykresu.
     */
    void onStationChanged(int index);

    /**
     * @brief Obsługuje żądanie analizy danych
     *
     * Funkcja jest wywoływana po kliknięciu przycisku "Analizuj".
     * Pobiera dane dla wybranego czujnika z API lub z lokalnej bazy danych,
     * analizuje je i wyświetla wyniki analizy oraz wykres.
     */
    void onAnalyzeData();

    /**
     * @brief Odświeża wykres
     *
     * Funkcja wyświetla wykres z aktualnymi danymi dla wybranych parametrów
     * w wybranym zakresie czasowym. Jest wywoływana po zmianie zakresu dat.
     */
    void refreshChart();

    /**
     * @brief Obsługuje żądanie znalezienia najbliższej stacji pomiarowej
     *
     * Funkcja wywoływana po kliknięciu przycisku wyszukiwania najbliższej stacji.
     * Sprawdza dostępność sieci, pobiera aktualną lokalizację użytkownika,
     * a następnie przeszukuje listę stacji pomiarowych, aby znaleźć najbliższą.
     */
    void onFindNearestStation();

    /**
     * @brief Obsługuje akcję wyjścia z aplikacji
     */
    void on_actionExit_triggered();

private:
    /**
     * @brief Próbuje wczytać dane o stacjach z lokalnej bazy danych
     *
     * Funkcja wyświetla tylko te stacje, dla których dostępne są dane pomiarowe offline.
     * Jest wywoływana, gdy nie można pobrać danych z API.
     */
    void tryLoadOfflineStations();

    /**
     * @brief Sprawdza czy dla danego czujnika istnieją dane offline
     * @param sensorId - Identyfikator czujnika
     * @return true jeśli dane istnieją, false w przeciwnym wypadku
     *
     * Funkcja sprawdza czy plik z danymi dla określonego czujnika istnieje
     * i czy zawiera poprawne dane w formacie JSON.
     */
    bool hasSensorOfflineData(int sensorId);

    /**
     * @brief Wczytuje i wyświetla czujniki dla wybranej stacji
     * @param stationId - Identyfikator stacji
     *
     * Funkcja wczytuje dane o czujnikach dla określonej stacji z pliku
     * i aktualizuje listę czujników w interfejsie użytkownika.
     */
    void loadSensorsForStation(int stationId);

    /**
     * @brief Aktualizuje kontrolkę ComboBox ze stacjami
     * @param stations - Obiekt JSON zawierający dane o stacjach
     *
     * Funkcja czyści obecną listę stacji, sortuje je alfabetycznie według nazwy
     * i dodaje do kontrolki ComboBox wraz z identyfikatorami jako dane użytkownika.
     */
    void updateStationsComboBox(const nlohmann::json& stations);

    /**
     * @brief Pobiera dane dla wszystkich czujników wybranej stacji
     *
     * Funkcja pobiera dane pomiarowe dla każdego czujnika z API lub
     * wczytuje je z lokalnej bazy danych, analizuje je i aktualizuje
     * menedżera wykresów oraz listę dostępnych parametrów.
     */
    void fetchDataForAllSensors();

    /**
     * @brief Aktualizuje listę czujników
     * @param data - Obiekt JSON zawierający dane o czujnikach
     *
     * Funkcja czyści obecną listę czujników i dodaje nowe dane
     * z obiektu JSON do kontrolki ComboBox.
     */
    void updateSensorList(const nlohmann::json& data);

    /**
     * @brief Przetwarza i wyświetla dane pomiarowe
     * @param data - Obiekt JSON zawierający dane pomiarowe
     * @param paramName - Nazwa parametru pomiarowego
     *
     * Funkcja analizuje dane pomiarowe, wyświetla je na wykresie
     * oraz pokazuje statystyki takie jak wartość średnia, minimalna i maksymalna.
     */
    void processAndDisplayData(const nlohmann::json& data, const QString& paramName);

    /**
     * @brief Wyświetla wyniki analizy danych
     * @param results - Mapa zawierająca wyniki analizy
     * @param paramName - Nazwa parametru pomiarowego
     *
     * Funkcja dodaje dane do menedżera wykresów, aktualizuje listę parametrów
     * i wyświetla wykres dla wybranych parametrów.
     */
    void displayAnalysisResults(const std::map<std::string, double>& results, const QString& paramName);

    /**
     * @brief Wyświetla statystyki analizy danych
     * @param analyzer - Analizator danych zawierający wyniki analizy
     *
     * Funkcja oblicza i wyświetla w interfejsie użytkownika statystyki
     * takie jak wartość średnia, minimalna i maksymalna.
     */
    void displayStatistics(const DataAnalyzer& analyzer);

    /**
     * @brief Pobiera położenie geograficzne na podstawie adresu IP
     * @return Współrzędne geograficzne aktualnej lokalizacji
     *
     * Funkcja używa zewnętrznego API do określenia położenia geograficznego
     * użytkownika na podstawie jego adresu IP.
     */
    GeoCoordinate getLocationFromIP();

    /**
     * @brief Znajduje najbliższą stację pomiarową na podstawie współrzędnych geograficznych
     * @param currentPosition - Aktualna pozycja geograficzna
     * @param stations - Lista dostępnych stacji pomiarowych
     *
     * Funkcja analizuje listę stacji pomiarowych i znajduje tę, która jest najbliżej
     * podanej lokalizacji.
     */
    void findNearestStationWithCoordinates(const GeoCoordinate& currentPosition, const nlohmann::json& stations);

    Ui::MainWindow *ui;                 ///< Wskaźnik do interfejsu użytkownika
    ChartManager *m_chartManager;       ///< Menedżer wykresów
};
