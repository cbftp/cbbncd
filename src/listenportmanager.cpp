#include "listenportmanager.h"

#include <ctime>
#include <list>

#include "core/tickpoke.h"
#include "globalcontext.h"


namespace {

const int tickinterval = 600000; // 10 minutes
const std::string tag = "[ListenPortManager] ";

}

ListenPortManager::ListenPortManager(int firstport, int lastport) : firstport(firstport), lastport(lastport) {
  global->getTickPoke()->startPoke(this, "ListenPortManager", tickinterval, 0);
  setPortRange(firstport, lastport);
}

ListenPortManager::~ListenPortManager() {
  global->getTickPoke()->stopPoke(this, 0);
}

void ListenPortManager::setPortRange(int first, int last) {
  firstport = first;
  lastport = last;
  availableports.clear();
  for (int i = first; i <= last; ++i) {
    availableports.insert(i);
  }
  std::list<int> eraselist;
  for (int outstandingport : outstandingports) {
    if (outstandingport < firstport || outstandingport > lastport) {
      eraselist.push_back(outstandingport);
      continue;
    }
    availableports.erase(outstandingport);
  }
  for (int eraseport : eraselist) {
    outstandingports.erase(eraseport);
  }
}

int ListenPortManager::acquirePort() {
  if (availableports.empty()) {
    return -1;
  }
  int port = *availableports.begin();
  availableports.erase(port);
  outstandingports.insert(port);
  return port;
}

void ListenPortManager::releasePort(int port) {
  if (port < firstport || port > lastport) {
    return;
  }
  outstandingports.erase(port);
  availableports.insert(port);
}

void ListenPortManager::markPortUnavailable(int port) {
  if (port < firstport || port > lastport) {
    return;
  }
  global->log(tag + "Marking port as unavailable: " + std::to_string(port));
  availableports.erase(port);
  outstandingports.erase(port);
  unavailableports[port] = time(nullptr);
}

void ListenPortManager::tick(int message) {
  unsigned int currtime = static_cast<unsigned int>(time(nullptr));
  std::list<int> expiredlist;
  for (const std::pair<const int, unsigned int>& portandtime : unavailableports) {
    if (portandtime.second + 3600 < currtime) {
      expiredlist.push_back(portandtime.first);
    }
  }
  for (int expiredunavailableport : expiredlist) {
    global->log(tag + "Unavailable port block expired: " + std::to_string(expiredunavailableport));
    unavailableports.erase(expiredunavailableport);
    releasePort(expiredunavailableport);
  }
}
