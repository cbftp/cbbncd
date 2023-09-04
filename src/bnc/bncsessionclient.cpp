#include "bncsessionclient.h"

#include <cassert>

#include "../core/iomanager.h"

#include "../globalcontext.h"

#include "bncsession.h"

BncSessionClient::BncSessionClient(BncSession * parentsession) : session(parentsession), paused(false), connected(false) {
}

void BncSessionClient::activate(const std::string& sessiontag, const std::string& host, int port) {
  this->sessiontag = sessiontag;
  identreceived = false;
  paused = false;
  connected = false;
  global->log("[" + sessiontag + "] Connecting to server: " + host + ":" + std::to_string(port));
  sockid = global->getIOManager()->registerTCPClientSocket(this, host, port);
}

void BncSessionClient::ident(const std::string& ident) {
  this->identstr = ident;
  identreceived = true;
  checkSendIdent();
}

void BncSessionClient::disconnect() {
  connected = false;
  global->getIOManager()->closeSocket(sockid);
}

void BncSessionClient::FDConnected(int sockid) {
  connected = true;
  global->log("[" + sessiontag + "] Server connection established.");
  checkSendIdent();
}

void BncSessionClient::checkSendIdent() {
  if (identreceived && connected) {
    std::string identstring = "IDNT " + identstr;
    global->log("[" + sessiontag + "] Sending: " + identstring);
    global->log("[" + sessiontag + "] Bouncing enabled.");
    global->getIOManager()->sendData(sockid, identstring + "\r\n");
  }
}

void BncSessionClient::FDDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) {
  global->log("[" + sessiontag + "] Server closed the connection. Disconnecting client. Session finished.");
  connected = false;
  session->targetDisconnected();
}

void BncSessionClient::FDFail(int sockid, const std::string& err) {
  global->log("[" + sessiontag + "] Server connection failed: " + err);
  global->log("[" + sessiontag + "] Disconnecting client. Session finished.");
  session->targetDisconnected();
}

void BncSessionClient::FDData(int sockid, char* data, unsigned int datalen) {
  if (!session->targetData(data, datalen) && !paused) {
    global->getIOManager()->pause(sockid);
    paused = true;
  }
}

void BncSessionClient::FDSendComplete(int sockid) {
  session->targetSendComplete();
}

void BncSessionClient::sendComplete() {
  if (paused) {
    global->getIOManager()->resume(sockid);
    paused = false;
  }
}

bool BncSessionClient::sendData(const char* data, unsigned int datalen) {
  assert (identreceived);
  return global->getIOManager()->sendData(sockid, data, datalen);
}
