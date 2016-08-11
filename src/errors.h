#ifndef _ERRORS_CPP_
#define _ERRORS_CPP_
#include <string>
#define OKAY 0

#define ERR_NODISPLAY  1001	// Could not open display
#define ERR_XPMERROR   1003     // Error Creating Pixmap
#define ERR_BADFONT    1004	// Error loading desired font
#define ERR_BADSTTEMP  1005	// Default Sttemp not defined
#define ERR_CFGNOTFOUND 1006	// Configuration file not found
#define ERR_BADDFTCLR  1007     // Bad default Text color
#define ERR_BADDFICLR  1008     // Bad default tIme color
#define ERR_BADDFECLR  1009     // Bad default tEmp color
#define ERR_NOCFGFILE  1010	// Can't locate configuration file

#define VERB_NONE      0	// No verbose
#define VERB_CRITICAL  100	// Just critical complains
#define VERB_WARNING   1000	// Critical complains + Warnings
#define VERB_NOTICE    10000	// Critical complains + Warnings + Notices
#define VERB_ASTTO     100000   // Any single thing that occurs.  Verbose everything

#define DEFAULT_VERBOSE VERB_ASTTO // Default verbose, modify wich -v

using namespace std;
void error_handler (int err_code, char *add_string);
void verbsth (int level, const char *msg);
void verbsth (int level, string msg);
bool file_exists( const char *name );

#endif
