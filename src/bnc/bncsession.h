#pragma once

#include <list>
#include <string>
#include <vector>

#include "../core/eventreceiver.h"

class BncSessionClient;
class Ident;

enum State {
  BNC_DISCONNECTED,
  BNC_IDENT,
  BNC_ESTABLISHED
};

class BncSession : private Core::EventReceiver {
public:
  BncSession(int listenport, const std::string& host, int port, bool ident);
  bool active();
  void activate(int sockid);
  void targetDisconnected();
  bool targetData(char* data, unsigned int datalen);
  void targetSendComplete();
  void ident(const std::string& ident);
private:
  void FDDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) override;
  void FDData(int sockid, char* data, unsigned int datalen) override;
  void FDSendComplete(int sockid) override;
  void sendQueuedData();
  BncSessionClient* sessionclient;
  Ident* identp;
  int state;
  int listenport;
  std::string host;
  int port;
  int sockid;
  std::string srcaddr;
  std::string sessiontag;
  bool paused;
  std::list<std::vector<char>> sendqueue;
};
