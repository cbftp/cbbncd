#include "pasvlistener.h"

#include "../core/iomanager.h"

#include "../globalcontext.h"

#include "trafficbncsession.h"

PasvListener::PasvListener(TrafficBncSession* tbnc) : tbnc(tbnc), sockid(-1), failedlistenport(-1) {
}

bool PasvListener::listen(Core::AddressFamily addrfam, int port, const std::string& sessiontag) {
  disconnect();
  listenport = port;
  this->sessiontag = sessiontag;
  sockid = global->getIOManager()->registerTCPServerSocket(this, listenport, addrfam);
  if (sockid == -1) {
    failedlistenport = listenport;
  }
  return sockid != -1;
}

void PasvListener::disconnect() {
  if (sockid == -1) {
    return;
  }
  global->getIOManager()->closeSocket(sockid);
  sockid = -1;
}

void PasvListener::FDNew(int sockid, int newsockid) {
  tbnc->pasvConnected(newsockid);
  disconnect();
}

void PasvListener::FDFail(int sockid, const std::string& err) {
  global->log("[" + sessiontag + "] Failed to bind on port " + std::to_string(failedlistenport) + ": " + err);
  sockid = -1;
}

int PasvListener::getListenPort() const {
  return listenport;
}
