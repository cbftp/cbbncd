#include "trafficbncsession.h"

#include "../globalcontext.h"
#include "../listenportmanager.h"
#include "../util.h"

#include "pasvlistener.h"
#include "pasvconn.h"
#include "portconn.h"

TrafficBncSession::TrafficBncSession(int id) : pasvlistener(new PasvListener(this)),
  pasvconn(new PasvConn(this)), portconn(new PortConn(this)), active(false),
  listenportreleased(true), id(id)
{
}

TrafficBncSession::~TrafficBncSession() {
}

int TrafficBncSession::activate(Core::AddressFamily pasvaddrfam, Core::AddressFamily portaddrfam, const std::string& host, int port, const std::string& sessiontag) {
  this->portaddrfam = portaddrfam;
  this->pasvaddrfam = pasvaddrfam;
  this->targethost = host;
  this->targetport = port;
  this->sessiontag = sessiontag + " #" + std::to_string(id);
  bool success = false;
  int bindport = -1;
  if (!listenportreleased) {
    global->getListenPortManager()->releasePort(pasvlistener->getListenPort());
    listenportreleased = true;
  }
  while (!success) {
    bindport = global->getListenPortManager()->acquirePort();
    if (bindport == -1) {
      // exhausted
      return -1;
    }
    success = pasvlistener->listen(pasvaddrfam, bindport, sessiontag);
  }
  global->log("[" + sessiontag + "] Listening for a traffic connection on " +
    (pasvaddrfam == Core::AddressFamily::IPV6 ? "IPv6 " : "") + "port " + std::to_string(bindport));
  listenportreleased = false;
  active = true;
  return bindport;
}

void TrafficBncSession::disconnect() {
  pasvconn->disconnect();
  portconn->disconnect();
  pasvlistener->disconnect();
  if (!listenportreleased) {
    global->getListenPortManager()->releasePort(pasvlistener->getListenPort());
    listenportreleased = true;
  }
  active = false;
}

void TrafficBncSession::pasvConnected(int newsockid) {
  pasvconn->activate(newsockid, sessiontag);
  global->log("[" + sessiontag + "] Traffic connection established from " +
    util::ipFormat(pasvaddrfam, pasvconn->getAddress()) + ":" + std::to_string(pasvconn->getPort()));
  portconn->activate(portaddrfam, targethost, targetport, sessiontag);
  global->log("[" + sessiontag + "] Opening traffic connection to " +
    util::ipFormat(portaddrfam, targethost) + ":" + std::to_string(targetport));
  global->getListenPortManager()->releasePort(pasvlistener->getListenPort());
  listenportreleased = true;
}

void TrafficBncSession::portConnectFailure() {
  pasvconn->disconnect();
  active = false;
}

void TrafficBncSession::portConnected() {
  pasvconn->startPolling();
  global->log("[" + sessiontag + "] Both traffic connections established. Traffic bouncing active: " +
    util::ipFormat(portaddrfam, targethost) + ":" + std::to_string(targetport) + " <-> " +
    util::ipFormat(pasvaddrfam, pasvconn->getAddress()) + ":" + std::to_string(pasvconn->getPort()));
}

void TrafficBncSession::pasvConnData(char* data, unsigned int len) {
  bool needspause = !portconn->sendData(data, len);
  if (needspause) {
    pasvconn->pause();
  }
}

void TrafficBncSession::portConnData(char* data, unsigned int len) {
  bool needspause = !pasvconn->sendData(data, len);
  if (needspause) {
    portconn->pause();
  }
}

void TrafficBncSession::pasvConnClosed() {
  portconn->disconnect();
  transferFinished();
}

void TrafficBncSession::portConnClosed() {
  pasvconn->disconnect();
  transferFinished();
}

void TrafficBncSession::transferFinished() {
  global->log("[" + sessiontag + "] Traffic connections closed. Transfer complete, traffic bounce session finished.");
  active = false;
}

void TrafficBncSession::pasvConnSendComplete() {
  if (portconn->isPaused()) {
    portconn->resume();
  }
}

void TrafficBncSession::portConnSendComplete() {
  if (pasvconn->isPaused()) {
    pasvconn->resume();
  }
}

bool TrafficBncSession::isActive() const {
  return active;
}
