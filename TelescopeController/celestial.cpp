#include "celestial.h"
#include <math.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif

static double deg2rad(double d) { return d * PI / 180.0; }
static double rad2deg(double r) { return r * 180.0 / PI; }
static double wrap360(double x) {
  while (x < 0) x += 360.0;
  while (x >= 360.0) x -= 360.0;
  return x;
}
static double wrap24(double h) {
  while (h < 0) h += 24.0;
  while (h >= 24.0) h -= 24.0;
  return h;
}

// Bright stars + popular deep-sky (approx J2000 RA hours / Dec degrees)
// const data lives in flash on ESP32
static const CelestialObject CATALOG[] = {
  // Sun / Moon / Planets — coords filled at runtime
  {"Sun",     ObjectKind::Sun,     0, 0, -26.7f},
  {"Moon",    ObjectKind::Moon,    0, 0, -12.6f},
  {"Mercury", ObjectKind::Planet,  0, 0,  0.0f},
  {"Venus",   ObjectKind::Planet,  0, 0, -4.0f},
  {"Mars",    ObjectKind::Planet,  0, 0,  1.0f},
  {"Jupiter", ObjectKind::Planet,  0, 0, -2.0f},
  {"Saturn",  ObjectKind::Planet,  0, 0,  0.5f},
  {"Uranus",  ObjectKind::Planet,  0, 0,  5.5f},
  {"Neptune", ObjectKind::Planet,  0, 0,  7.8f},

  // Bright stars
  {"Sirius",      ObjectKind::Star,  6.7525f, -16.7161f, -1.46f},
  {"Canopus",     ObjectKind::Star,  6.3992f, -52.6956f, -0.74f},
  {"Arcturus",    ObjectKind::Star, 14.2610f,  19.1824f, -0.05f},
  {"Vega",        ObjectKind::Star, 18.6156f,  38.7837f,  0.03f},
  {"Capella",     ObjectKind::Star,  5.2782f,  45.9980f,  0.08f},
  {"Rigel",       ObjectKind::Star,  5.2423f,  -8.2016f,  0.13f},
  {"Procyon",     ObjectKind::Star,  7.6550f,   5.2250f,  0.34f},
  {"Betelgeuse",  ObjectKind::Star,  5.9195f,   7.4071f,  0.42f},
  {"Achernar",    ObjectKind::Star,  1.6286f, -57.2367f,  0.46f},
  {"Hadar",       ObjectKind::Star, 14.0637f, -60.3730f,  0.61f},
  {"Altair",      ObjectKind::Star, 19.8463f,   8.8683f,  0.76f},
  {"Aldebaran",   ObjectKind::Star,  4.5987f,  16.5093f,  0.86f},
  {"Antares",    ObjectKind::Star, 16.4901f, -26.4320f,  0.96f},
  {"Spica",       ObjectKind::Star, 13.4199f, -11.1613f,  0.97f},
  {"Pollux",      ObjectKind::Star,  7.7553f,  28.0262f,  1.14f},
  {"Fomalhaut",   ObjectKind::Star, 22.9608f, -29.6222f,  1.16f},
  {"Deneb",       ObjectKind::Star, 20.6905f,  45.2803f,  1.25f},
  {"Regulus",     ObjectKind::Star, 10.1395f,  11.9672f,  1.35f},
  {"Adhara",      ObjectKind::Star,  6.9771f, -28.9721f,  1.50f},
  {"Castor",      ObjectKind::Star,  7.5766f,  31.8883f,  1.58f},
  {"Shaula",      ObjectKind::Star, 17.5601f, -37.1038f,  1.62f},
  {"Bellatrix",   ObjectKind::Star,  5.4189f,   6.3497f,  1.64f},
  {"Elnath",      ObjectKind::Star,  5.4382f,  28.6075f,  1.65f},
  {"Miaplacidus", ObjectKind::Star,  9.2200f, -69.7172f,  1.67f},
  {"Alnilam",     ObjectKind::Star,  5.6036f,  -1.2019f,  1.69f},
  {"Alnair",      ObjectKind::Star, 22.1372f, -46.9609f,  1.74f},
  {"Alioth",      ObjectKind::Star, 12.9004f,  55.9598f,  1.77f},
  {"Alnitak",     ObjectKind::Star,  5.6793f,  -1.9426f,  1.77f},
  {"Dubhe",       ObjectKind::Star, 11.0621f,  61.7510f,  1.79f},
  {"Mirfak",      ObjectKind::Star,  3.4054f,  49.8612f,  1.79f},
  {"Wezen",       ObjectKind::Star,  7.1399f, -26.3932f,  1.83f},
  {"Sadr",        ObjectKind::Star, 20.3705f,  40.2567f,  2.23f},
  {"Acrux",       ObjectKind::Star, 12.4433f, -63.0991f,  0.77f},
  {"Gacrux",      ObjectKind::Star, 12.5194f, -57.1132f,  1.63f},
  {"Polaris",     ObjectKind::Star,  2.5303f,  89.2641f,  1.98f},

  // Popular deep-sky
  {"M1 Crab",     ObjectKind::DeepSky,  5.5756f,  22.0145f,  8.4f},
  {"M13 Hercules",ObjectKind::DeepSky, 16.6948f,  36.4603f,  5.8f},
  {"M31 Andromeda",ObjectKind::DeepSky, 0.7123f,  41.2692f,  3.4f},
  {"M42 Orion",   ObjectKind::DeepSky,  5.5880f,  -5.3911f,  4.0f},
  {"M45 Pleiades",ObjectKind::DeepSky,  3.7903f,  24.1053f,  1.6f},
  {"M51 Whirlpool",ObjectKind::DeepSky,13.4978f,  47.1952f,  8.4f},
  {"M57 Ring",    ObjectKind::DeepSky, 18.8931f,  33.0292f,  8.8f},
  {"M81 Bode",    ObjectKind::DeepSky,  9.9259f,  69.0653f,  6.9f},
  {"M104 Sombrero",ObjectKind::DeepSky,12.6665f, -11.6231f,  8.0f},
  {"NGC5139 OmegaCen", ObjectKind::DeepSky, 13.4472f, -47.4794f, 3.9f},
};

