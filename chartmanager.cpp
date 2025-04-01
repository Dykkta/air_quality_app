#include "ChartManager.h"
#include <QChartView>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QDateTime>
#include <QLayoutItem>
#include <QLabel>

ChartManager::ChartManager(QObject *parent) : QObject(parent),
    //aby wykres aktualizował sie po kazdej zmianie parametrow do wyswietlenia
    m_currentChartLayout(nullptr),
    m_currentParamCheckList(nullptr)
{
}

ChartManager::~ChartManager()
{
}

//aby wykres aktualizował sie po kazdej zmianie parametrow do wyswietlenia
void ChartManager::connectParamCheckList(QListWidget* paramCheckList)
{
    if (!paramCheckList)
        return;

    m_currentParamCheckList = paramCheckList;

    // Połącz sygnał zmiany elementu z odpowiednim slotem
    connect(paramCheckList, &QListWidget::itemChanged, this, &ChartManager::onParamCheckStateChanged);
}

//aby wykres aktualizował sie po kazdej zmianie parametrow do wyswietlenia
void ChartManager::onParamCheckStateChanged(QListWidgetItem* item)
{
    // Odśwież wykres jeśli mamy wszystkie potrzebne dane
    if (m_currentChartLayout && m_currentParamCheckList &&
        m_startDate.isValid() && m_endDate.isValid()) {
        displayMultiParamChart(m_currentChartLayout, m_currentParamCheckList, m_startDate, m_endDate);
    }
}
void ChartManager::addParameterData(const std::string& paramName, const std::map<std::string, double>& results)
{
    // Save results in the global data map
    m_allResults[paramName] = results;

    // Update the list of available parameters
    m_availableParams.insert(paramName);
}

const std::set<std::string>& ChartManager::getAvailableParams() const
{
    return m_availableParams;
}

void ChartManager::clearData()
{
    m_allResults.clear();
    m_availableParams.clear();
}

void ChartManager::updateParamCheckList(QListWidget* paramCheckList, const QString& currentParam)
{
    if (!paramCheckList)
        return;

    std::string currentParamStd = currentParam.toStdString();

    // First time initialization of the check list
    if (paramCheckList->count() == 0) {
        // Initialize the check list with available parameters
        for (const auto& param : m_availableParams) {
            QListWidgetItem* item = new QListWidgetItem(QString::fromStdString(param), paramCheckList);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked); // Default to checked for new parameter
        }
    } else {
        // Check if this parameter already exists in the list
        bool found = false;
        for (int i = 0; i < paramCheckList->count(); ++i) {
            QListWidgetItem* item = paramCheckList->item(i);
            if (item->text() == currentParam) {
                found = true;
                item->setCheckState(Qt::Checked); // Check the currently analyzed parameter
                break;
            }
        }

        // If not found, add new
        if (!found) {
            QListWidgetItem* item = new QListWidgetItem(currentParam, paramCheckList);
            item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
            item->setCheckState(Qt::Checked); // Default to checked for new parameter
        }
    }
}

