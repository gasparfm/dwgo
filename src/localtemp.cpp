 /*******************************************************************************
 *  File: localtemp.cpp                  					*
 *  Version: 0.3            							*
 *  Date: 20080508                    						*
 *  Author: Gaspar Fernández (helyo@totaki.com) 				*
 *										*
 *  Copyright (C) 2008   Gaspar Fernández					*
 *										*
 *  This program is free software: you can redistribute it and/or modify	*
 *  it under the terms of the GNU General Public License as published by	*
 *  the Free Software Foundation, either version 3 of the License, or 		*
 *  (at your option) any later version.  	      	  	   		*
 *										*
 *  This program is distributed in the hope that it will be useful,		*
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of		*
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the		*
 *  GNU General Public License for more details.				*
 *										*
 *  You should have received a copy of the GNU General Public License		*
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.	*
 *									        *
 ********************************************************************************
 *   Description:
 *     It is part of a Window Maker dockapp called dwgo. This class reads weather
 *  of a selected location from National Weather Service (http://weather.noaa.gov/) 
 *  and parses some information to display it formatted on screen.  
 *     I know it parses unnecesary information, but I'm planning to update dwgo
 *  with some improvements that use all this information.
 *
 *   Change History:
 *    Date (D.M.Y)    Author              Modification
 *    08.05.2008      Gaspar Fernández    Initial release
 *    31.10.2010      Gaspar Fernández    Bug Corrections
 *    
 ********************************************************************************/  

#include "localtemp.h"
#include "MySock.h"
#include "dwgo.h"		// Theme numbers

using namespace std;