void Celestial::beginDefaults(ObserverSite& site) {
  site.latDeg = 40.0f;
  site.lonDeg = -105.0f;
  site.tzMinutes = -360;
  site.year = 2026;
  site.month = 8;
  site.day = 6;
  site.hour = 21;
  site.minute = 0;
  site.second = 0;
}

size_t Celestial::catalogSize() {
  return sizeof(CATALOG) / sizeof(CATALOG[0]);
}

const CelestialObject& Celestial::catalogAt(size_t i) {
  return CATALOG[i];
}

int Celestial::findByName(const char* name) {
  size_t n = catalogSize();
  for (size_t i = 0; i < n; i++) {
    if (strcasecmp(CATALOG[i].name, name) == 0) return (int)i;
  }
  return -1;
}

double Celestial::julianDate(const ObserverSite& site) {
  // Local civil → approximate UTC
  int h = site.hour;
  int mi = site.minute;
  int s = site.second;
  int totalMin = h * 60 + mi - site.tzMinutes;
  int dayAdj = 0;
  while (totalMin < 0) { totalMin += 1440; dayAdj--; }
  while (totalMin >= 1440) { totalMin -= 1440; dayAdj++; }
  h = totalMin / 60;
  mi = totalMin % 60;

  int Y = site.year;
  int M = site.month;
  int D = site.day + dayAdj;
  // normalize day roughly
  while (D < 1) { M--; D += 30; if (M < 1) { M = 12; Y--; } }
  while (D > 31) { D -= 30; M++; if (M > 12) { M = 1; Y++; } }

  if (M <= 2) { Y -= 1; M += 12; }
  int A = Y / 100;
  int B = 2 - A + A / 4;
  double jd = floor(365.25 * (Y + 4716)) + floor(30.6001 * (M + 1)) + D + B - 1524.5;
  jd += (h + mi / 60.0 + s / 3600.0) / 24.0;
  return jd;
}

