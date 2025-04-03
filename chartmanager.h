#pragma once
#include <QObject>
#include <QListWidget>
#include <QLayout>
#include <map>
#include <set>
#include <string>
#include <QDateTime>

/**
 * @class ChartManager
 * @brief Klasa zarządzająca wyświetlaniem wykresów danych jakości powietrza
 *
 * Klasa ChartManager dostarcza funkcjonalność do przechowywania, przetwarzania
 * i wizualizacji danych pomiarowych jakości powietrza. Umożliwia wyświetlanie
 * wielu parametrów na jednym wykresie, z automatycznym doborem kolorów i osi.
 * Zapewnia interaktywne funkcje jak podpowiedzi po najechaniu na punkty danych.
 */
class ChartManager : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Konstruktor klasy ChartManager
     * @param parent Wskaźnik na obiekt rodzica w hierarchii Qt
     */
    explicit ChartManager(QObject *parent = nullptr);

    /**
     * @brief Destruktor klasy ChartManager
     */
    ~ChartManager();

    /**
     * @brief Dodaje dane dla określonego parametru
     * @param paramName Nazwa parametru
     * @param results Mapa wyników zawierająca pary data-wartość dla parametru
     */
    void addParameterData(const std::string& paramName, const std::map<std::string, double>& results);
    /**
     * @brief Zwraca zbiór dostępnych parametrów
     * @return Referencja do zbioru zawierającego nazwy wszystkich dostępnych parametrów
     */
    const std::set<std::string>& getAvailableParams() const;

    /**
     * @brief Czyści wszystkie dane pomiarowe
     */
    void clearData();

    /**
     * @brief Aktualizuje listę wyboru parametrów
     * @param paramCheckList Widget listy parametrów do aktualizacji
     * @param currentParam Aktualnie wybrany parametr
     */
    void updateParamCheckList(QListWidget* paramCheckList, const QString& currentParam);

    /**
     * @brief Wyświetla wykres z wieloma parametrami
     * @param chartLayout Układ, w którym ma być wyświetlony wykres
     * @param paramCheckList Lista wyboru parametrów
     * @param startDate Data początkowa zakresu
     * @param endDate Data końcowa zakresu
     */
    void displayMultiParamChart(QLayout* chartLayout, QListWidget* paramCheckList,
                                QDateTime startDate, QDateTime endDate);

    /**
     * @brief Łączy listę wyboru parametrów z menedżerem wykresów
     * @param paramCheckList Wskaźnik na widget listy parametrów
     */
    void connectParamCheckList(QListWidget* paramCheckList);

private slots:
    /**
     * @brief Slot wywoływany przy zmianie stanu zaznaczenia parametru
     * @param item Wskaźnik na element listy, którego stan się zmienił
     */
    void onParamCheckStateChanged(QListWidgetItem* item);

private:
    std::map<std::string, std::map<std::string, double>> m_allResults; ///< Mapa wszystkich wyników dla parametrów
    std::set<std::string> m_availableParams; ///< Zbiór nazw dostępnych parametrów
    QLayout* m_currentChartLayout; ///< Wskaźnik na aktualnie używany układ wykresu
    QListWidget* m_currentParamCheckList; ///< Wskaźnik na aktualnie używaną listę parametrów
    QDateTime m_startDate; ///< Data początkowa zakresu danych
    QDateTime m_endDate; ///< Data końcowa zakresu danych
};
