#include "Server.hh"

#include <signal.h>

Server server;

void handleExitSignal( int /* signal */ )
{
  server.close();
}

int main( int argc, char** argv )
{
  signal( SIGINT, handleExitSignal );

  server.setPort( 2048 );
  server.listen();

  return 0;
}
