#pragma once

#include <list>
#include <string>

#include "../core/eventreceiver.h"
#include "../core/types.h"

class BncSession;

class Bnc : private Core::EventReceiver {
public:
  Bnc(int listenport, Core::AddressFamily addrfam, const std::string& host, int port, bool ident, bool traffic, int pasvportfirst, int pasvportlast);
private:
  void FDNew(int sockid, int newsockid) override;
  void FDFail(int sockid, const std::string& error) override;
  std::list<BncSession*> sessions;
  int listenport;
  Core::AddressFamily addrfam;
  std::string host;
  int port;
  bool ident;
  bool traffic;
  int pasvportfirst;
  int pasvportlast;
};
