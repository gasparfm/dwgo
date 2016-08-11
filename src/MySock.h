#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <iostream>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

using namespace std;
#define MAX_LINE 100
#define LINE_ARRAY_SIZE (MAX_LINE+1)

#define NO_ERROR                0
#define CANT_CREATE_SOCKET      20
#define CANT_RESOLVE_HOST       30
#define CANT_CONNECT            35
#define CANT_SEND_DATA          40
#define CANT_READ_DATA          45
#define NO_VALID_PROTOCOL       50
#define NO_VALID_HEADERS        52

#define CRLF "\r\n"
struct HTTP_Request
{
  string statusstr;
  int status;
  string date;
  string server;
  string last_modified;
  string content_type;
  string content_length;
  string data;
};

struct TKey_Value
{
  string key, value;
};

class MySock {
public:
  int error;
  bool connected;

  string GetTextData();
  int SendData(string data);
  HTTP_Request *GetHTTPData();
  void closeConnection();
  MySock();
  MySock(string server, int port);
  MySock(string uri);
  virtual ~MySock();
private:
  int sockd, puerto;
  struct hostent *server;

  void MakeConnection (string server, int port);
};

TKey_Value extract_key_value(string str);
