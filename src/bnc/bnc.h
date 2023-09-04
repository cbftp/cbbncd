#pragma once

#include <list>
#include <string>

#include "../core/eventreceiver.h"

class BncSession;

class Bnc : private Core::EventReceiver {
public:
  Bnc(int listenport, const std::string& host, int port, bool ident);
private:
  void FDNew(int sockid, int newsockid) override;
  void FDFail(int sockid, const std::string& error) override;
  std::list<BncSession*> sessions;
  int listenport;
  std::string host;
  int port;
  bool ident;
};