/*************************************************************
 *     Constructur: localtemp                                *
 *************************************************************
 *  Description:                                             *
 *     Initializes variables                                 *
 *                                                           *
 * Input:                                                    *
 *    char *metar - ICAO name locator (4 chars)              *
 *    char location_name - Name of this location             *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
localtemp::localtemp(char *metar, char *location_name)
{
  this->metar=metar;
  this->location_name=location_name;
  this->celsius=0;
  this->fahrenheit=0;
  this->loaded=false;
  this->theme=DEFAULT_THEME;
}

/*************************************************************
 *     Method: set_temp                                      *
 *************************************************************
 *  Description:                                             *
 *     Extract temperature information. In Celsius and       *
 *  Fahrenheit                                               *
 *                                                           *
 * Input:                                                    *
 *    string tmp_str - Temporary string containing           *
 *  temperature in Celsius and Fahrenheit                    *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void localtemp::set_temp(string tmp_str)
{
  char buf[8],buf3[8];
  // Format xx F (xx C)
  sscanf(tmp_str.data(), "%s %s (%s", buf, buf3, buf3); // We don't want to store the middle F
  this->celsius=atoi(buf3);
  this->fahrenheit=atoi(buf);
}

/*************************************************************
 *     Method: set_humidity                                  *
 *************************************************************
 *  Description:                                             *
 *     Extract humidity information.                         *
 *                                                           *
 * Input:                                                    *
 *   string hum - Temporary string containing humidity info. *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void localtemp::set_humidity(string hum)
{
  char buf[5];			// Be able to store a 100%\0
  // Format (xxx%)
  sscanf(hum.data(), "%s", buf);
  this->humidity=atoi(buf);
}

/*************************************************************
 *     Method: get_ob_info                                   *
 *************************************************************
 *  Description:                                             *
 *     Extracts METAR encoded information. We find it in     *
 *  the line starting with "ob: ". We parse this information *
 *  and fill in variables. Used for time and sky conditions. *
 *                                                           *
 * Input:                                                    *
 *   Nothing                                                 *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void localtemp::get_ob_info()	// ob: line stores METAR information. This info. is useful to know more exactly sky conditions.
{				// It stores also temperature, time and other info extracted by the decoded text

  size_t pos;
  struct tm *localtm;
  struct tm *remotetm;
  remotetm =new struct tm;
  string timeinfo;
try
  {
    timeinfo=this->ob.substr(6,this->ob.find(' ',6)-6); // DDHHMM[Z] Day Hour Minute Z (Zulu, Aviation Time Reference)
  } catch (exception &e)
  {
    cout<<"fallo en 1"<<ob<<"***"<<endl;
  }
    localtm=localtime(&get_time); // tm of the fetch time

  // We need month and year of the time given. So let's calculate.
try
  {

    remotetm->tm_mday=atoi(timeinfo.substr(0,2).data());
    remotetm->tm_hour=atoi(timeinfo.substr(2,2).data());
    remotetm->tm_min=atoi(timeinfo.substr(4,2).data());
  } catch (exception &e)
  {
    cout<<"fallo en 2"<<timeinfo<<"*"<<endl;
  }
  remotetm->tm_mon=localtm->tm_mon;
  remotetm->tm_year=localtm->tm_year;
  remotetm->tm_sec=0;
  
  if (remotetm->tm_mday==1)			// Month of the data
    {
      if (remotetm->tm_mday!=localtm->tm_mday)
	remotetm->tm_mon++;
      if (remotetm->tm_mon==12)   // It can't be 12!!
	{
	  remotetm->tm_mon=0;
	  remotetm->tm_year++;
	}       
    }
  info_time=mktime(remotetm);

try
  {
  this->ob=this->ob.substr(ob.find(' ',6)); // Extract ICAO station name and TIME INFO.
  // It will prevent some errors. We could find any of these strings in that name.

  //this->ob=this->ob.substr(this->ob.find(this->metar)+5);
  this->ob=this->ob.substr(0,this->ob.rfind('/')-2); // The info. we want is located betweet the last

  pos=this->ob.rfind('/');                           // 2 slashes. The las one indicates temperature and dew point
  if (pos!=string::npos)                             // The other one, may not exists but sometimes it does. 
    this->ob=this->ob.substr(this->ob.rfind('/'));

  } catch (exception &e)
  {
    cout <<"Fallo en 3"<<ob<<"***"<<endl;
  }
  //Sky
  if (this->ob.find("TCU")!=string::npos)
    mInfo.sky=tcu;
  else if ((this->ob.find("BKN")!=string::npos) ||
	   (this->ob.find("OVC")!=string::npos))
    mInfo.sky=brkovc;
  else if ((this->ob.find("FEW")!=string::npos) ||
	   (this->ob.find("SCT")!=string::npos))
    mInfo.sky=cloudy;
  else if ((this->ob.find("CAVOK")!=string::npos) ||
	   (this->ob.find("SKC")!=string::npos) ||
	   (this->ob.find("CLR")!=string::npos))
    mInfo.sky=clear;
  else 
    mInfo.sky=undef_sky;

  mInfo.CB=(this->ob.find("CB")!=string::npos);


  //Rain
  if ((this->ob.find("DZ")!=string::npos) ||
      (this->ob.find("RA")!=string::npos))
    mInfo.rain=rain;
  else if ((this->ob.find("SN")!=string::npos) ||
	   (this->ob.find("SG")!=string::npos))
    mInfo.rain=snow;
  else if ((this->ob.find("GS")!=string::npos) ||
	   (this->ob.find("GR")!=string::npos))
    mInfo.rain=hail;
  else 
    mInfo.rain=undef_rain;

  //Fog
  if ((this->ob.find("BR")!=string::npos) ||
      (this->ob.find("FG")!=string::npos))
    mInfo.fog=fog;
  else if ((this->ob.find("DU")!=string::npos) ||
	   (this->ob.find("DS")!=string::npos))
    mInfo.fog=dust;
  else if ((this->ob.find("FC")!=string::npos) ||
	   (this->ob.find("FU")!=string::npos) ||
	   (this->ob.find("HZ")!=string::npos) ||
	   (this->ob.find("SA")!=string::npos) ||
	   (this->ob.find("SS")!=string::npos) ||
	   (this->ob.find("VA")!=string::npos))
    mInfo.fog=other;
  else 
    mInfo.fog=undef_fog;

  // The most important for the display theme is rain, then fog and then sky conditions
  if (mInfo.rain!=undef_rain)
    {
      switch (mInfo.rain)
	{
	case rain: theme=RAINY_THEME; break;
	case snow: theme=SNOWY_THEME; break;
	case hail: theme=HAIL_THEME; break;
	default: break;
	}
    }
  else if (mInfo.fog!=undef_fog)
    {
      switch (mInfo.fog)
	{
	case fog  : theme=FOG_THEME; break;
	case dust : theme=DUST_THEME; break;
	case other: theme=PARTS_THEME; break;
	default: break;
	}
    }
  else if (mInfo.sky!=undef_sky)
    {
      switch (mInfo.sky)
	{
	case clear  : theme=CLEAR_THEME; break;
	case cloudy : theme=CLOUDY_THEME; break;
	case brkovc : theme=BROKEN_THEME; break;
	case tcu    : theme=TCU_THEME; break;
	default: break;
	}
    }
  else
    theme=DEFAULT_THEME;	// Nothing significant found.
}

/*************************************************************
 *     Method: getInfo                                       *
 *************************************************************
 *  Description:                                             *
 *     Fetch information and parse the file                  *
 *                                                           *
 * Output:                                                   *
 *    True if we downloaded it right, false if not.          *
 *                                                           *
 * Change History:                                           *
 *  Date      Author            Modification                 *
 * 20101031  Gaspar Fernández   Bug in skt                   *
 *************************************************************/ 
