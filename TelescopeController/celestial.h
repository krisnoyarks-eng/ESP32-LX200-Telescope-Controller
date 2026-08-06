#pragma once
#include <Arduino.h>

enum class ObjectKind : uint8_t {
  Star = 0,
  Planet,
  Sun,
  Moon,
  DeepSky
};

struct CelestialObject {
  const char* name;
  ObjectKind kind;
  // For fixed objects (stars / deep-sky): J2000-ish catalog coords
  float raHours;   // 0..24
  float decDeg;    // -90..90
  float mag;       // apparent magnitude (lower = brighter)
};

struct ObserverSite {
  float latDeg;
  float lonDeg;
  int tzMinutes;     // UTC offset
  int year;
  int month;
  int day;
  int hour;          // local civil hour 0-23
  int minute;
  int second;
};

struct SkyCoords {
  float raHours;
  float decDeg;
  bool valid;
};

class Celestial {
public:
  static void beginDefaults(ObserverSite& site);
  static size_t catalogSize();
  static const CelestialObject& catalogAt(size_t i);
  static int findByName(const char* name);  // index or -1

  // Compute instantaneous RA/Dec for Sun/Moon/planets; stars copy catalog
  static SkyCoords compute(const CelestialObject& obj, const ObserverSite& site);

  // Julian date helpers
  static double julianDate(const ObserverSite& site);
  static double gmstHours(double jd);

private:
  static SkyCoords sun(const ObserverSite& site);
  static SkyCoords moon(const ObserverSite& site);
  static SkyCoords planet(int planetIndex, const ObserverSite& site); // 0=Merc..6=Nep
};
