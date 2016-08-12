 /*******************************************************************************
 *  File: MySock.cpp  								*
 *  Version: 0.2        							*
 *  Date: 20081002            							*
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
 *     Class and functions to create socket connections and fetch information
 *   from http servers. It is part of another project I'm in. (Some comments are
 *   in Spanish.
 * 
 *   Change History:
 *    Date       Author           Modification
 *   10.02.2008 Gaspar Fernández   Initial release
 *   31.10.2010 Gaspar Fernández   Bug Corrections
 ********************************************************************************/ 
#include "MySock.h"
#include <unistd.h>
/*************************************************************
 *     Function: extract_key_value                           *
 *************************************************************
 *  Description:                                             *
 *   Extract information separated with a white space.       *
 *   We consider KEY before the whitespace and VALUE         *
 *   after that.                                             *
 *                                                           *
 * Input:                                                    *
 *   string str - String containing the relation             *
 *                                                           *
 * Output:                                                   *
 *   TKey_Value struct containing separated key and value.   *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
TKey_Value extract_key_value(string str)
{
  TKey_Value data;
  char buf[512];		 // Max characters
  try				 // Sólo si es posible
    {
      sscanf(str.data(), "%s", buf); // Extract header
      data.value=str.substr(strlen(buf));
      data.key=buf;
    }
  catch(exception& e)
    {
      // Do nothing
    }
  return data;
}

/*************************************************************
 *     Constructor MySock()                                  *
 *************************************************************
 *  Description:                                             *
 *    Creates MySock class and initialize it. The second     *
 *  constructor uses an HTTP URI to read information stored  *
 *  there.                                                   *
 *                                                           *
 * Input:                                                    *
 *  string uri (in the second one)                           *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
MySock::MySock()
{
  this->error=NO_ERROR;
  this->connected=false;
}

MySock::MySock(string uri)
{
  int port;
  string host;
  string params;
  string protostr;
  string req;
  this->error=NO_ERROR;

  string::size_type pos = uri.find("//", 0);
  string::size_type pos2;

  if ((pos!=string::npos) && (pos==uri.find("/", 0)) && (pos>0)) // Sin barras antes
    {
      protostr = uri.substr(0,(pos-1));
      if (protostr=="http")
	{
	  port=80;
	  pos2=uri.find("/", (pos+2));
	  if (pos2==string::npos)
	    pos2=uri.length();
	  host=uri.substr(pos+2, pos2-pos-2);
	  params=uri.substr(pos2);

	  this->MakeConnection(host, port);
	  req="GET "+params+" HTTP/1.0"+CRLF+
	    "Host: "+host+"\r\n"+
	    CRLF;
	  if (this->connected)
	    this->SendData(req);
	}
      else
	{
	  this->error=NO_VALID_PROTOCOL;	  
	}
    }
  else
    this->error=NO_VALID_PROTOCOL;
}

/*************************************************************
 *     Constructor MySock()                                  *
 *************************************************************
 *  Description:                                             *
 *    Creates MySock class and initialize it. Connect with   *
 *  a remote server using specified port.                    *
 *                                                           *
 * Input:                                                    *
 *  string server - Server to connect to                     *
 *  int port - Port in the remote server                     *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
MySock::MySock(string server, int port)
{
  this->MakeConnection(server, port);
}

/*************************************************************
 *     Method: MakeConnection()                              *
 *************************************************************
 *  Description:                                             *
 *     Makes a connection to an specified server using a     *
 *  remote port                                              *
 *                                                           *
 * Input:                                                    *
 *   string server                                           *
 *   int port                                                *
 *                                                           *
 * Output:                                                   *
 *   Nothing                                                 *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
void MySock::MakeConnection(string server, int port)
{
  struct sockaddr_in serverAddress;
  struct hostent *hostInfo;
  this->connected=false;
  this->error=NO_ERROR;
  hostInfo = gethostbyname(server.data());
  this->error=(hostInfo==NULL)?CANT_RESOLVE_HOST:NO_ERROR;
  if (this->error==0)
    {
      this->sockd = socket(AF_INET, SOCK_STREAM, 0);
      this->error = (this->sockd<0)?CANT_CREATE_SOCKET:NO_ERROR;
    }

  // Connect to server.  First we have to set some fields in the
  // serverAddress structure.  The system will assign me an arbitrary
  // local port that is not in use.
  if (this->error==0)
    {
      serverAddress.sin_family = hostInfo->h_addrtype;
      memcpy((char *) &serverAddress.sin_addr.s_addr,
	     hostInfo->h_addr_list[0], hostInfo->h_length);
      serverAddress.sin_port = htons(port);
      
      this->error= (connect(this->sockd,
			    (struct sockaddr *) &serverAddress,
			    sizeof(serverAddress)) < 0)?CANT_CONNECT:NO_ERROR;
    }
  this->connected=(error==NO_ERROR);
}

/*************************************************************
 *     Method: SendData()                                    *
 *************************************************************
 *  Description:                                             *
 *     Sends information through the already stablished      *
 *  connection.                                              *
 *                                                           *
 * Input:                                                    *
 *     string data - Data being sent                         *
 *                                                           *
 * Output:                                                   *
 *     Nothing                                                      *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
int  MySock::SendData(string data)
{
  if (this->error==0)
    this->error = (write(this->sockd,data.data(),strlen(data.data()))<0)?CANT_SEND_DATA:NO_ERROR;
  return this->error;
}

/*************************************************************
 *     Method: GetTextData()                                 *
 *************************************************************
 *  Description:                                             *
 *     Read received data                                    *
 *                                                           *
 * Input:                                                    *
 *     Nothing                                               *
 *                                                           *
 * Output:                                                   *
 *     string - returns data received.                       *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
string MySock::GetTextData()
{
  int n;
  char buffer[1024];
  string out("");

  if (this->error==0)
    {

      bzero(buffer,1024);
      while ((n=read(this->sockd,buffer,1023)>0))
	{
	  if (n>=0)
	    out+=buffer;
	  else
	    {
	      error=CANT_READ_DATA;
	      break;
	    }
	  bzero(buffer,1024);
	}
      return out;
    }
  return NULL;
}


/*************************************************************
 *     Method: GetHTTPData()                                 *
 *************************************************************
 *  Description:                                             *
 *     We connect to an HTTP server and We Want to Read and  *
 *  identify headers and contents. We use this function      *
 *                                                           *
 * Input:                                                    *
 *     Nothing                                               *
 *                                                           *
 * Output:                                                   *
 *     HTTP_Request structure - Returns headers and data     *
 *                                                           *
 * Change History:                                           *
 *  Date      Author      Modification                       *
 *                                                           *
 *************************************************************/ 
