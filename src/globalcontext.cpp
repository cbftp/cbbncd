#include "globalcontext.h"

#include <iostream>
#include <ctime>

GlobalContext * global = new GlobalContext();

GlobalContext::GlobalContext() : verbose(false) {
}

std::string ctimeLog() {
  time_t rawtime = time(NULL);
  char timebuf[26];
  ctime_r(&rawtime, timebuf);
  return std::string(timebuf + 11, 8);
}

void GlobalContext::linkComponents(WorkManager * wm, IOManager * iom, TickPoke * tp) {
  this->wm = wm;
  this->iom = iom;
  this->tp = tp;
}

IOManager * GlobalContext::getIOManager() {
  return iom;
}

WorkManager * GlobalContext::getWorkManager() {
  return wm;
}

TickPoke * GlobalContext::getTickPoke() {
  return tp;
}

void GlobalContext::setVerbose(bool verbose) {
  this->verbose = verbose;
}

void GlobalContext::log(const std::string & text) {
  if (verbose) {
    std::cout << "[" << ctimeLog() << "] " << text << std::endl;
  }
}
