#include "ClientSocket.hh"

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include <stdexcept>

ClientSocket::ClientSocket( int fileDescriptor )
  : _fileDescriptor( fileDescriptor )
{
}

ClientSocket::~ClientSocket()
{
  close( _fileDescriptor );
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
