#pragma once
#include <QObject>
#include <QListWidget>
#include <QLayout>
#include <map>
#include <set>
#include <string>
#include <QDateTime>

/**
 * @brief Klasa zarządzająca wykresami danych pomiarowych
 *
 * ChartManager odpowiada za przechowywanie, zarządzanie i wyświetlanie danych pomiarowych
 * na wykresach. Umożliwia dodawanie różnych parametrów, kontrolę wyświetlanych zakresów
 * dat oraz interaktywną selekcję parametrów do wyświetlenia.
 */
class ChartManager : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief Konstruktor klasy ChartManager
     * @param parent Obiekt rodzica (dla zarządzania pamięcią)
     */
    explicit ChartManager(QObject *parent = nullptr);

    /**
     * @brief Destruktor klasy ChartManager
     */
    ~ChartManager();

    /**
     * @brief Dodaje dane pomiarowe dla określonego parametru
     * @param paramName Nazwa parametru
     * @param results Mapa wyników (data -> wartość)
     */
    void addParameterData(const std::string& paramName, const std::map<std::string, double>& results);

    /**
     * @brief Ustawia jednostkę dla parametru
     * @param paramName Nazwa parametru
     * @param unit Jednostka miary (np. "μg/m³")
     */
    void setParameterUnit(const std::string& paramName, const std::string& unit);

    /**
     * @brief Pobiera jednostkę dla parametru
     * @param paramName Nazwa parametru
     * @return Jednostka miary parametru
     */
    std::string getParameterUnit(const std::string& paramName) const;

    /**
     * @brief Zwraca zbiór dostępnych parametrów
     * @return Referencja do zbioru nazw dostępnych parametrów
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
     * @brief Podłącza sygnały zmiany stanu parametrów
     * @param paramCheckList Lista wyboru parametrów do monitorowania
     */
    void connectParamCheckList(QListWidget* paramCheckList);

private slots:
    /**
     * @brief Obsługuje zmianę stanu zaznaczenia parametru
     * @param item Element listy, którego stan się zmienił
     */
    void onParamCheckStateChanged(QListWidgetItem* item);

private:
    /**
     * @brief Mapa przechowująca wszystkie wyniki pomiarów
     *
     * Struktura: nazwa parametru -> (data -> wartość)
     */
    std::map<std::string, std::map<std::string, double>> m_allResults;

    /**
     * @brief Zbiór dostępnych parametrów
     */
    std::set<std::string> m_availableParams;

    /**
     * @brief Wskaźnik na aktualny układ wykresu
     */
    QLayout* m_currentChartLayout;

    /**
     * @brief Wskaźnik na aktualną listę wyboru parametrów
     */
    QListWidget* m_currentParamCheckList;

    /**
     * @brief Data początkowa zakresu wyświetlania
     */
    QDateTime m_startDate;

    /**
     * @brief Data końcowa zakresu wyświetlania
     */
    QDateTime m_endDate;
};
