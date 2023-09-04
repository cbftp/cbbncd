#include "bnc.h"

#include <cstdlib>

#include "../core/iomanager.h"

#include "../globalcontext.h"

#include "bncsession.h"

Bnc::Bnc(int listenport, const std::string& host, int port, bool ident) : listenport(listenport), host(host), port(port), ident(ident) {
  global->getIOManager()->registerTCPServerSocket(this, listenport);
}

void Bnc::FDNew(int sockid, int newsockid) {
  std::list<BncSession*>::iterator it;
  for (it = sessions.begin(); it != sessions.end(); it++) {
    if (!(*it)->active()) {
      (*it)->activate(newsockid);
      return;
    }
  }
  BncSession* session = new BncSession(listenport, host, port, ident);
  session->activate(newsockid);
  sessions.push_back(session);
}

void Bnc::FDFail(int sockid, const std::string& err) {
  global->log("Error: failed to bind on port " + std::to_string(listenport) + ": " + err);
  exit(1);
}
