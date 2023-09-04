#include "globalcontext.h"

#include <iostream>
#include <ctime>

GlobalContext* global = new GlobalContext();

GlobalContext::GlobalContext() : verbose(false) {
}

std::string ctimeLog() {
  time_t rawtime = time(nullptr);
  char timebuf[26];
  ctime_r(&rawtime, timebuf);
  return std::string(timebuf + 11, 8);
}

void GlobalContext::linkComponents(Core::WorkManager* wm, Core::IOManager* iom, Core::TickPoke* tp) {
  this->wm = wm;
  this->iom = iom;
  this->tp = tp;
}

Core::IOManager* GlobalContext::getIOManager() {
  return iom;
}

Core::WorkManager* GlobalContext::getWorkManager() {
  return wm;
}

Core::TickPoke* GlobalContext::getTickPoke() {
  return tp;
}

void GlobalContext::setVerbose(bool verbose) {
  this->verbose = verbose;
}

void GlobalContext::log(const std::string& text) {
  if (verbose) {
    std::cout << "[" << ctimeLog() << "] " << text << std::endl;
  }
}
