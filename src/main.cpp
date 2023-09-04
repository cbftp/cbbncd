#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <list>
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

Core::BinaryData getPassphrase() {
  std::cerr << "Passphrase: ";
  showInput(false);
  std::string passphrase;
  std::getline(std::cin, passphrase);
  std::cerr << std::endl;
  showInput(true);
  return Core::BinaryData(passphrase.begin(), passphrase.end());
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

std::list<std::string> split(const std::string& in, const std::string& sep) {
  std::list<std::string> out;
  size_t start = 0;
  size_t end;
  size_t seplength = sep.length();
  while ((end = in.find(sep, start)) != std::string::npos) {
    out.push_back(in.substr(start, end - start));
    start = end + seplength;
  }
  out.push_back(in.substr(start));
  return out;
}

void parseData(const std::string& data, int& listenport, std::string& host, int& port, bool& ident, bool& bind, std::string& ipif) {
  std::list<std::string> params = split(data, ";");
  for (const std::string& param : params) {
    size_t sep = param.find("=");
    std::string key = param.substr(0, sep);
    std::string value = param.substr(sep + 1);
    if (key == "port") {
      listenport = std::stol(value);
    }
    else if (key == "host") {
      size_t portsep = value.find(":");
      port = 21;
      if (portsep != std::string::npos) {
        host = value.substr(0, portsep);
        port = std::stol(value.substr(portsep + 1));
      }
      else {
        host = value;;
      }
    }
    else if (key == "ident") {
      ident = value == "true";
    }
    else if (key == "bind") {
      bind = value == "true";
    }
    else if (key == "ipif") {
      ipif = value;
    }
  }
}

bool isAscii(const Core::BinaryData& data) {
  for (unsigned int i = 0; i < data.size(); ++i) {
    if (data[i] >= 128) {
      return false;
    }
  }
  return true;
}

int main(int argc, char** argv) {
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
  std::string data = BNCDATA;
  if (data.find(";") == std::string::npos) {
    Core::BinaryData passphrase = getPassphrase();
    Core::BinaryData decodeddata;
    Crypto::base64Decode(Core::BinaryData(data.begin(), data.end()), decodeddata);
    Core::BinaryData decrypteddata;
    Crypto::decrypt(decodeddata, passphrase, decrypteddata);
    if (!isAscii(decrypteddata)) {
      Crypto::decryptOld(decodeddata, passphrase, decrypteddata);
      if (!isAscii(decrypteddata)) {
        std::cerr << "Error: Passphrase invalid. Exiting." << std::endl;
        exit(1);
      }
    }
    data = std::string(decrypteddata.begin(), decrypteddata.end());
  }
  int listenport = 65432;
  std::string host = "ftp.example.com";
  int port = 21;
  bool ident = true;
  bool bind = false;
  std::string ipif = "0.0.0.0";
  parseData(data, listenport, host, port, ident, bind, ipif);

  if (daemon) {
    daemonize();
  }
  else {
    global->setVerbose(true);
    global->log("Starting in verbose foreground mode. Use -d to start in daemon mode.");
  }
  Core::WorkManager* wm = new Core::WorkManager();
  Core::TickPoke* tp = new Core::TickPoke(*wm);
  Core::IOManager* iom = new Core::IOManager(*wm, *tp);
  if (bind) {
    std::list<std::pair<std::string, std::string>> ifs = iom->listInterfaces();
    bool isif = false;
    for (const std::pair<std::string, std::string>& interface : ifs) {
      if (ipif == interface.first) {
        isif = true;
        break;
      }
    }
    if (isif) {
      iom->setBindInterface(ipif);
    }
    else {
      iom->setBindAddress(ipif);
    }
  }
  global->linkComponents(wm, iom, tp);

  new Bnc(listenport, host, port, ident);
  wm->init("cbbncd");
  iom->init("cbbncd");
  tp->tickerLoop();
}