HTTP_Request *MySock::GetHTTPData()
{
  string txtData;
  int i=0;			// Cuenta líneas
  string::size_type pos, pos2;
  HTTP_Request *http;
  string header;
  string value;
  bool err, data;
  char buf[255], buf2[255];
  TKey_Value hdata;

  if (this->error==NO_ERROR)
    {
      txtData=this->GetTextData();
      // Close the socket, we will analyze data
      this->closeConnection();

      data=false;
      err=false;
      pos=0;
      http=new HTTP_Request;
      while ((!data) && (!err))
 	{
	  pos2=txtData.find("\r\n",pos);      
	  if (pos2==string::npos)
	      err=true;
	  else if (pos2==pos)
	    data=true;
	  else
	  {
	    header=txtData.substr(pos,pos2-pos);
// 	    	      cout<<header<<"*"<<endl;
	    if (i==0)
	      {
		http->statusstr=header;
		sscanf(header.data(),"%s %s", buf, buf2); // HTTP/1.0 200
		http->status=atoi(buf2);
	      }
	    else
	      {
		hdata=extract_key_value(header);
		if (hdata.key=="Date:")
		  http->date=hdata.value;
		else if (hdata.key=="Server: ")
		  http->server=hdata.value;
		else if (hdata.key=="Last-Modified")
		  http->last_modified=hdata.value;
		else if (hdata.key=="Content-Type")
		  http->content_type=hdata.value;
		else if (hdata.key=="Content-Length")
		  http->content_length=hdata.value;
		}
	    pos=pos2+2;
	    i++;		
	  }
	  
	}
      if (err)
	{
	  this->error=NO_VALID_HEADERS;
	  return NULL;
	}
      else
	http->data=txtData.substr(pos2+2); // Skip \r\n
      
      return http;
    }
  return NULL;
}

void MySock::closeConnection()
{
  if (connected)
    {
      close(sockd);
      connected=false;
    }
}

MySock::~MySock()
{
  closeConnection();
}
// It is a test function to use MySock class. It was written for DWGO
// int main(int argc, char *argv[])
// {

//   string servidor;
//   MySock sock;
//   int portno;
//   HTTP_Request *http;

//     if (argc < 3) {
//        fprintf(stderr,"usage %s hostname port\n", argv[0]);
//        exit(0);
//     }
//     portno = atoi(argv[2]);
//     servidor=argv[1];
// //     sock = MySock(servidor, portno);
// //     sock.SendData("GET /pub/data/observations/metar/decoded/LEMG.TXT HTTP/1.0\n\n");

// //     cout<<sock.GetTextData();

//     sock = MySock("http://weather.noaa.gov/pub/data/observations/metar/decoded/LEMG.TXT");
//     //    cout<<"Error: "<<sock.error;
//     http=sock.GetHTTPData();
//     if (http==NULL)
//       cout<<"HUBO UN FALLO";
//     else
//       cout<<"Stado: "<<http->status<<"Fecha: "<<http->date<<endl<<"DATA:"<<http->data;
//     return 0;
// }
