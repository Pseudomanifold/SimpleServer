#include "ClientSocket.hh"
#include "Server.hh"

#include <signal.h>

#include <fstream>
#include <random>
#include <string>
#include <vector>

#include <iostream>

Server server;

void handleExitSignal( int /* signal */ )
{
  server.close();
}

int main( int /* argc */, char** /* argv */ )
{
  signal( SIGINT, handleExitSignal );

  server.setPort( 1031 );

  server.onRead( [&] ( std::weak_ptr<ClientSocket> socket )
  {
    if( auto s = socket.lock() )
    {
      auto data = s->read();
      s->write( data );
    }
  } );

  server.listen();

  return 0;
}
