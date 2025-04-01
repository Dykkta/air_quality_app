#ifndef GEOLOCATIONMANAGER_H
#define GEOLOCATIONMANAGER_H

#include <string>
#include <nlohmann/json.hpp>
#include "JsonHandler.h"
#include <QString>
#include <QDir>
#include <QFile>
#include <QMessageBox>

struct GeoCoordinate {
    double latitude;
    double longitude;
    double distanceTo(const GeoCoordinate& other) const;
};

class GeoLocationManager {
public:
    GeoLocationManager();
    ~GeoLocationManager();

    // Get user's current location based on IP
    static GeoCoordinate getLocationFromIP();

    // Find the nearest station from the provided list
    static int findNearestStation(const GeoCoordinate& currentPosition,
                                  const nlohmann::json& stations,
                                  QString& stationName,
                                  double& distance);

    // Alternative method for manual coordinate input
    static bool parseCoordinatesFromString(const QString& input, GeoCoordinate& result);
};

#endif // GEOLOCATIONMANAGER_H
