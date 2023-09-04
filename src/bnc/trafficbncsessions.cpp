#include "trafficbncsessions.h"

#include "trafficbncsession.h"

TrafficBncSessions::TrafficBncSessions() {
}

TrafficBncSessions::~TrafficBncSessions() {
}

int TrafficBncSessions::activate(Core::AddressFamily pasvaddrfam, Core::AddressFamily portaddrfam, const std::string& host, int port, const std::string& sessiontag) {
  for (const std::unique_ptr<TrafficBncSession>& session : tbncsessions) {
    if (!session->isActive()) {
      return session->activate(pasvaddrfam, portaddrfam, host, port, sessiontag);
    }
  }
  tbncsessions.emplace_back(new TrafficBncSession(tbncsessions.size()));
  return tbncsessions.back()->activate(pasvaddrfam, portaddrfam, host, port, sessiontag);
}

void TrafficBncSessions::disconnect() {
  for (const std::unique_ptr<TrafficBncSession>& session : tbncsessions) {
    session->disconnect();
  }
}
