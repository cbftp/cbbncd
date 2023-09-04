#include "ident.h"

#include "../core/iomanager.h"
#include "../core/tickpoke.h"
#include "../core/util.h"

#include "../globalcontext.h"

#include "bncsession.h"

Ident::Ident(BncSession * bncsession) : bncsession(bncsession), active(false) {
}

void Ident::activate(const std::string & sourcehost, int sourceport, int targetport) {
  this->sourceport = sourceport;
  this->targetport = targetport;
  sessiontag = sourcehost + ":" + coreutil::int2Str(sourceport);
  active = true;
  sockid = global->getIOManager()->registerTCPClientSocket(this, sourcehost, 113);
  global->log("[" + sessiontag + "] Connecting to client ident server " + sourcehost + ":113");
  global->getTickPoke()->startPoke(this, "Ident", 3000, sockid);
}

void Ident::FDConnected(int) {
  std::string identrequest = coreutil::int2Str(sourceport) + ", " + coreutil::int2Str(targetport);
  global->log("[" + sessiontag + "] Ident connection established. Sending ident request: " + identrequest);
  global->getIOManager()->sendData(sockid, identrequest + "\r\n");
}

void Ident::FDData(int, char * buf, unsigned int buflen) {
  std::string identstr(buf, buflen);
  size_t len = identstr.length();
  if (len > 2 && identstr[len - 2] == '\r') {
    identstr = identstr.substr(0, len - 2);
  }
  if (identstr.find("USERID") != std::string::npos) {
    size_t pos = identstr.rfind(":");
    if (pos != std::string::npos) {
      while (++pos < identstr.length() && identstr[pos] == ' ');
      std::string user = identstr.substr(pos);
      global->log("[" + sessiontag + "] Received ident response: " + identstr);
      bncsession->ident(user);
    }
  }
  else {
    global->log("[" + sessiontag + "] Received unknown ident response: " + identstr);
    noIdent();
  }
  global->getIOManager()->closeSocket(sockid);
  deactivate();
}

void Ident::FDDisconnected(int) {
  if (active) {
    global->log("[" + sessiontag + "] Client ident server closed the connection unexpectedly.");
    noIdent();
    deactivate();
  }
}

void Ident::FDFail(int sockid, const std::string & err) {
  if (active) {
    global->log("[" + sessiontag + "] Client ident server connection failed: " + err);
    noIdent();
    deactivate();
  }
}

void Ident::tick(int sockid) {
  if (active && sockid == this->sockid) {
    global->log("[" + sessiontag + "] Ident request timed out.");
    noIdent();
  }
  deactivate();
}

void Ident::deactivate() {
  active = false;
  global->getTickPoke()->stopPoke(this, sockid);
}

void Ident::noIdent() {
  bncsession->ident("*");
}

void Ident::close() {
  deactivate();
  global->getIOManager()->closeSocket(sockid);
}