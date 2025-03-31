/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDateTimeEdit>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionLoadData;
    QAction *actionSaveData;
    QAction *actionExit;
    QAction *actionRefreshStations;
    QWidget *centralwidget;
    QHBoxLayout *mainHorizontalLayout;
    QWidget *leftPanel;
    QVBoxLayout *leftPanelLayout;
    QGroupBox *stationGroupBox;
    QHBoxLayout *horizontalLayout;
    QLabel *stationLabel;
    QComboBox *stationComboBox;
    QGroupBox *sensorGroupBox;
    QHBoxLayout *horizontalLayout_2;
    QLabel *sensorLabel;
    QComboBox *sensorComboBox;
    QPushButton *analyzeButton;
    QGroupBox *dateRangeGroupBox;
    QHBoxLayout *horizontalLayout_3;
    QLabel *startDateLabel;
    QDateTimeEdit *startDateTimeEdit;
    QLabel *endDateLabel;
    QDateTimeEdit *endDateTimeEdit;
    QGroupBox *parametersGroupBox;
    QHBoxLayout *horizontalLayout_4;
    QListWidget *paramCheckList;
    QGroupBox *analysisGroupBox;
    QGridLayout *gridLayout;
    QLabel *minValueLabel;
    QLineEdit *minValueEdit;
    QLabel *minDateLabel;
    QLineEdit *minDateEdit;
    QLabel *maxValueLabel;
    QLineEdit *maxValueEdit;
    QLabel *maxDateLabel;
    QLineEdit *maxDateEdit;
    QLabel *avgValueLabel;
    QLineEdit *avgValueEdit;
    QLabel *trendLabel;
    QLineEdit *trendEdit;
    QSpacerItem *verticalSpacer;
    QWidget *rightPanel;
    QVBoxLayout *rightPanelLayout;
    QGroupBox *chartGroupBox;
    QVBoxLayout *chartLayout;
    QGroupBox *mapGroupBox;
    QVBoxLayout *mapLayout;
    QLabel *mapPlaceholderLabel;
    QMenuBar *menubar;
    QMenu *menuFile;
    QMenu *menuTools;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1200, 800);
        actionLoadData = new QAction(MainWindow);
        actionLoadData->setObjectName("actionLoadData");
        actionSaveData = new QAction(MainWindow);
        actionSaveData->setObjectName("actionSaveData");
        actionExit = new QAction(MainWindow);
        actionExit->setObjectName("actionExit");
        actionRefreshStations = new QAction(MainWindow);
        actionRefreshStations->setObjectName("actionRefreshStations");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        mainHorizontalLayout = new QHBoxLayout(centralwidget);
        mainHorizontalLayout->setObjectName("mainHorizontalLayout");
        leftPanel = new QWidget(centralwidget);
        leftPanel->setObjectName("leftPanel");
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy.setHorizontalStretch(1);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(leftPanel->sizePolicy().hasHeightForWidth());
        leftPanel->setSizePolicy(sizePolicy);
        leftPanelLayout = new QVBoxLayout(leftPanel);
        leftPanelLayout->setObjectName("leftPanelLayout");
        stationGroupBox = new QGroupBox(leftPanel);
        stationGroupBox->setObjectName("stationGroupBox");
        horizontalLayout = new QHBoxLayout(stationGroupBox);
        horizontalLayout->setObjectName("horizontalLayout");
        stationLabel = new QLabel(stationGroupBox);
        stationLabel->setObjectName("stationLabel");

        horizontalLayout->addWidget(stationLabel);

        stationComboBox = new QComboBox(stationGroupBox);
        stationComboBox->setObjectName("stationComboBox");

        horizontalLayout->addWidget(stationComboBox);


        leftPanelLayout->addWidget(stationGroupBox);

        sensorGroupBox = new QGroupBox(leftPanel);
        sensorGroupBox->setObjectName("sensorGroupBox");
        horizontalLayout_2 = new QHBoxLayout(sensorGroupBox);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        sensorLabel = new QLabel(sensorGroupBox);
        sensorLabel->setObjectName("sensorLabel");

        horizontalLayout_2->addWidget(sensorLabel);

        sensorComboBox = new QComboBox(sensorGroupBox);
        sensorComboBox->setObjectName("sensorComboBox");

        horizontalLayout_2->addWidget(sensorComboBox);

        analyzeButton = new QPushButton(sensorGroupBox);
        analyzeButton->setObjectName("analyzeButton");

        horizontalLayout_2->addWidget(analyzeButton);


        leftPanelLayout->addWidget(sensorGroupBox);

        dateRangeGroupBox = new QGroupBox(leftPanel);
        dateRangeGroupBox->setObjectName("dateRangeGroupBox");
        horizontalLayout_3 = new QHBoxLayout(dateRangeGroupBox);
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        startDateLabel = new QLabel(dateRangeGroupBox);
        startDateLabel->setObjectName("startDateLabel");

        horizontalLayout_3->addWidget(startDateLabel);

        startDateTimeEdit = new QDateTimeEdit(dateRangeGroupBox);
        startDateTimeEdit->setObjectName("startDateTimeEdit");

        horizontalLayout_3->addWidget(startDateTimeEdit);

        endDateLabel = new QLabel(dateRangeGroupBox);
        endDateLabel->setObjectName("endDateLabel");

        horizontalLayout_3->addWidget(endDateLabel);

        endDateTimeEdit = new QDateTimeEdit(dateRangeGroupBox);
        endDateTimeEdit->setObjectName("endDateTimeEdit");

        horizontalLayout_3->addWidget(endDateTimeEdit);


        leftPanelLayout->addWidget(dateRangeGroupBox);

        parametersGroupBox = new QGroupBox(leftPanel);
        parametersGroupBox->setObjectName("parametersGroupBox");
        horizontalLayout_4 = new QHBoxLayout(parametersGroupBox);
        horizontalLayout_4->setObjectName("horizontalLayout_4");
        paramCheckList = new QListWidget(parametersGroupBox);
        paramCheckList->setObjectName("paramCheckList");

        horizontalLayout_4->addWidget(paramCheckList);


        leftPanelLayout->addWidget(parametersGroupBox);

        analysisGroupBox = new QGroupBox(leftPanel);
        analysisGroupBox->setObjectName("analysisGroupBox");
        gridLayout = new QGridLayout(analysisGroupBox);
        gridLayout->setObjectName("gridLayout");
        minValueLabel = new QLabel(analysisGroupBox);
        minValueLabel->setObjectName("minValueLabel");

        gridLayout->addWidget(minValueLabel, 0, 0, 1, 1);

        minValueEdit = new QLineEdit(analysisGroupBox);
        minValueEdit->setObjectName("minValueEdit");
        minValueEdit->setReadOnly(true);

        gridLayout->addWidget(minValueEdit, 0, 1, 1, 1);

        minDateLabel = new QLabel(analysisGroupBox);
        minDateLabel->setObjectName("minDateLabel");

        gridLayout->addWidget(minDateLabel, 0, 2, 1, 1);

        minDateEdit = new QLineEdit(analysisGroupBox);
        minDateEdit->setObjectName("minDateEdit");
        minDateEdit->setReadOnly(true);

        gridLayout->addWidget(minDateEdit, 0, 3, 1, 1);

        maxValueLabel = new QLabel(analysisGroupBox);
        maxValueLabel->setObjectName("maxValueLabel");

        gridLayout->addWidget(maxValueLabel, 1, 0, 1, 1);

        maxValueEdit = new QLineEdit(analysisGroupBox);
        maxValueEdit->setObjectName("maxValueEdit");
        maxValueEdit->setReadOnly(true);

        gridLayout->addWidget(maxValueEdit, 1, 1, 1, 1);

        maxDateLabel = new QLabel(analysisGroupBox);
        maxDateLabel->setObjectName("maxDateLabel");

        gridLayout->addWidget(maxDateLabel, 1, 2, 1, 1);

        maxDateEdit = new QLineEdit(analysisGroupBox);
        maxDateEdit->setObjectName("maxDateEdit");
        maxDateEdit->setReadOnly(true);

        gridLayout->addWidget(maxDateEdit, 1, 3, 1, 1);

        avgValueLabel = new QLabel(analysisGroupBox);
        avgValueLabel->setObjectName("avgValueLabel");

        gridLayout->addWidget(avgValueLabel, 2, 0, 1, 1);

        avgValueEdit = new QLineEdit(analysisGroupBox);
        avgValueEdit->setObjectName("avgValueEdit");
        avgValueEdit->setReadOnly(true);

        gridLayout->addWidget(avgValueEdit, 2, 1, 1, 1);

        trendLabel = new QLabel(analysisGroupBox);
        trendLabel->setObjectName("trendLabel");

        gridLayout->addWidget(trendLabel, 2, 2, 1, 1);

        trendEdit = new QLineEdit(analysisGroupBox);
        trendEdit->setObjectName("trendEdit");
        trendEdit->setReadOnly(true);

        gridLayout->addWidget(trendEdit, 2, 3, 1, 1);


        leftPanelLayout->addWidget(analysisGroupBox);

        verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        leftPanelLayout->addItem(verticalSpacer);


        mainHorizontalLayout->addWidget(leftPanel);

        rightPanel = new QWidget(centralwidget);
        rightPanel->setObjectName("rightPanel");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy1.setHorizontalStretch(2);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(rightPanel->sizePolicy().hasHeightForWidth());
        rightPanel->setSizePolicy(sizePolicy1);
        rightPanelLayout = new QVBoxLayout(rightPanel);
        rightPanelLayout->setObjectName("rightPanelLayout");
        chartGroupBox = new QGroupBox(rightPanel);
        chartGroupBox->setObjectName("chartGroupBox");
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Preferred);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(1);
        sizePolicy2.setHeightForWidth(chartGroupBox->sizePolicy().hasHeightForWidth());
        chartGroupBox->setSizePolicy(sizePolicy2);
        chartLayout = new QVBoxLayout(chartGroupBox);
        chartLayout->setObjectName("chartLayout");

        rightPanelLayout->addWidget(chartGroupBox);

        mapGroupBox = new QGroupBox(rightPanel);
        mapGroupBox->setObjectName("mapGroupBox");
        sizePolicy2.setHeightForWidth(mapGroupBox->sizePolicy().hasHeightForWidth());
        mapGroupBox->setSizePolicy(sizePolicy2);
        mapLayout = new QVBoxLayout(mapGroupBox);
        mapLayout->setObjectName("mapLayout");
        mapPlaceholderLabel = new QLabel(mapGroupBox);
        mapPlaceholderLabel->setObjectName("mapPlaceholderLabel");
        mapPlaceholderLabel->setAlignment(Qt::AlignCenter);

        mapLayout->addWidget(mapPlaceholderLabel);


        rightPanelLayout->addWidget(mapGroupBox);


        mainHorizontalLayout->addWidget(rightPanel);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1200, 25));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName("menuFile");
        menuTools = new QMenu(menubar);
        menuTools->setObjectName("menuTools");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menubar->addAction(menuTools->menuAction());
        menuFile->addAction(actionLoadData);
        menuFile->addAction(actionSaveData);
        menuFile->addSeparator();
        menuFile->addAction(actionExit);
        menuTools->addAction(actionRefreshStations);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "Pogoda i Jako\305\233\304\207 Powietrza w Polsce", nullptr));
        actionLoadData->setText(QCoreApplication::translate("MainWindow", "Wczytaj dane", nullptr));
        actionSaveData->setText(QCoreApplication::translate("MainWindow", "Zapisz dane", nullptr));
        actionExit->setText(QCoreApplication::translate("MainWindow", "Wyjd\305\272", nullptr));
        actionRefreshStations->setText(QCoreApplication::translate("MainWindow", "Od\305\233wie\305\274 list\304\231 stacji", nullptr));
        stationGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Wyb\303\263r stacji pomiarowej", nullptr));
        stationLabel->setText(QCoreApplication::translate("MainWindow", "Stacja:", nullptr));
        sensorGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Wyb\303\263r czujnika", nullptr));
        sensorLabel->setText(QCoreApplication::translate("MainWindow", "Czujnik:", nullptr));
        analyzeButton->setText(QCoreApplication::translate("MainWindow", "Analizuj dane", nullptr));
        dateRangeGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Zakres dat", nullptr));
        startDateLabel->setText(QCoreApplication::translate("MainWindow", "Od:", nullptr));
        endDateLabel->setText(QCoreApplication::translate("MainWindow", "Do:", nullptr));
        parametersGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Parametry do wy\305\233wietlenia", nullptr));
        analysisGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Wyniki analizy", nullptr));
        minValueLabel->setText(QCoreApplication::translate("MainWindow", "Warto\305\233\304\207 minimalna:", nullptr));
        minDateLabel->setText(QCoreApplication::translate("MainWindow", "Data:", nullptr));
        maxValueLabel->setText(QCoreApplication::translate("MainWindow", "Warto\305\233\304\207 maksymalna:", nullptr));
        maxDateLabel->setText(QCoreApplication::translate("MainWindow", "Data:", nullptr));
        avgValueLabel->setText(QCoreApplication::translate("MainWindow", "Warto\305\233\304\207 \305\233rednia:", nullptr));
        trendLabel->setText(QCoreApplication::translate("MainWindow", "Trend:", nullptr));
        chartGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Dane pomiarowe", nullptr));
        mapGroupBox->setTitle(QCoreApplication::translate("MainWindow", "Mapa", nullptr));
        mapPlaceholderLabel->setText(QCoreApplication::translate("MainWindow", "Miejsce na przysz\305\202\304\205 map\304\231", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "Plik", nullptr));
        menuTools->setTitle(QCoreApplication::translate("MainWindow", "Narz\304\231dzia", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
