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
  this->close();
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
  if( _socket )
    ::close( _socket );

  for( auto&& clientSocket : _clientSockets )
    clientSocket->close();

  _clientSockets.clear();
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

    // Will be updated in the loop as soon as a new client has been
    // accepted. This saves us from modifying the variable *during*
    // the loop execution.
    int newHighestFileDescriptor = highestFileDescriptor;

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

        FD_SET( clientFileDescriptor, &masterSocketSet );
        newHighestFileDescriptor = std::max( highestFileDescriptor, clientFileDescriptor );

        auto clientSocket = std::make_shared<ClientSocket>( clientFileDescriptor, *this );

        if( _handleAccept )
          auto result = std::async( std::launch::async, _handleAccept, clientSocket );

        _clientSockets.push_back( clientSocket );
      }

      // Known client socket
      else
      {
        char buffer[2] = {0,0};

        // Let's attempt to read at least one byte from the connection, but
        // without removing it from the queue. That way, the server can see
        // whether a client has closed the connection.
        int result = recv( i, buffer, 1, MSG_PEEK );

        if( result <= 0 )
        {
          FD_CLR( i, &masterSocketSet );

          // FIXME: This is horrible; it would be easier to use erase-remove
          // here, but this leads to a deadlock.
          this->close( i );
        }
        else
        {
          auto itSocket = std::find_if( _clientSockets.begin(), _clientSockets.end(),
                                        [&] ( std::shared_ptr<ClientSocket> socket )
                                        {
                                          return socket->fileDescriptor() == i;
                                        } );

          if( itSocket != _clientSockets.end() && _handleRead )
            auto result = std::async( std::launch::async, _handleRead, *itSocket );
        }
      }
    }

    // Update the file descriptor if a new client has been accepted in
    // the loop above.
    highestFileDescriptor = std::max( newHighestFileDescriptor, highestFileDescriptor );

    // Handle stale connections. This is in an extra scope so that the
    // lock guard unlocks the mutex automatically.
    {
      std::lock_guard<std::mutex> lock( _staleFileDescriptorsMutex );

      for( auto&& fileDescriptor : _staleFileDescriptors )
      {
        FD_CLR( fileDescriptor, &masterSocketSet );
        ::close( fileDescriptor );
      }

      _staleFileDescriptors.clear();
    }
  }
}

void Server::close( int fileDescriptor )
{
  std::lock_guard<std::mutex> lock( _staleFileDescriptorsMutex );

  _clientSockets.erase( std::remove_if( _clientSockets.begin(), _clientSockets.end(),
                                        [&] ( std::shared_ptr<ClientSocket> socket )
                                        {
                                          return socket->fileDescriptor() == fileDescriptor;
                                        } ),
                                        _clientSockets.end() );

  _staleFileDescriptors.push_back( fileDescriptor );
}