bool localtemp::getInfo()
{
  char *metar_fetch; 		               // Get Metar data URL
  MySock *skt;
  HTTP_Request *http;
  string::size_type pos, pos2;
  TKey_Value datarl;
  this->error=0;        // No error
  this->loaded=false;
  metar_fetch= (char*) malloc(80);
  string tmp;

  sprintf(metar_fetch, METAR_URL, this->metar.data());

  skt = new MySock(metar_fetch);
  verbsth(VERB_ASTTO, "Open connection: ");
 
  http=skt->GetHTTPData();

  delete skt;			// We don't need this anymore

  if (http!=NULL)
    {
      if (http->status==200)
	{
	  time(&get_time);	// We got the file at this moment
	  pos=0;

	  verbsth(VERB_ASTTO, "Get Data: ");

	  do
	    {
	      pos2=http->data.find("\n",pos+1);
	      if (pos==0)	// Extracts the first line (Name of the Station)
		{
		  this->long_location=http->data.substr(0, http->data.find("(",0)-1);
		  verbsth(VERB_ASTTO, "Station name: "+this->long_location);
		}
	      if (pos2!=string::npos)
		{
		  tmp = http->data.substr(pos,pos2-pos);
		  datarl.key=tmp.substr(0, tmp.find(":"));
		  datarl.value=tmp.substr(tmp.find(":")+1);
		  if (datarl.key=="Temperature")
		    this->set_temp(datarl.value);
		  else if (datarl.key=="Relative Humidity")
		    this->set_humidity(datarl.value);
		  else if (datarl.key=="Sky conditions")
		    this->sky=datarl.value;
		  else if (datarl.key=="ob")
		    this->ob=datarl.value;
		  // Search interesting data
		  pos=pos2+1;
		}
	    } while (pos2!=string::npos);
	  verbsth(VERB_ASTTO, "Get ob info: ");

	  get_ob_info();	// Get info from the METAR string
	  this->loaded=true;
	  verbsth(VERB_ASTTO, "Close connection: ");

	}
      else
// 	{
	  this->error=1;		// Not found
      // Print request
// 	  cout<<http->data<<endl;
// 	}
    }
  else
    return false;		// If there is a socket error...

  return true;			// Everything is OK (or almost)

}
