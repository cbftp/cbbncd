#pragma once

#include <memory>

#include "../core/types.h"

class PasvListener;
class PasvConn;
class PortConn;

class TrafficBncSession {
public:
  TrafficBncSession();
  ~TrafficBncSession();
  int activate(Core::AddressFamily pasvaddrfam, Core::AddressFamily portaddrfam, const std::string& host, int port, const std::string& sessiontag);
  void disconnect();
  void pasvConnected(int newsockid);
  void portConnectFailure();
  void portConnected();
  void pasvConnData(char* data, int unsigned len);
  void portConnData(char* data, int unsigned len);
  void pasvConnClosed();
  void portConnClosed();
  void pasvConnSendComplete();
  void portConnSendComplete();
private:
  void transferFinished();
  std::unique_ptr<PasvListener> pasvlistener;
  std::unique_ptr<PasvConn> pasvconn;
  std::unique_ptr<PortConn> portconn;
  std::string targethost;
  int targetport;
  std::string sessiontag;
  bool listenportreleased;
  Core::AddressFamily portaddrfam;
  Core::AddressFamily pasvaddrfam;
};
