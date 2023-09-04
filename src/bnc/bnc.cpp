#include "bnc.h"

#include <cstdlib>

#include "../core/iomanager.h"
#include "../core/util.h"

#include "../globalcontext.h"

#include "bncsession.h"

Bnc::Bnc(int listenport, const std::string & host, int port) : listenport(listenport), host(host), port(port) {
  global->getIOManager()->registerTCPServerSocket(this, listenport);
}

void Bnc::FDNew(int sockid) {
  std::list<BncSession *>::iterator it;
  for (it = sessions.begin(); it != sessions.end(); it++) {
    if (!(*it)->active()) {
      (*it)->activate(sockid);
      return;
    }
  }
  BncSession * session = new BncSession(listenport, host, port);
  session->activate(sockid);
  sessions.push_back(session);
}

void Bnc::FDFail(int sockid, const std::string & err) {
  global->log("Error: failed to bind on port " + coreutil::int2Str(listenport) + ": " + err);
  exit(1);
}
