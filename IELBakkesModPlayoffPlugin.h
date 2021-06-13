#pragma once
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "pluginsdk.lib")
#pragma comment(lib, "bakkesmod.lib")

#include <thread>
#include "HTTP.h"
#include "bakkesmod/plugin/bakkesmodplugin.h"

class IELBakkesModPlayoffPlugin : public BakkesMod::Plugin::BakkesModPlugin
{
public: 
    void onLoad() override;
    void onUnload() override;
private:
    std::shared_ptr<std::string> m_gameCode = std::make_shared<std::string>("");
    std::shared_ptr<bool> m_enabled = std::make_shared<bool>(0);

    void HookEvents();
    void OnStatEvent(ServerWrapper caller, void* args);
    void GetNameAndID(PriWrapper pri, std::string& name, std::string& id);
    void MatchEndedHook();

    void OnGoalScored(std::string& scorer, int& scorerIdx, int& time);
    void OnMatchEnded(std::string& teamIdx);

    void SendRequest(http::Request& req);
};

struct DummyStatEventContainer
{
    uintptr_t Receiver;
    uintptr_t Victim;
    uintptr_t StatEvent;
};
