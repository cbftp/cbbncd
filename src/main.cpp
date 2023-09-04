#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>

#include "core/workmanager.h"
#include "core/tickpoke.h"
#include "core/iomanager.h"

#include "bnc/bnc.h"

#include "crypto.h"
#include "globalcontext.h"

int str2Int(std::string str) {
  int num;
  std::istringstream ss(str);
  ss >> num;
  return num;
}

void showInput(bool show) {
  termios tty;
  tcgetattr(STDIN_FILENO, &tty);
  if (show) {
    tty.c_lflag |= ECHO;
  }
  else {
    tty.c_lflag &= ~ECHO;
  }
  tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

BinaryData getPassphrase() {
  std::cerr << "Passphrase: ";
  showInput(false);
  std::string passphrase;
  std::getline(std::cin, passphrase);
  std::cerr << std::endl;
  showInput(true);
  return BinaryData(passphrase.begin(), passphrase.end());
}

void daemonize() {
  pid_t pid;
  if ((pid = fork())) {
    std::cerr << "Started in daemon mode as pid " <<  pid << "." << std::endl;
    exit(0);
  }
  setsid();
  chdir("/");
  umask(0);
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  open("/dev/null", O_RDONLY);
  open("/dev/null", O_WRONLY);
  open("/dev/null", O_WRONLY);
}

void parseData(const std::string & data, int & listenport, std::string & host, int & port, bool & ident) {
    size_t sep1 = data.find(";");
    size_t sep2 = data.find(";", sep1 + 1);
    size_t portsep = data.find(":", sep1 + 1);
    listenport = str2Int(data.substr(0, sep1));
    port = 21;
    if (portsep != std::string::npos) {
      host = data.substr(sep1 + 1, portsep - sep1 - 1);
      port = str2Int(data.substr(portsep + 1, sep2 - portsep));
    }
    else {
      host = data.substr(sep1 + 1, sep2 - sep1);
    }
    ident = data.substr(sep2 + 1) != "n";
}

bool isAscii(const BinaryData & data) {
  for (unsigned int i = 0; i < data.size(); ++i) {
    if (data[i] >= 128) {
      return false;
    }
  }
  return true;
}

int main(int argc, char ** argv) {
  bool daemon = false;
  if (argc >= 2) {
    if (!strcmp(argv[1], "-d") || !strcmp(argv[1], "--daemon")) {
      daemon = true;
    }
    else {
      std::cerr << "Usage: ./cbbncd [-d]" << std::endl << "  -d, --daemon    Start in daemon mode"  << std::endl;
      return 1;
    }
  }
  std::string data = DATA;
  if (data.find(":") == std::string::npos) {
    BinaryData passphrase = getPassphrase();
    BinaryData decodeddata;
    Crypto::base64Decode(BinaryData(data.begin(), data.end()), decodeddata);
    BinaryData decrypteddata;
    Crypto::decrypt(decodeddata, passphrase, decrypteddata);
    if (!isAscii(decrypteddata)) {
      std::cerr << "Error: Passphrase invalid. Exiting." << std::endl;
      exit(1);
    }
    data = std::string(decrypteddata.begin(), decrypteddata.end());
  }
  int listenport;
  std::string host;
  int port;
  bool ident;
  parseData(data, listenport, host, port, ident);

  if (daemon) {
    daemonize();
  }
  else {
    global->setVerbose(true);
    global->log("Starting in verbose foreground mode. Use -d to start in daemon mode.");
  }
  WorkManager * wm = new WorkManager();
  TickPoke * tp = new TickPoke(wm);
  IOManager * iom = new IOManager(wm, tp);
  global->linkComponents(wm, iom, tp);

  Bnc * bnc = new Bnc(listenport, host, port, ident);
  wm->init();
  iom->init();
  tp->tickerLoop();
}
