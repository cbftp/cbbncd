#pragma once

#include "../core/eventreceiver.h"

class BncSession;

class BncSessionClient : private Core::EventReceiver {
public:
  BncSessionClient(BncSession* parentsession);
  void activate(const std::string& sessiontag, const std::string& host, int port, const std::string& ident);
  void disconnect();
  bool sendData(char* data, unsigned int datalen);
  void sendComplete();
private:
  void FDConnected(int sockid) override;
  void FDData(int sockid, char* data, unsigned int datalen) override;
  void FDDisconnected(int sockid) override;
  void FDFail(int sockid, const std::string& err) override;
  void FDSendComplete(int sockid) override;
  int sockid;
  BncSession* session;
  std::string ident;
  std::string sessiontag;
  bool paused;
};
