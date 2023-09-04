#pragma once

#include <string>

#include "../core/eventreceiver.h"

class BncSession;

class Ident : public EventReceiver {
public:
  Ident(BncSession *);
  void activate(const std::string &, int, int);
  void close();
  void FDConnected(int);
  void FDData(int, char *, unsigned int);
  void FDDisconnected(int);
  void FDFail(int, const std::string &);
  void tick(int);
private:
  void deactivate();
  void noIdent();
  BncSession * bncsession;
  bool active;
  int sockid;
  int sourceport;
  int targetport;
  std::string sessiontag;
};
