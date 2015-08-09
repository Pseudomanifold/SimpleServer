#ifndef SERVER_HH
#define SERVER_HH

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

class ClientSocket;

class Server
{
public:
  ~Server();

  void setBacklog( int backlog );
  void setPort( int port );

  void close();
  void listen();

  template <class F> void onAccept( F&& f )
  {
    _handleAccept = f;
  }

  template <class F> void onRead( F&& f )
  {
    _handleRead = f;
  }

  void close( int fileDescriptor );

private:
  int _backlog =  1;
  int _port    = -1;
  int _socket  = -1;

  std::function< void ( std::weak_ptr<ClientSocket> socket ) > _handleAccept;
  std::function< void ( std::weak_ptr<ClientSocket> socket ) > _handleRead;

  std::vector< std::shared_ptr<ClientSocket> > _clientSockets;

  std::vector<int> _staleFileDescriptors;
  std::mutex _staleFileDescriptorsMutex;
};

#endif
