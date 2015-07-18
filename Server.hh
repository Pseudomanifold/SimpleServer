#ifndef SERVER_HH
#define SERVER_HH

#include <functional>

class Server
{
public:
  ~Server();

  void setBacklog( int backlog );
  void setPort( int port );

  void close();
  void listen();

private:
  int _backlog =  1;
  int _port    = -1;
  int _socket  = -1;
};

#endif
