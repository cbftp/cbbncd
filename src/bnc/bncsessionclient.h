#pragma once

#include <list>
#include <vector>

#include "../core/eventreceiver.h"

class BncSession;

class BncSessionClient : private Core::EventReceiver {
public:
  BncSessionClient(BncSession* parentsession);
  void activate(const std::string& sessiontag, const std::string& host, int port);
  void ident(const std::string& ident);
  void disconnect();
  bool sendData(const char* data, unsigned int datalen);
  void sendComplete();
private:
  void FDConnected(int sockid) override;
  void FDData(int sockid, char* data, unsigned int datalen) override;
  void FDDisconnected(int sockid, Core::DisconnectType reason, const std::string& details) override;
  void FDFail(int sockid, const std::string& err) override;
  void FDSendComplete(int sockid) override;
  void checkSendIdent();
  int sockid;
  BncSession* session;
  std::string identstr;
  bool identreceived;
  std::string sessiontag;
  bool paused;
  bool connected;
};
