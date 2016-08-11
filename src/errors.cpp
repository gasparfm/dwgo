#include <iostream>
#include "errors.h"

using namespace std;

int verbose_level=DEFAULT_VERBOSE;

/*************************************************************
 *     Function: error_handler                                             *
 *************************************************************
 *  Description:                                             *
 *     Shows error message on the screen and abort execution *
 *  of the program.                                          *
 *                                                           *
 * Input:                                                    *
 *   int err_code - Which error?                             *
 *   char *add_string - Some errors may display additional   *
 *                      information.                         *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void error_handler (int err_code, char *add_string)
{
  if (err_code==OKAY)
    {
      return;			// Does nothing
    }
  else
    {
      switch (err_code)
	{
	case ERR_NODISPLAY:
	  cout<<"Couldn't open display.";
	  break;
	case ERR_XPMERROR:
	  cout<<"Couldn't load XPM image";
	  break;
	case ERR_BADFONT:
	  cout<<"Couldn't load desired font: "<<add_string;
	  break;
	case ERR_BADSTTEMP:
	  cout<<"Default temperature position not defined";
	  break;
	case ERR_CFGNOTFOUND:
	  cout<<"Configuration file ~/.dwgo.conf not found. See documentation";
	  break;
	case ERR_BADDFTCLR:
	  cout<<"Bad default text color. See default_textcolor in config. file";
	  break;
	case ERR_BADDFICLR:
	  cout<<"Bad default time color. See default_timecolor in config. file";
	  break;
	case ERR_BADDFECLR:
	  cout<<"Bad default temp color. See default_tempcolor in config. file";
	  break;
	case ERR_NOCFGFILE:
	  cout<<"Configuration file not found.";
	  break;

	default:
	  cout<<"Unknown error!!! :S";
	}
      cout <<endl;
      exit (err_code);
    }
}

/*************************************************************
 *     Function: verbsth                                     *
 *************************************************************
 *  Description:                                             *
 *     The application wants to tell the user something via  *
 *  stdout, acording to the verbose level chosen, it will    *
 *  display some messages, but some don't. Are we debugging? *
 *                                                           *
 * Input:                                                    *
 *    int level - Verbose level                              *
 *    char *msg or string msg - Is the message to be         *
 *                              displayed.                   *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/

void verbsth (int level, const char *msg)
{
  if (level<=verbose_level)
    cout << msg <<endl;
}

void verbsth (int level, string msg)
{
  if (level<=verbose_level)
    cout << msg <<endl;
}

/*************************************************************
 *     Function: file_exists                                 *
 *************************************************************
 *  Description:                                             *
 *     Does the specified file exist?                        *
 *                                                           *
 * Input:                                                    *
 *    const char *name - File name                           *
 *                                                           *
 * Output:                                                   *
 *    True if the file exists, it not, False                                                       *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
bool file_exists( const char *name )
{
    FILE *fp = NULL;

    fp = fopen( name, "rb" );
    if( fp != NULL )
    {
        fclose( fp );
        return true;
    }

    return false;
}
