#pragma once

#include <map>
#include <set>

#include "core/eventreceiver.h"

class ListenPortManager : private Core::EventReceiver {
public:
  ListenPortManager(int firstport = 50000, int lastport = 51000);
  ~ListenPortManager();
  void setPortRange(int first, int last);
  int acquirePort();
  void releasePort(int port);
  void markPortUnavailable(int port);
private:
  void tick(int message) override;
  std::map<int, unsigned int> unavailableports;
  std::set<int> availableports;
  std::set<int> outstandingports;
  int firstport;
  int lastport;
};
