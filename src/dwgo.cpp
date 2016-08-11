 /********************************************************************************
 *  File: wdgo.cpp    								*
 *  Version: 0.3        							*
 *  Date: 20080508            							*
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
 *     It is a Window Maker dockapp wich fetches weather information of the 
 *  selected cities from National Weather Service (http://weather.noaa.gov/) 
 *  to display current temperature and chage background image depending on 
 *  the current weather conditions.
 *
 *   Change History:
 *    Date (D.M.Y)    Author              Modification
 *     08.05.2008      Gaspar Fernández    Initial release
 *    
 ********************************************************************************/ 

#include <iostream>
#include <fstream>
#include <vector>
#include <string.h>
#include <strings.h>
#include <pthread.h>
#include <X11/Xlib.h>
#include <X11/xpm.h>
#include <X11/Xutil.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <math.h>
#include <time.h>
#include <X11/XKBlib.h>

#include "XDraw.h"
#include "errors.h"
#include "localtemp.h"
#include "dwgo.h"
#include "strutils.cpp"
#include "config.h"

using namespace std;
 
typedef struct
{
  vector <localtemp *> weathers;
  bool loaded;
  int refresh;			// When timer reaches refresh it reloads the information
  int counter;			// Timer
} Wth_vector;

typedef struct
{
  char station[5];		// Metar ID [ 4 characters + \0
  char name[16];		// Station name (it's for you). It's short to fit in.
} T_Station;

typedef struct
{
  int x, y, z;
} T_Point;

typedef struct
{
  char *img;			// Xpm asociated
  char *text_font;		// Location text font
  char *temp_font;		// Temperature text font
  char *time_font;		// Temperature text font
  XDraw::XDrawColor text_color;	// Text color
  XDraw::XDrawColor temp_color;	// Temperature Color
  XDraw::XDrawColor time_color;	// Temperature Color
  T_Point stname;		// Where to put the station name
  T_Point sttemp;		// Where to put the temperature
  T_Point sttime;		// Where to put the temperature
} T_Theme;

typedef struct
{
  int x1, x2, y1, y2;
  XDraw::XDrawColor in_color, out_color, bar_color;
} T_WaitBox;

typedef struct
{
  vector <T_Station> stations;	// List of preferred stations

  T_Theme metar_themes[TOTAL_THEMES];	// Load themes preferences
  T_WaitBox wbox;			// Wait box

  int update_int;		// Update interval
  char deg_unit;		// Celsius or Fahrenheit
} DwgoConf;

/*************************************************************
 *     Function: fetchWeatherInfo                            *
 *************************************************************
 *  Description:                                             *
 *      Fetch weather information from the server. It        *
 *  downloads a file for every location in our vector.       *
 *                                                           *
 * Input:                                                    *
 *   Wth_vector *weathers - Our weather vector               *
 *                                                           * 
 * Output:                                                   *
 *   Nothing                                                 * 
 *                                                           *
 * Change History:                                           *
 *  Date      Author             Modification                *
 * 20101106 Gaspar Fernández     Added error status          *
 *************************************************************/  
void fetchWeatherInfo(Wth_vector *weathers)
{
  Wth_vector *wths= (Wth_vector *)weathers;
  //  bool allok=true;
  for (unsigned int k=0; k<wths->weathers.size(); k++) {
    localtemp *current = wths->weathers.at(k);
    wths->loaded=false; // This will make animated bar disappear when loaded
    
    if (!current->getInfo())
      {
	verbsth(VERB_WARNING, "Don't have access to the server... I'll try later");
	break;
      }
    else if (current->error)
      {
	verbsth(VERB_WARNING, "Got an error while retrieving information.");
	break;
      }
  }
  //  wths->loaded=allok;	// All loaded
}