double Celestial::gmstHours(double jd) {
  double T = (jd - 2451545.0) / 36525.0;
  double gmst = 280.46061837 + 360.98564736629 * (jd - 2451545.0)
              + 0.000387933 * T * T - T * T * T / 38710000.0;
  gmst = wrap360(gmst) / 15.0;
  return gmst;
}

static SkyCoords eclipticToEquatorial(double lonDeg, double latDeg, double oblDeg) {
  double l = deg2rad(lonDeg);
  double b = deg2rad(latDeg);
  double e = deg2rad(oblDeg);
  double ra = atan2(sin(l) * cos(e) - tan(b) * sin(e), cos(l));
  double dec = asin(sin(b) * cos(e) + cos(b) * sin(e) * sin(l));
  SkyCoords c;
  c.raHours = wrap24(rad2deg(ra) / 15.0);
  c.decDeg = (float)rad2deg(dec);
  c.valid = true;
  return c;
}

SkyCoords Celestial::sun(const ObserverSite& site) {
  double jd = julianDate(site);
  double n = jd - 2451545.0;
  double L = wrap360(280.460 + 0.9856474 * n);
  double g = wrap360(357.528 + 0.9856003 * n);
  double gr = deg2rad(g);
  double lambda = wrap360(L + 1.915 * sin(gr) + 0.020 * sin(2 * gr));
  double eps = 23.439 - 0.0000004 * n;
  return eclipticToEquatorial(lambda, 0.0, eps);
}

SkyCoords Celestial::moon(const ObserverSite& site) {
  // Low-precision lunar ecliptic longitude/latitude (good to ~1–2°)
  double jd = julianDate(site);
  double d = jd - 2451545.0;
  double N = wrap360(125.1228 - 0.0529538083 * d);
  double i = 5.1454;
  double w = wrap360(318.0634 + 0.1643573223 * d);
  double a = 60.2666; // Earth radii — unused for unit sphere direction
  (void)a;
  double e = 0.054900;
  double M = wrap360(115.3654 + 13.0649929509 * d);
  double Mr = deg2rad(M);
  double E = Mr + e * sin(Mr) * (1.0 + e * cos(Mr));
  double xv = cos(E) - e;
  double yv = sqrt(1.0 - e * e) * sin(E);
  double v = rad2deg(atan2(yv, xv));
  double r = sqrt(xv * xv + yv * yv);
  (void)r;
  double l = wrap360(v + w);
  double Nr = deg2rad(N);
  double ir = deg2rad(i);
  double lr = deg2rad(l);
  double xh = cos(Nr) * cos(lr) - sin(Nr) * sin(lr) * cos(ir);
  double yh = sin(Nr) * cos(lr) + cos(Nr) * sin(lr) * cos(ir);
  double zh = sin(lr) * sin(ir);
  double lon = wrap360(rad2deg(atan2(yh, xh)));
  double lat = rad2deg(atan2(zh, sqrt(xh * xh + yh * yh)));
  double n = d;
  double eps = 23.439 - 0.0000004 * n;
  return eclipticToEquatorial(lon, lat, eps);
}

// Simplified mean orbital elements for planets (Meeus-ish low precision)
struct PlanetEl {
  double L0, L1; // mean longitude deg, deg/day from J2000
  double a;      // AU
  double e;
  double i0;
  double w0;     // perihelion
  double N0;     // ascending node
};

