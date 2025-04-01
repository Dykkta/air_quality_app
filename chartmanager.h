#ifndef CHARTMANAGER_H
#define CHARTMANAGER_H

#include <QObject>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QDateTime>
#include <QLayout>
#include <QListWidget>
#include <QLabel>
#include <map>
#include <set>
#include <string>


    class ChartManager : public QObject
{
    Q_OBJECT
public:
    explicit ChartManager(QObject *parent = nullptr);
    ~ChartManager();

    // Main functions for chart display
    void displayMultiParamChart(QLayout* chartLayout, QListWidget* paramCheckList,
                                QDateTime startDate, QDateTime endDate);

    // Function to add new parameter data
    void addParameterData(const std::string& paramName, const std::map<std::string, double>& results);

    // Function to clear all data
    void clearData();

    // Function to get available parameters
    const std::set<std::string>& getAvailableParams() const;

    // Function to update parameters list widget
    void updateParamCheckList(QListWidget* paramCheckList, const QString& currentParam);

    void connectParamCheckList(QListWidget* paramCheckList);
public slots:
    void onParamCheckStateChanged(QListWidgetItem* item);
private:
    // Data storage
    std::map<std::string, std::map<std::string, double>> m_allResults;
    std::set<std::string> m_availableParams;

    //Pola do odniesień do UI
    QLayout* m_currentChartLayout;
    QListWidget* m_currentParamCheckList;
    QDateTime m_startDate;
    QDateTime m_endDate;
};

#endif // CHARTMANAGER_H
