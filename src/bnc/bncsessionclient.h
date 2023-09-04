#pragma once

#include "../core/eventreceiver.h"

class BncSession;

class BncSessionClient : private EventReceiver {
public:
  BncSessionClient(BncSession *);
  void activate(const std::string &, const std::string &, int, const std::string &);
  void disconnect();
  void FDConnected(int);
  void FDData(int, char *, unsigned int);
  void FDDisconnected(int);
  void FDFail(int, const std::string &);
  void FDSendComplete(int);
  bool sendData(char *, unsigned int);
  void sendComplete();
private:
  int sockid;
  BncSession * session;
  std::string ident;
  std::string sessiontag;
  bool paused;
};
