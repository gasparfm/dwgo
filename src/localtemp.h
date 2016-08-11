#include <string.h>
#include <strings.h>
#include "errors.h"

#define METAR_URL               "http://weather.noaa.gov/pub/data/observations/metar/decoded/%s.TXT"

class localtemp {
public: 
  enum ESky
    {
      undef_sky,
      clear,			// CAVOK, SKC, CLR
      cloudy,			// FEW, SCT
      brkovc,			// Broken, Overcast (BKN, OVC)
      tcu		        // TCU (Towering CUmulus)
    };
  enum ERain
    {
      undef_rain,
      rain,			// Rainy (DZ, RA)
      snow,			// Snowy (SN, SG)
      hail			// Hail (GS, GR)
    };
  enum EFog
    {
      undef_fog,
      fog,			// (BR, FG)
      dust,			// (DU, DS)
      other			// (FC, FU, HZ, SA, SS, VA)
    };

  struct TMetar
  {
    ESky sky;
    bool CB;			// Cumulonimbus
    ERain rain;
    EFog fog;
  };
  std::string metar;
  std::string location_name;
  std::string long_location;
  std::string ob;			// ob: line, METAR info.
  int error, celsius, fahrenheit;
  int theme;			// Âº theme to use
  short humidity;
  bool loaded;
  string sky;
  TMetar mInfo;
  time_t info_time;		// Time stored in file
  time_t get_time;		// Time when we got the file
  localtemp(char* metar, char* location_name);
  bool getInfo();
private:
  void set_temp(std::string temp);
  void set_humidity(std::string hum);
  void get_ob_info();
};
