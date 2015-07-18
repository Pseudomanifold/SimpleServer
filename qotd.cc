#include "ClientSocket.hh"
#include "Server.hh"

#include <signal.h>

Server server;

void handleExitSignal( int /* signal */ )
{
  server.close();
}

int main( int /* argc */, char** /* argv */ )
{
  signal( SIGINT, handleExitSignal );

  server.setPort( 2048 );

  server.onAccept( [] ( std::unique_ptr<ClientSocket> socket )
  {
    socket->write( "Sorry, no quote today, mate.\n" );
    socket->close();
  } );

  server.listen();

  return 0;
}
