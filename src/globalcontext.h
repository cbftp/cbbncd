#pragma once

#include <string>

namespace Core {
class IOManager;
class WorkManager;
class TickPoke;
}

class GlobalContext {
  private:
    Core::IOManager* iom;
    Core::WorkManager* wm;
    Core::TickPoke* tp;
    bool verbose;
  public:
    GlobalContext();
    void linkComponents(Core::WorkManager* wm, Core::IOManager* iom, Core::TickPoke* tp);
    Core::WorkManager* getWorkManager();
    Core::IOManager* getIOManager();
    Core::TickPoke* getTickPoke();
    void setVerbose(bool);
    void log(const std::string& text);
};

extern GlobalContext* global;