static const PlanetEl PLANETS[] = {
  // Mercury .. Neptune — rough elements
  {252.2509, 4.09233445, 0.387098, 0.205630, 7.005, 77.456, 48.332},
  {181.9798, 1.60213034, 0.723330, 0.006773, 3.395,131.603, 76.680},
  {355.4533, 0.98560028, 1.000000, 0.016710, 0.000,102.937,  0.000}, // Earth (for elongation)
  { 34.3515, 0.52403304, 1.523688, 0.093405, 1.850,336.060, 49.579},
  { 50.0774, 0.08308529, 5.20256,  0.048498, 1.303, 14.331,100.464},
  { 64.1335, 0.03350387, 9.55475,  0.055546, 2.489, 92.599,113.665},
  {318.3190, 0.01173084,19.18171,  0.047318, 0.773,170.954, 74.006},
  {300.3240, 0.00598149,30.05826,  0.008606, 1.770, 44.965,131.784},
};

static void helioPos(const PlanetEl& p, double d, double& x, double& y, double& z) {
  double N = deg2rad(wrap360(p.N0));
  double i = deg2rad(p.i0);
  double w = deg2rad(wrap360(p.w0));
  double a = p.a;
  double e = p.e;
  double M = wrap360(p.L0 + p.L1 * d - p.w0 - p.N0);
  double Mr = deg2rad(M);
  double E = Mr + e * sin(Mr) * (1.0 + e * cos(Mr));
  for (int k = 0; k < 4; k++) {
    E = E - (E - e * sin(E) - Mr) / (1.0 - e * cos(E));
  }
  double xv = a * (cos(E) - e);
  double yv = a * (sqrt(1 - e * e) * sin(E));
  double v = atan2(yv, xv);
  double r = sqrt(xv * xv + yv * yv);
  x = r * (cos(N) * cos(v + w) - sin(N) * sin(v + w) * cos(i));
  y = r * (sin(N) * cos(v + w) + cos(N) * sin(v + w) * cos(i));
  z = r * (sin(v + w) * sin(i));
}

SkyCoords Celestial::planet(int planetIndex, const ObserverSite& site) {
  // planetIndex: 0=Mercury,1=Venus,2=Mars,3=Jupiter,4=Saturn,5=Uranus,6=Neptune
  // Map to PLANETS[] skipping Earth at index 2
  static const int mapIdx[] = {0, 1, 3, 4, 5, 6, 7};
  if (planetIndex < 0 || planetIndex > 6) {
    SkyCoords bad{0, 0, false};
    return bad;
  }
  double jd = julianDate(site);
  double d = jd - 2451545.0;
  double xe, ye, ze, xp, yp, zp;
  helioPos(PLANETS[2], d, xe, ye, ze);
  helioPos(PLANETS[mapIdx[planetIndex]], d, xp, yp, zp);
  double x = xp - xe;
  double y = yp - ye;
  double z = zp - ze;
  double lon = wrap360(rad2deg(atan2(y, x)));
  double lat = rad2deg(atan2(z, sqrt(x * x + y * y)));
  double eps = 23.439 - 0.0000004 * d;
  return eclipticToEquatorial(lon, lat, eps);
}

SkyCoords Celestial::compute(const CelestialObject& obj, const ObserverSite& site) {
  switch (obj.kind) {
    case ObjectKind::Sun:  return sun(site);
    case ObjectKind::Moon: return moon(site);
    case ObjectKind::Planet: {
      int idx = -1;
      if (!strcasecmp(obj.name, "Mercury")) idx = 0;
      else if (!strcasecmp(obj.name, "Venus")) idx = 1;
      else if (!strcasecmp(obj.name, "Mars")) idx = 2;
      else if (!strcasecmp(obj.name, "Jupiter")) idx = 3;
      else if (!strcasecmp(obj.name, "Saturn")) idx = 4;
      else if (!strcasecmp(obj.name, "Uranus")) idx = 5;
      else if (!strcasecmp(obj.name, "Neptune")) idx = 6;
      return planet(idx, site);
    }
    default: {
      SkyCoords c;
      c.raHours = obj.raHours;
      c.decDeg = obj.decDeg;
      c.valid = true;
      return c;
    }
  }
}
