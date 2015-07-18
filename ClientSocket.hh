#ifndef CLIENT_SOCKET_HH
#define CLIENT_SOCKET_HH

#include <string>

class ClientSocket
{
public:
  ClientSocket( int fileDescriptor );
  ~ClientSocket();

  void close();
  void write( const std::string& data );

  ClientSocket( const ClientSocket& )            = delete;
  ClientSocket& operator=( const ClientSocket& ) = delete;

private:
  int _fileDescriptor = -1;
};

#endif
