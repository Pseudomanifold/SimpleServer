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
#include <future>
#include <memory>
#include <stdexcept>
#include <string>

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

  {
    int option = 1;

    setsockopt( _socket,
                SOL_SOCKET,
                SO_REUSEADDR,
                reinterpret_cast<const void*>( &option ),
                sizeof( option ) );
  }

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

  fd_set masterSocketSet;
  fd_set clientSocketSet;

  FD_ZERO( &masterSocketSet );
  FD_SET( _socket, &masterSocketSet );

  int highestFileDescriptor = _socket;

  while( 1 )
  {
    clientSocketSet = masterSocketSet;

    int numFileDescriptors = select( highestFileDescriptor + 1,
                                     &clientSocketSet,
                                     nullptr,   // no descriptors to write into
                                     nullptr,   // no descriptors with exceptions
                                     nullptr ); // no timeout

    if( numFileDescriptors == -1 )
      break;

    for( int i = 0; i <= highestFileDescriptor; i++ )
    {
      if( !FD_ISSET( i, &clientSocketSet ) )
        continue;

      // Handle new client
      if( i == _socket )
      {
        sockaddr_in clientAddress;
        auto clientAddressLength = sizeof(clientAddress);

        int clientFileDescriptor = accept( _socket,
                                           reinterpret_cast<sockaddr*>( &clientAddress ),
                                           reinterpret_cast<socklen_t*>( &clientAddressLength ) );

        if( clientFileDescriptor == -1 )
          break;

        auto clientSocket = std::unique_ptr<ClientSocket>( new ClientSocket( clientFileDescriptor ) );
        auto result       = std::async( std::launch::async, _handleAccept, std::move( clientSocket ) );
      }
    }
  }
}
