#include "bncsessionclient.h"

#include "../core/iomanager.h"
#include "../core/util.h"

#include "../globalcontext.h"

#include "bncsession.h"

BncSessionClient::BncSessionClient(BncSession * parentsession) : paused(false) {
  session = parentsession;
}

void BncSessionClient::activate(const std::string & sessiontag, const std::string & host, int port, const std::string & ident) {
  this->sessiontag = sessiontag;
  this->ident = ident;
  paused = false;
  global->log("[" + sessiontag + "] Connecting to server: " + host + ":" + coreutil::int2Str(port));
  sockid = global->getIOManager()->registerTCPClientSocket(this, host, port);
}

void BncSessionClient::disconnect() {
  global->getIOManager()->closeSocket(sockid);
}

void BncSessionClient::FDConnected(int) {
  std::string identstring = "IDNT " + ident;
  global->log("[" + sessiontag + "] Connection established. Sending: " + identstring);
  global->log("[" + sessiontag + "] Bouncing enabled.");
  global->getIOManager()->sendData(sockid, identstring + "\r\n");
}

void BncSessionClient::FDDisconnected(int) {
  global->log("[" + sessiontag + "] Server closed the connection. Disconnecting client. Session finished.");
  session->targetDisconnected();
}

void BncSessionClient::FDFail(int, const std::string & err) {
  global->log("[" + sessiontag + "] Server connection failed: " + err);
  global->log("[" + sessiontag + "] Disconnecting client. Session finished.");
  session->targetDisconnected();
}

void BncSessionClient::FDData(int, char * data, unsigned int datalen) {
  if (!session->targetData(data, datalen) && !paused) {
    global->getIOManager()->pause(sockid);
    paused = true;
  }
}

void BncSessionClient::FDSendComplete(int) {
  session->targetSendComplete();
}

void BncSessionClient::sendComplete() {
  if (paused) {
    global->getIOManager()->resume(sockid);
    paused = false;
  }
}

bool BncSessionClient::sendData(char * data, unsigned int datalen) {
  return global->getIOManager()->sendData(sockid, data, datalen);
}
