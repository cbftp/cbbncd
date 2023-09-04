#pragma once

#include <list>
#include <string>

#include "../core/eventreceiver.h"

class BncSession;

class Bnc : private EventReceiver {
public:
  Bnc(int, const std::string &, int);
  void FDNew(int);
  void FDFail(int, const std::string &);
private:
  std::list<BncSession *> sessions;
  int listenport;
  std::string host;
  int port;
};
