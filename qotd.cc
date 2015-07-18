#include "Server.hh"

int main( int argc, char** argv )
{
  Server server;
  server.setPort( 2048 );
  server.listen();

  return 0;
}
