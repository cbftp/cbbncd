#include "bnc.h"

#include <cstdlib>

#include "../core/iomanager.h"

#include "../globalcontext.h"

#include "bncsession.h"

Bnc::Bnc(int listenport, Core::AddressFamily addrfam, const std::string& host, int port, bool ident,
  bool traffic, int pasvportfirst, int pasvportlast) : listenport(listenport),
  addrfam(addrfam), host(host), port(port), ident(ident), traffic(traffic),
  pasvportfirst(pasvportfirst), pasvportlast(pasvportlast)
{
  global->getIOManager()->registerTCPServerSocket(this, listenport, Core::AddressFamily::IPV4);
  global->getIOManager()->registerTCPServerSocket(this, listenport, Core::AddressFamily::IPV6);
}

void Bnc::FDNew(int sockid, int newsockid) {
  std::list<BncSession*>::iterator it;
  for (it = sessions.begin(); it != sessions.end(); it++) {
    if (!(*it)->active()) {
      (*it)->activate(newsockid);
      return;
    }
  }
  BncSession* session = new BncSession(listenport, addrfam, host, port, ident, traffic);
  session->activate(newsockid);
  sessions.push_back(session);
}

void Bnc::FDFail(int sockid, const std::string& err) {
  global->log("Error: failed to bind on port " + std::to_string(listenport) + ": " + err);
  exit(1);
}
