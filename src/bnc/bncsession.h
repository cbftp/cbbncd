#pragma once

#include <string>

#include "../core/eventreceiver.h"

class BncSessionClient;
class Ident;

enum State {
  BNC_DISCONNECTED,
  BNC_IDENT,
  BNC_ESTABLISHED
};

class BncSession : private EventReceiver {
public:
  BncSession(int, const std::string &, int, bool);
  bool active();
  void activate(int);
  void FDDisconnected(int);
  void FDData(int, char *, unsigned int);
  void FDSendComplete(int);
  void targetDisconnected();
  bool targetData(char *, unsigned int);
  void targetSendComplete();
  void ident(const std::string &);
private:
  BncSessionClient * sessionclient;
  Ident * identp;
  int state;
  int listenport;
  std::string host;
  int port;
  int sockid;
  std::string srcaddr;
  std::string sessiontag;
  bool paused;
};
