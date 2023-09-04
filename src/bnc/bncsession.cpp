#include "bncsession.h"

#include <sstream>

#include "../core/iomanager.h"
#include "../core/util.h"

#include "../globalcontext.h"

#include "bncsessionclient.h"
#include "ident.h"

BncSession::BncSession(int listenport, const std::string & host, int port, bool ident) :
  sessionclient(new BncSessionClient(this)),
  identp(ident ? new Ident(this) : NULL),
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
  sessiontag = srcaddr + ":" + coreutil::int2Str(srcport);
  global->log("[" + sessiontag + "] New client connection");
  if (identp != NULL) {
    state = BNC_IDENT;
    identp->activate(srcaddr, srcport, listenport);
  }
  else {
    state = BNC_ESTABLISHED;
    sessionclient->activate(sessiontag, host, port, "*@" + srcaddr);
  }
}

void BncSession::ident(const std::string & ident) {
  if (state == BNC_IDENT) {
    state = BNC_ESTABLISHED;
    sessionclient->activate(sessiontag, host, port, ident + '@' + srcaddr);
  }
}

void BncSession::FDDisconnected(int) {
  if (state == BNC_IDENT) {
    identp->close();
    global->log("[" + sessiontag + "] Client closed the connection. Session finished.");
  }
  else {
    global->log("[" + sessiontag + "] Client closed the connection. Disconnecting server. Session finished.");
    sessionclient->disconnect();
  }
  state = BNC_DISCONNECTED;
}

void BncSession::FDData(int, char * data, unsigned int datalen) {
  if (state == BNC_ESTABLISHED) {
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
  }
}

void BncSession::FDSendComplete(int) {
  sessionclient->sendComplete();
}

void BncSession::targetDisconnected() {
  global->getIOManager()->closeSocket(sockid);
  state = BNC_DISCONNECTED;
}

bool BncSession::targetData(char * data, unsigned int datalen) {
  return global->getIOManager()->sendData(sockid, data, datalen);
}
