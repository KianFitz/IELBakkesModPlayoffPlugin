#include "IELBakkesModPlayoffPlugin.h"
#include "bakkesmod/wrappers/includes.h"
#include "bakkesmod/wrappers/GameObject/Stats/StatEventWrapper.h"

BAKKESMOD_PLUGIN(IELBakkesModPlayoffPlugin, "IEL Playoff Plugin", "1.0", PLUGINTYPE_SPECTATOR)

void IELBakkesModPlayoffPlugin::onLoad()
{
    cvarManager->registerCvar("iel_enabled", "1", "Enables the plugin to allow the upload to the server.").bindTo(m_enabled);
    cvarManager->registerCvar("iel_matchcode", "0", "The code for the plugin to send to the server to display the correct game on stream.").bindTo(m_gameCode);

    HookEvents();
}

void IELBakkesModPlayoffPlugin::onUnload()
{ }

void IELBakkesModPlayoffPlugin::HookEvents()
{
    gameWrapper->HookEventPost("Function TAGame.GameEvent_Soccar_TA.EventMatchEnded", std::bind(&IELBakkesModPlayoffPlugin::MatchEndedHook, this));

    gameWrapper->HookEventWithCaller<ServerWrapper>("Function TAGame.PRI_TA.ClientNotifyStatTickerMessage", std::bind(&IELBakkesModPlayoffPlugin::OnStatEvent, 
        this, std::placeholders::_1, std::placeholders::_2));
}

void IELBakkesModPlayoffPlugin::OnStatEvent(ServerWrapper caller, void* args)
{
    auto convertedArgs = (DummyStatEventContainer*)args;
    auto statEvent = StatEventWrapper(convertedArgs->StatEvent);
    auto label = statEvent.GetLabel();
    auto eventStr = label.ToString();

    auto receiver = PriWrapper(convertedArgs->Receiver);
    std::string receiverName, receiverID;
    GetNameAndID(receiver, receiverName, receiverID);

    if (eventStr == "Goal")
    {
        //auto teams = caller.GetTeams();
        //if (teams.IsNull()) {
        //    return;
        //}
        //int t1Score = teams.Get(0).IsNull() ? -1 : teams.Get(0).GetScore();
        //int t2Score = teams.Get(1).IsNull() ? -1 : teams.Get(1).GetScore();

        int teamIdx = receiver.GetTeamNum();
        
        int timer = caller.GetGameTime();

        //if (timer <= 0.0f)
        //    timer = caller.GetOvertimeTimePlayed();

        OnGoalScored(receiverName, teamIdx, timer);
    }
}

void IELBakkesModPlayoffPlugin::GetNameAndID(PriWrapper pri, std::string& name, std::string& id)
{
    if (pri.IsNull())
    {
        name = "";
        id = "";
    }
    else
    {
        name = pri.GetPlayerName().IsNull() ? "" : pri.GetPlayerName().ToString();
        id = name + "_" + std::to_string(pri.GetSpectatorShortcut());
    }
}

void IELBakkesModPlayoffPlugin::OnGoalScored(std::string& name, int& scorerIdx , int& timer)
{
    try
    {
        std::string s;
        s += "http://134.209.176.148/goal?";
        s += "scorer=" + name + "&";
        s += "scorerIdx=" + std::to_string(scorerIdx) + "&";
        s += "time=" + std::to_string(timer) + "&";
        s += "matchCode=" + *m_gameCode;

        http::Request goalRequest(s);
        const http::Response response = goalRequest.send("GET");
        cvarManager->log(std::string(response.body.begin(), response.body.end()));
    }
    catch(std::exception&)
    {
    }
}

void IELBakkesModPlayoffPlugin::SendRequest(http::Request& req)
{

}

void IELBakkesModPlayoffPlugin::MatchEndedHook()
{
    ServerWrapper server = gameWrapper->GetCurrentGameState();

    if (!server.IsNull())
    {
        cvarManager->log("Server is not null");
        auto winner = server.GetWinningTeam();
        std::string str = std::to_string(winner.GetTeamNum());
        OnMatchEnded(str);
    }
}

void IELBakkesModPlayoffPlugin::OnMatchEnded(std::string& teamIdx)
{
    std::string s;
    s += "http://134.209.176.148/matchEnded?";
    s += "winner=" + teamIdx + "&";
    s += "matchCode=" + *m_gameCode;

    http::Request gameEndedRequest(s);

    const http::Response response = gameEndedRequest.send("GET");
    cvarManager->log(std::string(response.body.begin(), response.body.end()));
}
