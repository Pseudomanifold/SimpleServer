#include "ClientSocket.hh"
#include "Server.hh"

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <stdexcept>

ClientSocket::ClientSocket( int fileDescriptor, Server& server )
  : _fileDescriptor( fileDescriptor )
  , _server( server )
{
}

ClientSocket::~ClientSocket()
{
}

int ClientSocket::fileDescriptor() const
{
  return _fileDescriptor;
}

void ClientSocket::close()
{
  _server.close( _fileDescriptor );
}

void ClientSocket::write( const std::string& data )
{
  auto result = send( _fileDescriptor,
                      reinterpret_cast<const void*>( data.c_str() ),
                      data.size(),
                      0 );

  if( result == -1 )
    throw std::runtime_error( std::string( strerror( errno ) ) );
}

std::string ClientSocket::read()
{
  std::string message;

  char buffer[256] = { 0 };
  ssize_t numBytes = 0;

  while( ( numBytes = recv( _fileDescriptor, buffer, sizeof(buffer), MSG_DONTWAIT ) ) > 0 )
  {
    buffer[numBytes]  = 0;
    message          += buffer;
  }

  return message;
}
