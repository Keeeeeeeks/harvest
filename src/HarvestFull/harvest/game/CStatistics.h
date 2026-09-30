// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
#ifndef HARVEST_GAME_CSTATISTICS_H
#define HARVEST_GAME_CSTATISTICS_H

#include "ox/TArray.h"
#include "ox/core/CHiddenInt.h"
#include "ox/core/CString.h"

namespace harvest {
namespace game {

//! A wave's duration, level statistics and four statistics for each alien type.
struct SLevelStats
{
    SLevelStats() : Time(0), Level(0)
    {
        for (int i = 0; i < 7; ++i) Stats[i] = 0;
        for (unsigned int i = 0; i < 4; ++i)
            for (int j = 0; j < 14; ++j) AlienStats[i][j] = 0;
    }
    float Time;
    int Level;
    float Stats[7];
    float AlienStats[4][14];
};

struct SEventLog
{
    float Time;
    int Type;
    int Value;
};

//! Per-wave statistics and protected game totals, used by the score screens.
class CStatistics
{
public:
    CStatistics();
    virtual ~CStatistics();
    bool read(ox::io::IReadFile* file, int version);
    bool write(ox::io::IWriteFile* file);
    void reportNewThreatLevel(int level, float time);
    void modifyLevelStatValue(int stat, float delta);
    void reportAlienStatChange(int statistic, int alienType, float amount);
    float getRushModeDamage();
    void setHighest(int stat, float value);
    void modifyGameStatValue(int stat, int delta);
    const ox::TArray<SLevelStats*>& getAllLevelStats();
    void addLog(float time, int type, int value);
    const ox::TArray<SEventLog>& getLogs();
    int getGameStatValue(int stat);
private:
    ox::TArray<SLevelStats*> Levels;
    SLevelStats* Current;
    ox::core::CHiddenInt GameStats[7];
    float RushModeDamage;
    ox::TArray<SEventLog> Logs;
};

class CHighscoreInfo
{
public:
    CHighscoreInfo();
    CHighscoreInfo(const wchar_t* name, const wchar_t* group, int random, int startTime,
        int mode, int planet, int minerals, int level, float playTime);
    virtual ~CHighscoreInfo();
    void createHighscoreString();
    const wchar_t* getPlayerName() const;
    const wchar_t* getPlayerGroup() const;
    int getRandomValue() const;
    int getStartPlayTime() const;
    int getGameMode() const;
    int getGamePlanet() const;
    int getTotalMinerals();
    int getHighestLevel();
    float getPlayTime() const;
    const char* getHighscoreString() const;
    static void setNewHighscoreInfo(CHighscoreInfo* info);
    static CHighscoreInfo* getCurrentHighscoreInfo();
private:
    ox::core::CString<wchar_t> PlayerName;
    ox::core::CString<wchar_t> PlayerGroup;
    int RandomValue;
    int StartPlayTime;
    int GameMode;
    int GamePlanet;
    ox::core::CHiddenInt TotalMinerals;
    ox::core::CHiddenInt HighestLevel;
    float PlayTime;
    ox::core::CString<char> HighscoreString;
    static CHighscoreInfo* CurrentInfo;
};

extern CStatistics* gp_statistics;

} // end namespace game
} // end namespace harvest
#endif