void ChartManager::displayMultiParamChart(QLayout* chartLayout, QListWidget* paramCheckList,
                                          QDateTime startDate, QDateTime endDate)
{
    if (!chartLayout || !paramCheckList)
        return;

    // Zapisz odniesienia do bieżącego stanu //aby wykres aktualizował sie po kazdej zmianie parametrow do wyswietlenia
    m_currentChartLayout = chartLayout;
    m_currentParamCheckList = paramCheckList;
    m_startDate = startDate;
    m_endDate = endDate;

    // Clear existing chart layout
    QLayoutItem *child;
    while ((child = chartLayout->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }

    // Create chart
    QChart *chart = new QChart();
    chart->setTitle(QString("Pomiary parametrów"));
    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    // Variables to track data range and axes for parameters
    QDateTime firstDate, lastDate;

    // Map to store series and axes per parameter
    std::map<std::string, QLineSeries*> paramSeries;
    std::map<std::string, QValueAxis*> paramAxes;
    std::map<std::string, std::pair<double, double>> paramRanges; // min, max

    // Colors for different data series
    QList<QColor> colors = {Qt::red, Qt::blue, Qt::green, Qt::magenta, Qt::cyan,
                            Qt::yellow, Qt::darkRed, Qt::darkBlue, Qt::darkGreen,
                            Qt::darkMagenta, Qt::darkCyan};
    int colorIndex = 0;

    // Check which parameters are selected and prepare data
    bool anySeriesAdded = false;

    for (int i = 0; i < paramCheckList->count(); ++i) {
        QListWidgetItem* item = paramCheckList->item(i);
        if (item->checkState() == Qt::Checked) {
            QString paramName = item->text();
            std::string paramNameStd = paramName.toStdString();

            // Check if parameter exists in the data
            if (m_allResults.find(paramNameStd) == m_allResults.end()) {
                continue;
            }

            const auto& results = m_allResults.at(paramNameStd);

            QLineSeries *series = new QLineSeries();
            series->setName(paramName);

            // Set color for the series
            QColor color = colors[colorIndex % colors.size()];
            series->setColor(color);
            colorIndex++;

            // Initialize min/max for this parameter
            double minValue = std::numeric_limits<double>::max();
            double maxValue = std::numeric_limits<double>::lowest();

            // Prepare data for the chart
            std::vector<std::pair<QDateTime, double>> filteredData;

            for (const auto& [dateStr, value] : results) {
                QDateTime date = QDateTime::fromString(QString::fromStdString(dateStr), Qt::ISODate);

                // Check if date is within the selected range
                if (date >= startDate && date <= endDate && value != 0.0) {  // Skip null values (replaced with 0.0)
                    filteredData.push_back({date, value});

                    // Update ranges
                    if (value < minValue) minValue = value;
                    if (value > maxValue) maxValue = value;

                    if (firstDate.isNull() || date < firstDate) firstDate = date;
                    if (lastDate.isNull() || date > lastDate) lastDate = date;
                }
            }

            // Store parameter range
            if (minValue != std::numeric_limits<double>::max() && maxValue != std::numeric_limits<double>::lowest()) {
                // Add margin to min/max
                double margin = (maxValue - minValue) * 0.1; // 10% margin
                if (margin < 0.1) margin = 1.0; // In case of small difference

                paramRanges[paramNameStd] = {minValue - margin, maxValue + margin};
            }

            // Sort data by time
            std::sort(filteredData.begin(), filteredData.end(),
                      [](const auto& a, const auto& b) { return a.first < b.first; });

            // Add sorted data to the series
            for (const auto& [date, value] : filteredData) {
                series->append(date.toMSecsSinceEpoch(), value);
            }

            chart->addSeries(series);
            paramSeries[paramNameStd] = series;
            anySeriesAdded = true;
        }
    }

    // If there's no data to display, finish
    if (!anySeriesAdded) {
        QLabel *noDataLabel = new QLabel("Brak danych do wyświetlenia w wybranym zakresie dat lub nie wybrano parametrów.");
        noDataLabel->setAlignment(Qt::AlignCenter);
        chartLayout->addWidget(noDataLabel);
        delete chart;
        return;
    }

    // Create time axis (common for all parameters)
    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setFormat("dd.MM.yyyy"); // Format with date only, no time
    axisX->setTitleText("Data pomiaru");
    axisX->setLabelsAngle(45); // Angle to make text more readable

    // Ensure better time axis formatting
    if (!firstDate.isNull() && !lastDate.isNull()) {
        // Add small margins to the time range
        axisX->setRange(firstDate.addSecs(-3600), lastDate.addSecs(3600));

        // Set appropriate number of ticks on the axis depending on the time range
        qint64 timeRangeDays = firstDate.daysTo(lastDate);
        if (timeRangeDays <= 7) {
            axisX->setTickCount(timeRangeDays + 1); // One tick per day for short ranges
        } else if (timeRangeDays <= 31) {
            axisX->setTickCount(qMin(int(timeRangeDays / 2) + 1, 15)); // Every other day for medium ranges
        } else {
            axisX->setTickCount(qMin(int(timeRangeDays / 7) + 1, 12)); // Weekly for long ranges
        }
    }

    chart->addAxis(axisX, Qt::AlignBottom);

    // Create value axes for each parameter
    int axisCount = 0;
    for (const auto& [paramName, series] : paramSeries) {
        QValueAxis *axisY = new QValueAxis();
        QString paramQStr = QString::fromStdString(paramName);
        axisY->setTitleText(paramQStr);

        // Set color matching the series
        axisY->setLabelsColor(series->color());
        axisY->setTitleBrush(QBrush(series->color()));

        // Set value range
        if (paramRanges.find(paramName) != paramRanges.end()) {
            double minVal = paramRanges[paramName].first;
            double maxVal = paramRanges[paramName].second;
            axisY->setRange(minVal, maxVal);

            // Automatically select number of ticks on Y axis
            double range = maxVal - minVal;
            if (range <= 10) {
                axisY->setTickCount(qMin(int(range) + 2, 10));
            } else {
                axisY->setTickCount(10);
            }

            // Set number format to display full values
            axisY->setLabelFormat("%.2f");
        }

        // Alternate sides for Y axes (left or right)
        // First two on the left, rest on the right to prevent overlapping
        if (axisCount < 2) {
            chart->addAxis(axisY, Qt::AlignLeft);
        } else {
            chart->addAxis(axisY, Qt::AlignRight);
        }

        // Connect only this series to its axis
        series->attachAxis(axisX);
        series->attachAxis(axisY);

        // Store axis for parameter
        paramAxes[paramName] = axisY;
        axisCount++;
    }

    // Add grid for better readability
    for (const auto& [paramName, axis] : paramAxes) {
        axis->setGridLineVisible(true);
        axis->setShadesPen(Qt::NoPen); // No border for shades
        axis->setShadesBrush(QBrush(QColor(0, 0, 0, 0))); // Transparent background
    }
    axisX->setGridLineVisible(true);

    // Create chart view
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // Add view to layout
    chartLayout->addWidget(chartView);
}
