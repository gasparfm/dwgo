 /********************************************************************************
 *  File: strutils.cpp            								*
 *  Date: 20080508                    							*
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
 *     Some functions with strings.
 *   
 *   Change History:
 *    Date       Author     Modification
 *    
 ********************************************************************************/ 
const std::string whiteSpaces( " \f\n\r\t\v" );

/*************************************************************
 *  Functions: trimRight, trimLeft, trim                      *
 *************************************************************
 *  Description:                                             *
 *     Erases whitespaces (or any other characters) at the   *
 *  beginning, at the end, and at both of one string.        *
 *  Everything considered whitespace is in shiteSpaces       *
 *  constant (at the top of this file)                       *
 *                                                           *
 * Input:                                                    *
 *   string& str - The strim we'd like to trim               *
 *   string& trimChars - Characters we want to erase.        * 
 *                                                           * 
 * Output:                                                   *
 *   string& str (trimRight, trimLeft) - Our original string * 
 *                                       trimmed.            * 
 *   string (return value in trim()) - Trimmed string.       * 
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void trimRight(string& str,
      const string& trimChars = whiteSpaces )
{
  string::size_type pos = str.find_last_not_of( trimChars );
  str.erase( pos + 1 );

}

void trimLeft(string& str,
      const string& trimChars = whiteSpaces )
{
   string::size_type pos = str.find_first_not_of( trimChars );
   str.erase( 0, pos );

}

string trim( string str, const string& trimChars = whiteSpaces )
{
   trimRight( str, trimChars );
   trimLeft( str, trimChars );
   return str;
}

/*************************************************************
 *     Function: load_str_chr                                *
 *************************************************************
 *  Description:                                             *
 *     Reserves memory for a char* that will contain the     *
 *  data within a string or another *char                    *
 *                                                           *
 *                                                           *
 * Input:                                                    *
 *   char **ptr - Where we will store the string             * 
 *   string str (or char* str) - Where the string is         * 
 *                                                           *
 * Output:                                                   *
 *   The string will be in char **ptr                        * 
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void load_str_chr(char **ptr, const string str)
{
  *ptr=new char[str.size()+1];
  std::strcpy(*ptr,str.data());
}

void load_str_chr(char **ptr, const char* str)
{
  *ptr=new char[std::strlen(str)+1];
  std::strcpy(*ptr,str);
}

/*************************************************************
 *     Function: find_str_in_array                           *
 *************************************************************
 *  Description:                                             *
 *     Finds a str as an element in an array of char*        *
 *                                                           *
 * Input:                                                    *
 *   const char *str - The string we want to find            *
 *   const char *array - The array where the string may be   *
 *   int elements - The number of strings the array has      *
 *                                                           * 
 * Output:                                                   *
 *   int - The index of the array where the string is.       *
 *      If we couldn't find it, it returns -1                * 
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
int find_str_in_array(const char *str, const char *array[], int elements)
{
  int i=0, pos=-1;
  while ((i<elements) && (pos==-1))
    {
      if (std::strcmp(array[i], str)==0)
	pos=i;
      i++;
    }
  return pos;
}
