#include "bncsession.h"

#include <sstream>

#include "../core/iomanager.h"

#include "../globalcontext.h"
#include "../util.h"

#include "bncsessionclient.h"
#include "ident.h"

BncSession::BncSession(int listenport, Core::AddressFamily addrfam, const std::string& host, int port, bool ident, bool traffic) :
  sessionclient(new BncSessionClient(this, traffic)),
  identp(ident ? new Ident(this) : nullptr),
  state(State::DISCONNECTED),
  listenport(listenport),
  siteaddrfam(addrfam),
  sitehost(host),
  siteport(port),
  sockid(-1),
  paused(false)
{
}

bool BncSession::active() {
  return state != State::DISCONNECTED;
}

void BncSession::activate(int sockid) {
  this->sockid = sockid;
  paused = false;
  global->getIOManager()->registerTCPServerClientSocket(this, sockid);
  srcaddr = global->getIOManager()->getSocketAddress(sockid);
  Core::StringResult res = global->getIOManager()->getInterfaceAddress4(sockid);
  if (res.success) {
    ifaddr4 = res.result;
  }
  res = global->getIOManager()->getInterfaceAddress6(sockid);
  if (res.success) {
    ifaddr6 = res.result;
  }
  srcaddrfam = global->getIOManager()->getAddressFamily(sockid);
  int srcport = global->getIOManager()->getSocketPort(sockid);
  sessiontag = util::ipFormat(srcaddrfam, srcaddr) + ":" + std::to_string(srcport);
  global->log("[" + sessiontag + "] New client connection");
  sessionclient->activate(sessiontag, srcaddrfam, srcaddr, siteaddrfam, sitehost, siteport, ifaddr4, ifaddr6);
  if (identp != nullptr) {
    state = State::IDENT;
    identp->activate(sessiontag, srcaddrfam, srcaddr, srcport, listenport);
  }
  else {
    state = State::ESTABLISHED;
    sessionclient->ident("*@" + srcaddr + ":" + srcaddr);
  }
}

void BncSession::ident(const std::string& ident) {
  if (state == State::IDENT) {
    state = State::ESTABLISHED;
    sessionclient->ident(ident + '@' + util::ipFormat(srcaddrfam, srcaddr) + ":" + util::ipFormat(srcaddrfam, srcaddr));
    sendQueuedData();
  }
}

void BncSession::sendQueuedData() {
  while (!sendqueue.empty() && !paused) {
    const std::vector<char>& data = sendqueue.front();
    bool needspause = !sessionclient->sendData(const_cast<char*>(data.data()), data.size());
    sendqueue.pop_front();
    if (needspause) {
      if (!paused) {
        global->getIOManager()->pause(sockid);
        paused = true;
      }
      break;
    }
  }
}

void BncSession::FDDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) {
  if (state == State::IDENT) {
    identp->close();
  }
  global->log("[" + sessiontag + "] Client closed the connection. Disconnecting server. Session finished.");
  sessionclient->disconnect();
  sendqueue.clear();
  state = State::DISCONNECTED;
}

void BncSession::FDData(int sockid, char* data, unsigned int datalen) {
  if (state == State::IDENT) {
    sendqueue.emplace_back(data, data + datalen);
  }
  else if (state == State::ESTABLISHED) {
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

void BncSession::FDSSLSuccess(int sockid, const std::string& cipher) {
  sessionclient->parentTLSSuccess(cipher);
}

void BncSession::targetDisconnected() {
  if (state == State::IDENT) {
    identp->close();
  }
  global->getIOManager()->closeSocket(sockid);
  sendqueue.clear();
  state = State::DISCONNECTED;
}

bool BncSession::targetData(char* data, unsigned int datalen) {
  return global->getIOManager()->sendData(sockid, data, datalen);
}

void BncSession::negotiateTLS() {
  global->getIOManager()->negotiateSSLAccept(sockid);
}
