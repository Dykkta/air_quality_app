/**
 * @file main.cpp
 * @brief Punkt wejścia do aplikacji monitorującej jakość powietrza
 * @details Inicjalizuje aplikację Qt, tworzy i wyświetla główne okno aplikacji
 */

#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>

/**
 * @brief Funkcja główna programu
 * @param argc Liczba argumentów wiersza poleceń
 * @param argv Tablica argumentów wiersza poleceń
 * @return Kod wyjścia aplikacji
 * @details Tworzy instancję QApplication, inicjalizuje główne okno aplikacji
 *          i uruchamia główną pętlę zdarzeń Qt
 */
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();  // Uruchomienie głównej pętli aplikacji
}
