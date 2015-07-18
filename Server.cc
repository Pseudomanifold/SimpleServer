#include "ClientSocket.hh"
#include "Server.hh"

#include <errno.h>
#include <string.h>

#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <unistd.h>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <thread>

Server::~Server()
{
  if( _socket )
    ::close( _socket );
}

void Server::setBacklog( int backlog )
{
  _backlog = backlog;
}

void Server::setPort( int port )
{
  _port = port;
}

void Server::close()
{
  ::close( _socket );
  _socket = -1;
}

void Server::listen()
{
  _socket = socket( AF_INET, SOCK_STREAM, 0 );

  if( _socket == -1 )
    throw std::runtime_error( std::string( strerror( errno ) ) );

  sockaddr_in socketAddress;

  std::fill( reinterpret_cast<char*>( &socketAddress ),
             reinterpret_cast<char*>( &socketAddress ) + sizeof( socketAddress ),
             0 );

  socketAddress.sin_family      = AF_INET;
  socketAddress.sin_addr.s_addr = htonl( INADDR_ANY );
  socketAddress.sin_port        = htons( _port );

  {
    int result = bind( _socket,
                       reinterpret_cast<const sockaddr*>( &socketAddress ),
                       sizeof( socketAddress ) );

    if( result == -1 )
      throw std::runtime_error( std::string( strerror( errno ) ) );
  }

  {
    int result = ::listen( _socket, _backlog );

    if( result == -1 )
      throw std::runtime_error( std::string( strerror( errno ) ) );
  }

  while( 1 )
  {
    sockaddr_in clientAddress;
    auto clientAddressLength = sizeof(clientAddress);

    int clientFileDescriptor = accept( _socket,
                                       reinterpret_cast<sockaddr*>( &clientAddress ),
                                       reinterpret_cast<socklen_t*>( &clientAddressLength ) );

    if( clientFileDescriptor == -1 )
      break;

    ClientSocket clientSocket( clientFileDescriptor );
    clientSocket.write( "This is a test.\n" );
  }
}
