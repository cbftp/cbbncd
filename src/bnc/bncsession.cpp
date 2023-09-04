#include "bncsession.h"

#include <sstream>

#include "../core/iomanager.h"

#include "../globalcontext.h"

#include "bncsessionclient.h"
#include "ident.h"

BncSession::BncSession(int listenport, const std::string & host, int port, bool ident) :
  sessionclient(new BncSessionClient(this)),
  identp(ident ? new Ident(this) : nullptr),
  state(BNC_DISCONNECTED),
  listenport(listenport),
  host(host),
  port(port),
  sockid(-1),
  paused(false)
{
}

bool BncSession::active() {
  return state != BNC_DISCONNECTED;
}

void BncSession::activate(int sockid) {
  this->sockid = sockid;
  paused = false;
  global->getIOManager()->registerTCPServerClientSocket(this, sockid);
  srcaddr = global->getIOManager()->getSocketAddress(sockid);
  int srcport = global->getIOManager()->getSocketPort(sockid);
  sessiontag = srcaddr + ":" + std::to_string(srcport);
  global->log("[" + sessiontag + "] New client connection");
  sessionclient->activate(sessiontag, host, port);
  if (identp != nullptr) {
    state = BNC_IDENT;
    identp->activate(srcaddr, srcport, listenport);
  }
  else {
    state = BNC_ESTABLISHED;
    sessionclient->ident("*@" + srcaddr + ":" + srcaddr);
  }
}

void BncSession::ident(const std::string& ident) {
  if (state == BNC_IDENT) {
    state = BNC_ESTABLISHED;
    sessionclient->ident(ident + '@' + srcaddr + ":" + srcaddr);
    sendQueuedData();
  }
}

void BncSession::sendQueuedData() {
  while (!sendqueue.empty() && !paused) {
    const std::vector<char>& data = sendqueue.front();
    bool pause = sessionclient->sendData(const_cast<char*>(data.data()), data.size());
    sendqueue.pop_front();
    if (pause) {
      if (!paused) {
        global->getIOManager()->pause(sockid);
        paused = true;
      }
      break;
    }
  }
}

void BncSession::FDDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) {
  if (state == BNC_IDENT) {
    identp->close();
  }
  global->log("[" + sessiontag + "] Client closed the connection. Disconnecting server. Session finished.");
  sessionclient->disconnect();
  sendqueue.clear();
  state = BNC_DISCONNECTED;
}

void BncSession::FDData(int sockid, char* data, unsigned int datalen) {
  if (state == BNC_IDENT) {
    sendqueue.emplace_back(data, data + datalen);
  }
  else if (state == BNC_ESTABLISHED) {
    if (!sessionclient->sendData(data, datalen) && !paused) {
      global->getIOManager()->pause(sockid);
      paused = true;
    }
  }
}

void BncSession::targetSendComplete() {
  if (paused) {
    global->getIOManager()->resume(sockid);
    paused = false;
    sendQueuedData();
  }
}

void BncSession::FDSendComplete(int sockid) {
  sessionclient->sendComplete();
}

void BncSession::targetDisconnected() {
  if (state == BNC_IDENT) {
    identp->close();
  }
  global->getIOManager()->closeSocket(sockid);
  sendqueue.clear();
  state = BNC_DISCONNECTED;
}

bool BncSession::targetData(char* data, unsigned int datalen) {
  return global->getIOManager()->sendData(sockid, data, datalen);
}