/*************************************************************
 *     Function: getWeatherInfo                              *
 *************************************************************
 *  Description:                                             *
 *    Calls fetchWeatherInfo periodically                    *
 *                                                           *
 * Input:                                                    *
 *   voir *weathers - Our weather vector. It's a void type   *
 *   because it is called using pthread_create.              *
 *                                                           *
 * Output:                                                   *
 *   NULL                                                           *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void *getWeatherInfo(void *weathers)
{
  while (1)			// Fetch periodically temperatures
    {
      if (((Wth_vector *)weathers)->counter>=((Wth_vector *)weathers)->refresh)
	{
	  ((Wth_vector *)weathers)->counter=0;
	  fetchWeatherInfo((Wth_vector *)weathers);
	}
      sleep(1);
      //      fetchWeatherInfo((Wth_vector *)weathers);
      ((Wth_vector *)weathers)->counter++;
    }
  return NULL;
}

/*************************************************************
 *     Function: load_coords                                 *
 *************************************************************
 *  Description:                                             *
 *    Translates a string with a,b,c with a,b,c three ints   *
 *  to a T_Point structure. But the string may contain two,  *
 *  or one number. It is used to locate text, so X           * 
 *  coordinate, can wi CENTER_TEXT, and Z can be             * 
 *  DEFAULT_MAX_WIDTH                                        * 
 *                                                           *
 * Input:                                                    *
 *    a string with the coordinates separated with commas.   *
 *                                                           * 
 * Output:                                                   *
 *    T_Point structure containing x, y and z                * 
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
T_Point load_coords(const string &str)
{
  T_Point coord;
  size_t coma = str.find(',',0);	// Finds comma
  size_t coma2 = str.find(',', coma+1); // Finds another comma
  coord.z = DEFAULT_MAX_WIDTH;

  if (coma!=string::npos)
    {				// Load coordinates
      coord.x = atoi(trim(str.substr(0,coma)).data());
      if (coma2!=string::npos)
	{
	  coord.z = atoi(trim(str.substr(coma+1, coma2-coma+1)).data());
	  coord.y = atoi(trim(str.substr(coma2+1)).data());
	}
      else
	coord.y = atoi(trim(str.substr(coma+1)).data());
    }
  else
    {				// Centers text in X
      coord.x = CENTER_TEXT;
      coord.y = atoi(str.data());
    }

  return coord;
}

/*************************************************************
 *     Function: load_colors                                 *
 *************************************************************
 *  Description:                                             *
 *     Read the string containing red, green and blue values *
 *  for the desired colour separated with commas. And stores *
 *  this values in a XDrawColor structure                    *
 *                                                           *
 * Input:                                                    *
 *  string with r,g,b values                                 * 
 *                                                           *                                                *
 * Output:                                                   *
 *  XDrawColor structure with that information               * 
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
XDraw::XDrawColor load_colors(string str)
{
  XDraw::XDrawColor color;
  sscanf(str.data(),"%d %d %d", &color.r, &color.g, &color.b);
  return color;
}

/*************************************************************
 *     Function: load_wbcoords                               *
 *************************************************************
 *  Description:                                             *
 *     Made only to read from config. file the coords of the *
 *  upper left and lower right corners of the wait box       *
 *  (this one that appears when the data is being downloaded)*
 *                                                           *
 * Input:                                                    *
 *  string with the coords. separated with spaces            *
 *  T_WaitBox structure where to store the data              * 
 *                                                           *
 * Output:                                                   *
 *  inside T_WaitBox structure, the coordinates of the       *
 *  box are loaded from the string                                                          * 
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void load_wbcoords(string str, T_WaitBox *wbox)
{
  sscanf(str.data(),"%d %d %d %d", &wbox->x1, &wbox->y1, &wbox->x2, &wbox->y2);
}

bool existsfilepath(const char* path, const char* file, char pathfile[])
{
  bzero(pathfile, 256);		// Erase last data
  strcpy(pathfile, path);
  strcat((char*)pathfile, file);
  return (file_exists(pathfile));
}

/*************************************************************
 *     Function: load_config                                 *
 *************************************************************
 *  Description:                                             *
 *     Loads the config file into the DwgoConf structure.    *
 *     We also do some checks to return error codes.         *
 *                                                           *
 * Input:                                                    *
 *    char* file - File to read the configuration            *
 *    DwgoConf &config - Where to put the configuration      *
 *    Display *display - Used to make some checks            *
 *                                                           *
 * Output:                                                   *
 *    DwgoConf &config - We store here the configuration     *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void load_config(const char *file, DwgoConf &config, Display *disp, const char *homedir)
{
  ifstream inFile;
  
  string temp,a,b,c,tmp;
  char *other_dir;
  char *datadir;
  size_t pos, pos2;
  T_Station station;
  int aux;
  char pathfile[256];		// 256 temp chars for path+file
  // This array stores all themes, as we may read it in the file.
  const char *theme_keywords[TOTAL_THEMES]={"default", "clear", "cloudy", "broken", "tcu", "rainy", "snowy", "hail", "foggy", "dust", "otparts"};
  cout<<homedir<<"*\n";
  other_dir=(char*)malloc(strlen(homedir)+6);  strcpy(other_dir, homedir);  strcat(other_dir, (char*)"/.dwgo");
  datadir  =(char*)malloc(strlen(DATADIR)+5);  strcpy(datadir, DATADIR);    strcat(datadir, (char*)"/dwgo");
  config.stations.clear();	// Clear station vector
  config.update_int=0;
  config.deg_unit=DEFAULT_DEG_UNIT;
  inFile.open(file) ;

//   verbsth(VERB_NOTICE, "K"+strcat((char*)homedir,(char*)file););
  if (inFile.is_open())
    {
      while (!inFile.eof()){
	getline(inFile,temp);
	pos = temp.find_first_of((char*)" =#"); // We can find \n (skipped when reading) , #, =, (space) symbols
	if (pos!=string::npos)
	  {
	    a=trim(temp.substr(0,pos)); // Store here what's before the " ","=" of "#"
	    b=trim(temp.substr(pos+1)); // Store here what's after that
	    switch (temp[pos])
	      {
	      case '=':		                // If we've found =...
		pos2=a.find("->");		// An arrow indicates theme or waitbox properties
		if (pos2!=string::npos)
		  {
		    c=a.substr(pos2+2);         // Store here what's after the ->
		    a=a.substr(0,pos2);         // Store here what's between the = and the ->
		    aux=find_str_in_array(a.data(),theme_keywords, TOTAL_THEMES); // if we've found a, in the array of themes...
		    if (aux>-1)	// Right theme, let's see the options
		      {
			if (c=="img")          // img property, specify the XPM to load when this theme is active
			  {
			    tmp="/"+b;
			    if (file_exists(b.data()))
			      load_str_chr(&config.metar_themes[aux].img, b);
			    else if (existsfilepath(other_dir, tmp.data(), pathfile))
			      load_str_chr(&config.metar_themes[aux].img, pathfile);
			    else if (existsfilepath(datadir, tmp.data(), pathfile))
			      load_str_chr(&config.metar_themes[aux].img, pathfile);
			    else
			      verbsth(VERB_WARNING,"Can't find default img file: "+b+" for "+c+"_img attribute");
			  }
			else if (c=="txtfont") // text found - Generally station name
			  {
			    if (testfont(b.data(), disp))
			      load_str_chr(&config.metar_themes[aux].text_font, b);
			    else
			      {
				if (aux>0)
				  {
				    load_str_chr(&config.metar_themes[aux].text_font, config.metar_themes[0].text_font); // Load default font
				    verbsth(VERB_WARNING, ("Can't load font: ")+b+(" loading default instead."));
				  }
				else
				  error_handler(ERR_BADFONT, (char*) b.data()); // Can't continue
			      }
			  } 
			else if (c=="tempfont") // temperature font
			  {
			    if (testfont(b.data(), disp))
			      load_str_chr(&config.metar_themes[aux].temp_font, b);
			    else
			      {
				if (aux>0)
				  {
				    load_str_chr(&config.metar_themes[aux].temp_font, config.metar_themes[0].temp_font); // Load default font
				    verbsth(VERB_WARNING, ("Can't load font: ")+b+(" loading default instead."));
				  }
				else
				  error_handler(ERR_BADFONT, (char*) b.data()); // Can't continue
			      }
			  }
			else if (c=="timefont") // time fond - Specify when was the data taken
			  {
			    if (testfont(b.data(), disp))
			      load_str_chr(&config.metar_themes[aux].time_font, b);
			    else
			      {
				if (aux>0)
				  {
				    load_str_chr(&config.metar_themes[aux].time_font, config.metar_themes[0].time_font); // Load default font
				    verbsth(VERB_WARNING, ("Can't load font: ")+b+(" loading default instead."));
				  }
				else
				  error_handler(ERR_BADFONT, (char*) b.data()); // Can't continue
			      }

			  }
			else if (c=="txtloc") // Where to put the station name text
			  config.metar_themes[aux].stname=load_coords(b);
			else if (c=="timloc") // Where to put the time text
			  config.metar_themes[aux].sttime=load_coords(b);
			else if (c=="tmploc") // Where to put the temperature text
			  config.metar_themes[aux].sttemp=load_coords(b);
			else if (c=="textcolor") // Station name text color
			  config.metar_themes[aux].text_color=load_colors(b);
			else if (c=="timecolor") // Time text color
			  config.metar_themes[aux].time_color=load_colors(b);
			else if (c=="tempcolor") // Temperature textcolor
			  config.metar_themes[aux].temp_color=load_colors(b);
		      }
		    else if (a=="waitbox") // If it is not a theme, it may be the waitbox
		      {
			if (c=="coords") // Coordinates
			  load_wbcoords(b, &config.wbox);
			else if (c=="out_color") // Border color
			  config.wbox.out_color=load_colors(b);
			else if (c=="in_color") // Fill in color
			  config.wbox.in_color=load_colors(b);
			else if (c=="bar_color") // Moving bar color
			  config.wbox.bar_color=load_colors(b);
		      }
		    else
		      verbsth(VERB_WARNING, "Sorry, I can't recognise "+a+"_"+c+" attribute.");
		  }
		else if (a=="update_interval") // Weather will update each (update_interval) seconds
		  config.update_int=atoi(b.data());
		else if (a=="deg_unit") // (C)elsius or (F)ahrenheit
		  config.deg_unit=b[0]; // Only first char

		break;
	      case ' ':		// If the first thing we see is a " ", insead of a "="
		if (a== "AddStation") // Adds station to monitor
		  {
		    pos=b.find(":",0); // The stations must be specified with the station name, separated with ":"
		    a=b.substr(0,pos);
		    b=b.substr(pos+1);
		    strcpy(station.station,a.data());
		    strcpy(station.name,b.substr(0,14).data()); // Just 15 characters
		    config.stations.push_back(station);
		    verbsth(VERB_NOTICE, "Add Station \""+a+"\" called \""+b+"\"");
		  }
		break;
	    
	      }
	  }
      }
      inFile.close();

      if (config.update_int==0)	// Prevent update_interval from being zero
	config.update_int=DEFAULT_UPDATE_INTERVAL; // If it is not defined, or 0, replace it with the default value
      if (config.metar_themes[DEFAULT_THEME].sttemp.y==-1) // Temperature must be shown
	error_handler(ERR_BADSTTEMP,NULL);      
      // Colors must have positive values
      if (config.metar_themes[DEFAULT_THEME].text_color.r==-1)
	error_handler(ERR_BADDFTCLR,NULL);      
      if (config.metar_themes[DEFAULT_THEME].time_color.r==-1)
	error_handler(ERR_BADDFICLR,NULL);      
      if (config.metar_themes[DEFAULT_THEME].temp_color.r==-1)
	error_handler(ERR_BADDFECLR,NULL);      

    }
    else
      error_handler(ERR_CFGNOTFOUND,NULL);
}

/*************************************************************
 *     Function: time_diff                                   *
 *************************************************************
 *  Description:                                             *
 *     Calculates local timezone time difference with GMT.   *
 *                                                           *
 * Input:                                                    *
 *     Nothing                                               *
 *                                                           * 
 * Output:                                                   *
 *     int - Time difference in seconds (local_time-gmt_time)*
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
int time_diff ()
{
  struct tm *local, *gmt;
  int local_s, gmt_s;
  time_t tmp;

  tmp = time(0);
  gmt = gmtime(&tmp);
  if (gmt->tm_yday==0)
    gmt->tm_yday=366;

  gmt_s = gmt->tm_yday*86400+gmt->tm_hour*3600 + gmt->tm_min* 60 + gmt->tm_sec;

  tmp = time(0);
  local = localtime(&tmp);
  if (local->tm_yday==0)
    local->tm_yday=366;
  local_s = local->tm_yday*86400 + local->tm_hour*3600 + local->tm_min*60 + local->tm_sec;

  return (local_s-gmt_s);
}

/*************************************************************
 *     Function: displaytemp                                 *
 *************************************************************
 *  Description:                                             *
 *    Draws an image inside the dockapp and renders text     *
 *  with location name, temperature and time when the data   *
 *  was taken.                                               *
 *                                                           *
 * Input:                                                    *
 *     XDraw *image - Image which will be drawn in the       *
 *                    dockapp.                               *
 *     DwgoConf cfg - Configuration                          *
 *     localtemp local_stt - Local Station information       *
 *     int tm_diff  - Time difference (see time_diff() func.)*
 *     char *xpmthemes - XPM images to display               *
 *                                                           *
 * Output:                                                   *
 *     Nothing                                                      *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void displaytemp(XDraw *image, const DwgoConf cfg, localtemp *local_stt, int tm_diff, char *xpm_themes[])
{
   char tmp_disp[10];		// Aux to display temperature
   char *txt_font;
   char *tmp_font;
   char *tim_font;
   int theme=local_stt->theme;
   XDraw::XDrawColor txcolor, ticolor, tecolor;
   T_Point sttemp, sttext, sttime;
   time_t time_taking;
   struct tm *moment;

   // We load each xpm_themes image when it is needed.
   if (xpm_themes[theme]==NULL)
     {
       if (file_exists(cfg.metar_themes[theme].img))
	 {
	   XpmReadFileToBuffer(cfg.metar_themes[theme].img, &xpm_themes[theme]);
	   verbsth(VERB_NOTICE, (string)"Loading img. theme: "+cfg.metar_themes[theme].img);
	 }
       else
	 {			// If xpm file doesn't exists we load the default theme
	   local_stt->theme=DEFAULT_THEME;
	   theme=DEFAULT_THEME;
	 }

     }

   // We do some checks and we change theme details to default details when we can't use them.
   // We copy this data to our temp string. If we are quick, themes can change and the app crash
   if (cfg.metar_themes[theme].text_font!=NULL)
     load_str_chr(&txt_font,cfg.metar_themes[theme].text_font);
   else 
     load_str_chr(&txt_font,cfg.metar_themes[0].text_font);

   if (cfg.metar_themes[theme].temp_font!=NULL)
     load_str_chr(&tmp_font,cfg.metar_themes[theme].temp_font);
   else 
     load_str_chr(&tmp_font,cfg.metar_themes[0].temp_font);

   if (cfg.metar_themes[theme].time_font!=NULL)
     load_str_chr(&tim_font,cfg.metar_themes[theme].time_font);
   else 
     load_str_chr(&tim_font,cfg.metar_themes[0].time_font);

   // (C)elsius of (F)ahrenheit
   if (cfg.deg_unit=='F')
     sprintf(tmp_disp, "%d "DEG_SYMBOL"F", local_stt->fahrenheit);
   else
     sprintf(tmp_disp, "%d "DEG_SYMBOL"C", local_stt->celsius);

   image->replace_background(xpm_themes[theme]); // Load the background image
   image->setWindowPixmapShaped();

   // If y position of the text is -1 in the current theme, it will be replaced with default theme position.
   // but the temperature. It will be if y<=-1 (if text or time y position is >-1 it won't be displayed)
   sttemp=(cfg.metar_themes[theme].sttemp.y>-1)?cfg.metar_themes[theme].sttemp:cfg.metar_themes[0].sttemp;
   sttext=(cfg.metar_themes[theme].stname.y==-1)?cfg.metar_themes[DEFAULT_THEME].stname:cfg.metar_themes[theme].stname;
   sttime=(cfg.metar_themes[theme].sttime.y==-1)?cfg.metar_themes[DEFAULT_THEME].sttime:cfg.metar_themes[theme].sttime;

   // Test current theme color
   // If red value of the color is -1, the color will be replaced with the default.
   txcolor=(cfg.metar_themes[theme].text_color.r==-1)?cfg.metar_themes[DEFAULT_THEME].text_color:cfg.metar_themes[theme].text_color;
   ticolor=(cfg.metar_themes[theme].time_color.r==-1)?cfg.metar_themes[DEFAULT_THEME].time_color:cfg.metar_themes[theme].time_color;
   tecolor=(cfg.metar_themes[theme].temp_color.r==-1)?cfg.metar_themes[DEFAULT_THEME].temp_color:cfg.metar_themes[theme].temp_color;

   // Draw strings
   image->drawString(sttemp.x, sttemp.y, sttemp.z, tecolor, (char*)tmp_font, tmp_disp);
   if (sttext.y>-1)
     image->drawString(sttext.x, sttext.y, sttext.z, txcolor, (char*)txt_font, (char*)local_stt->location_name.data());
   if ((sttime.y>-1) && (local_stt->loaded))
     {
       time_taking=local_stt->info_time+tm_diff; // Translates to local time
       moment=localtime(&time_taking);
       sprintf(tmp_disp, "%.2d:%.2d", moment->tm_hour,moment->tm_min); // Makes it 00:00
       image->drawString(sttime.x, sttime.y, sttime.z, ticolor, (char*)tim_font, (char*)tmp_disp);
     }

     image->Sync();

}

/*************************************************************
 *     Function: config_defaults                             *
 *************************************************************
 *  Description:                                             *
 *     Loads the default configuration in DwgoConf structure.*
 *                                                           *
 *                                                           *
 * Input:                                                    *
 *     DwgoConf *Dwgo_Configuration                          *
 *                                                           *
 * Output:                                                   *
 *     DwgoConf *Dwgo_Configuration - It will have defaults  *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void config_defaults(DwgoConf *Dwgo_Configuration)
{
   const T_Point def_sttemp = DEFAULT_STTEMP;
   const XDraw::XDrawColor clnull = DEFAULT_STTEMP;

   Dwgo_Configuration->wbox.x1=2;
   Dwgo_Configuration->wbox.x2=62;
   Dwgo_Configuration->wbox.y1=2;
   Dwgo_Configuration->wbox.y1=12;
   Dwgo_Configuration->wbox.in_color=setDrawColor(100,100,20);   
   Dwgo_Configuration->wbox.out_color=setDrawColor(10,10,2);   
   Dwgo_Configuration->wbox.bar_color=setDrawColor(50,50,12);   

   for (int j=0;j<TOTAL_THEMES;j++)
     {
       Dwgo_Configuration->metar_themes[j].text_font=NULL;
       Dwgo_Configuration->metar_themes[j].temp_font=NULL;
       Dwgo_Configuration->metar_themes[j].time_font=NULL;
       Dwgo_Configuration->metar_themes[j].sttemp=def_sttemp;
       Dwgo_Configuration->metar_themes[j].stname=def_sttemp;
       Dwgo_Configuration->metar_themes[j].sttime=def_sttemp;
       Dwgo_Configuration->metar_themes[j].text_color=clnull;
       Dwgo_Configuration->metar_themes[j].temp_color=clnull;
       Dwgo_Configuration->metar_themes[j].time_color=clnull;
     }

}

/*************************************************************
 *     Function: weathers_create_list                        *
 *************************************************************
 *  Description:                                             *
 *     Fills the Wth_vector weathers vector with the         *
 *  stations loaded from the confiuration file. And flag     *
 *  the structure to update.                                 *
 *                                                           *
 * Input:                                                    *
 *   Wth_vector *weathers -                                  *
 *   DwgoConf Dwgo_Configuration - Where to get the data from*
 *                                                           *
 * Output:                                                   *
 *   Wth_vector - We will fill in the weathers vector here.  *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void weathers_create_list(Wth_vector *weathers, DwgoConf Dwgo_Configuration)
{
   weathers->counter=Dwgo_Configuration.update_int; // Refresh now !!
   weathers->refresh=Dwgo_Configuration.update_int; // Refresh now !!


   weathers->loaded=false;
   weathers->weathers.clear();
   
   for (unsigned int k=0; k<Dwgo_Configuration.stations.size(); k++)
     weathers->weathers.push_back(new localtemp((char*)Dwgo_Configuration.stations.at(k).station,
						 (char*)Dwgo_Configuration.stations.at(k).name));
   
}

/*************************************************************
 *     Function: weathers_loader                             *
 *************************************************************
 *  Description:                                             *
 *     Calls weathers_create_list and creates the thread     *
 *  that will periodically reload weather information from   *
 *  the server                                               *
 *                                                           *
 * Input:                                                    *
 *  Wth_vector - Where the weather info will be stored       *
 *  DwgoConf Dwgo_Configuration - Configuration loaded from  *
 *                                file                       *
 *                                                           *
 * Output:                                                   *
 *  Nothing                                                  *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void weathers_loader(Wth_vector *weathers, DwgoConf Dwgo_Configuration)
{
  Wth_vector *th_parm;
  pthread_t *threads;
  pthread_attr_t pthread_custom_attr;

  weathers_create_list(weathers, Dwgo_Configuration);

   threads=(pthread_t *)malloc(1*sizeof(*threads));
   pthread_attr_init(&pthread_custom_attr);
   
   th_parm=weathers;

   pthread_create(threads, &pthread_custom_attr, getWeatherInfo, (void *)(th_parm));
}

/*************************************************************
 *     Function: getHomeDir                                  *
 *************************************************************
 *  Description:                                             *
 *     Try to guess home directory. First searching HOME     *
 *  environment variable, then try getpwuid.                 *
 *                                                           *
 * Input:                                                    *
 *     Nothing                                               *
 *                                                           *
 * Output:                                                   *
 *     char* path to user home                               *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/  
char *getHomeDir()
{
  static char *home = NULL;
  
  if (!home)
    {
      home = getenv("HOME") ;
    }
  if (!home)
    {
      struct passwd *pw = getpwuid(getuid());
      if (pw) home = pw->pw_dir ;
    }
  return home;
}

bool copyfile(const char* filefrom, const char* fileto)
{

    FILE *ffrom = NULL;
    FILE *fto = NULL;
    char buf[512];

    ffrom = fopen( filefrom, "rb" );
    if( ffrom != NULL )
    {
      fto = fopen( fileto, "wb" );
      if( fto != NULL )
	{
	  while (fgets(buf, 256, ffrom)) {
	    if (fputs(buf, fto) == EOF) { // error writing data
	      return false;
	    }
	  }
	}
      else
	return false;
      
      fclose( ffrom );
    }
    else
      return false;
    return true;
    
}

char *locateConfig(const char* homeuser)
{
  static char cfgpath[256];
  char tmp[256];
  if (existsfilepath(homeuser, "/.dwgo.conf", cfgpath))
    return cfgpath;
  else if (existsfilepath(homeuser, "/.dwgo/dwgo.conf", cfgpath))
    return cfgpath;
  else if (existsfilepath(DATADIR, "/dwgo/dwgo.conf", cfgpath))
    {
      existsfilepath(homeuser, "/.dwgo.conf", tmp);
      if (copyfile(cfgpath, tmp)) // Copies /usr/share/dwgo/dwgo.conf -> ~/.dwgo.conf
	strcpy(cfgpath, tmp);		  // If config file is copied -> Return user configuration

	return cfgpath;
    }
  else if (file_exists("dwgo.conf"))
    return (char*)"dwgo.conf";
  else
    return NULL;
}

// Main application
int main(int argc, char** argv) {
 
  Wth_vector weathers;

   Display*  disp;
   Window    mRoot;
   Window    mAppWin;
   Window    mIconWin;
   char*     xpm_themes[TOTAL_THEMES]; // Where to load XPMs
   bool      bar=false;		       // Toggle the bar beacuse you like it
   XClassHint classHint;
   XSizeHints sizeHints;
   XWMHints   wmHints;
   Atom       deleteWindow;
   XDraw*       image;
   XEvent     report;
   KeySym     ksym;
   DwgoConf   Dwgo_Configuration; // Main configuration
   int eventmask;
   
   char *user_homedir;		// User home directory

   char *home_dir;		// Copy of user_homedir because we overwrite user_homedir Q&D

   char *config_path;		// Config file with path

   int anim=0;			// Waitbar animation status
   bool direc=true;		// Direction of the waitbar animation

   int punter=0;		// Location being displayed actually

   int tm_diff=0;		// Time differente
   timespec tmp={0,30000000L};	// Sleep time between X updates

   user_homedir= getHomeDir();
   home_dir=(char*)malloc(strlen(user_homedir));
   strcpy(home_dir, user_homedir);

   config_path=locateConfig(user_homedir);
   if (config_path==NULL)
     error_handler(ERR_NOCFGFILE, NULL);

   if ((disp = XOpenDisplay(NULL)) == NULL)
     error_handler(ERR_NODISPLAY, NULL);
   
   config_defaults(&Dwgo_Configuration);
   for (int j=0;j<TOTAL_THEMES;j++)
     xpm_themes[j]=NULL;	// Point all xpms to NULL

   load_config(config_path, Dwgo_Configuration, disp, home_dir);

   tm_diff=time_diff();
   weathers_loader(&weathers, Dwgo_Configuration);

 
   // Get root window
   mRoot = RootWindow(disp, DefaultScreen(disp));
   // Create windows
   mAppWin = XCreateSimpleWindow(disp, mRoot, 1, 1, 64, 64, 0, 0, 0);
   mIconWin = XCreateSimpleWindow(disp, mAppWin, 0, 0, 64, 64, 0, 0, 0);

   // Set classhint
   classHint.res_name =  (char*) "DWGO";
   classHint.res_class = (char*) "DWGO";
   XSetClassHint(disp, mAppWin, &classHint);


   // Create delete atom
   deleteWindow = XInternAtom(disp, "WM_DELETE_WINDOW", False);
   XSetWMProtocols(disp, mAppWin, &deleteWindow, 1);
   XSetWMProtocols(disp, mIconWin, &deleteWindow, 1);

   // Set windowname
   XStoreName(disp, mAppWin, "DWGO 0.3");
   XSetIconName(disp, mAppWin, "DWGO 0.3");

   // Set sizehints
   sizeHints.flags= USPosition;
   sizeHints.x = 0;
   sizeHints.y = 0;
   XSetWMNormalHints(disp, mAppWin, &sizeHints);
   // Set wmhints.. Make it a dockapp
   wmHints.initial_state = WithdrawnState;
   wmHints.icon_window = mIconWin;	  
   wmHints.icon_x = 0;
   wmHints.icon_y = 0;
   wmHints.window_group = mAppWin;
   wmHints.flags = StateHint | IconWindowHint | IconPositionHint | WindowGroupHint;
   XSetWMHints(disp, mAppWin, &wmHints);

   // Set command
   XSetCommand(disp, mAppWin, argv, argc); // X Params.
//    // Set background image

   XpmReadFileToBuffer(Dwgo_Configuration.metar_themes[DEFAULT_THEME].img, &xpm_themes[DEFAULT_THEME]);
   image = new XDraw(disp, mRoot, xpm_themes[DEFAULT_THEME]);
   image->setDefaultWindow(mIconWin);
    image->setWindowPixmapShaped(mIconWin);

   XMapWindow(disp, mIconWin);
   XMapWindow(disp, mAppWin);
   XSync(disp, False);

   eventmask= ButtonPressMask | //ButtonReleaseMask | 
              FocusChangeMask | LeaveWindowMask | KeyPressMask |
              EnterWindowMask;
   XSelectInput(disp, mIconWin, eventmask );  

   while (1)			// Main loop!!
     {
       if (XCheckMaskEvent(disp, eventmask, &report))
	 {
	   switch (report.type)
	     {
	     case KeyPress:
	       ksym = XLookupKeysym(&(report.xkey), report.xkey.state);

	       switch (ksym)
		 {
// 		 case XK_Escape:
// 		   exit(0);
// 		   break;
		 case XK_F5:
		   if (weathers.counter>3)		// Min. time between refreshes
		     weathers.counter=weathers.refresh; // Refresh weathers
		   else
		     verbsth(VERB_WARNING,"You just have updated temperatures!!! Wait a little bit!");
		   break;
		 case XK_F6:	// Reload conf. file, Useful form theme creation and conf. checks
		   punter=0;
		   weathers.counter=0; // Make sure it won't update during reload
		   config_defaults(&Dwgo_Configuration);
		   for (int j=0;j<TOTAL_THEMES;j++)
		     xpm_themes[j]=NULL;	// Point all xpms to NULL
		   
		   load_config(".dwgo", Dwgo_Configuration, disp, home_dir);
		   weathers_create_list(&weathers, Dwgo_Configuration);
		   break;
		 case XK_Down:
		 case XK_Left:
		   punter--;
		   if (punter==-1)
		     punter=weathers.weathers.size()-1;
		   displaytemp(image, Dwgo_Configuration, weathers.weathers.at(punter), tm_diff, xpm_themes);
		   break;
		 case XK_Up:
		 case XK_Right:
		   punter++;
		   if ((unsigned)punter==weathers.weathers.size())
		     punter=0;
		   displaytemp(image, Dwgo_Configuration, weathers.weathers.at(punter), tm_diff, xpm_themes);
		   break;
		 case XK_b:
		   bar=!bar;
		   displaytemp(image, Dwgo_Configuration, weathers.weathers.at(punter), tm_diff, xpm_themes);

		 default: break;

		 }
	       break;
	     case ButtonPress:
	       switch (report.xbutton.button) {	       
	       case Button1:
		 punter++;
		 if ((unsigned)punter==weathers.weathers.size())
		   punter=0;
		 displaytemp(image, Dwgo_Configuration, weathers.weathers.at(punter), tm_diff, xpm_themes);
		 break;
	       case Button3:
		 verbsth(VERB_ASTTO,"Right click");
		 break;
	       }
	       break;
	     case EnterNotify: 
	     case LeaveNotify: 
	       XSetInputFocus(disp, mIconWin, RevertToParent, CurrentTime);
	       verbsth(VERB_ASTTO, " Focus Change");
	       break; 
	     default:
	       verbsth(VERB_ASTTO, "Another Event");
	     }
	 }
       else
	 {

	   if ((!weathers.weathers.at(punter)->loaded) || (bar))
	     {
	       image->DrawRect(Dwgo_Configuration.wbox.x1,Dwgo_Configuration.wbox.y1,Dwgo_Configuration.wbox.x2,Dwgo_Configuration.wbox.y2, Dwgo_Configuration.wbox.out_color);
	       image->DrawRect(Dwgo_Configuration.wbox.x1+1,Dwgo_Configuration.wbox.y1+1,Dwgo_Configuration.wbox.x2-2,Dwgo_Configuration.wbox.y2-2, Dwgo_Configuration.wbox.in_color);
	       image->DrawRect(Dwgo_Configuration.wbox.x1+1+anim,Dwgo_Configuration.wbox.y1+1,6,Dwgo_Configuration.wbox.y2-2, Dwgo_Configuration.wbox.bar_color);
	       if (direc)
		 anim++;
	       else
		 anim--;
	       if ((anim==Dwgo_Configuration.wbox.x2-8) || (anim==0)) // Dwgo_Configuration.wbox.x2 -2 -6 (bar width)
		 direc=!direc;
	       image->Sync();
	     }
	   else if (!weathers.loaded)
	     {
	       weathers.loaded=true;
	       displaytemp(image, Dwgo_Configuration, weathers.weathers.at(punter), tm_diff,  xpm_themes);
	     }
	   
	   
	 }
       nanosleep(&tmp, NULL);
     }
   delete image;		  
}
