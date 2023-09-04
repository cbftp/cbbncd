#pragma once

#include <string>

class IOManager;
class WorkManager;
class TickPoke;

class GlobalContext {
  private:
    IOManager * iom;
    WorkManager * wm;
    TickPoke * tp;
    bool verbose;
  public:
    GlobalContext();
    void linkComponents(WorkManager *, IOManager *, TickPoke *);
    WorkManager * getWorkManager();
    IOManager * getIOManager();
    TickPoke * getTickPoke();
    void setVerbose(bool);
    void log(const std::string &);
};

extern GlobalContext * global;
