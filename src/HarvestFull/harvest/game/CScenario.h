// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Layout member names are ours. Event names and methods follow the Mac symbols.
#ifndef HARVEST_GAME_CSCENARIO_H
#define HARVEST_GAME_CSCENARIO_H

#include "harvest/gui/CStoryScreen.h"
#include "ox/core/CString.h"
#include "ox/TArray.h"
#include <list>
#include "SInfoLineMessage.h"
#include "ox/event/IEventReceiver.h"
#include "harvest/entity/CEntityManager.h"
#include "harvest/entity/CDropshipEntity.h"

namespace harvest {
namespace entity { class CDropshipEntity; }
namespace game {

class CScenario;

//! An event waits Delay seconds after the previous timed event.
class CHarvestEvent
{
public:
    CHarvestEvent(float delay, int type) : Delay(delay), Type(type) {}
    virtual ~CHarvestEvent() {}
    virtual void runEvent() = 0;
    float Delay;
    int Type;
};

class CPostEventEvent : public CHarvestEvent
{
public:
    CPostEventEvent(float delay, ox::event::IEventReceiver* receiver, int data1, int data2, int data3)
        : CHarvestEvent(delay, 1), Receiver(receiver), Data1(data1), Data2(data2), Data3(data3) {}
    virtual void runEvent()
    {
        if (!Receiver) return;
        ox::event::SEvent event;
        event.EventType = ox::event::EET_USER_EVENT;
        event.UserEvent.UserData1 = Data1;
        event.UserEvent.UserData2 = Data2;
        event.UserEvent.UserData3 = Data3;
        Receiver->OnEvent(event);
    }
    ox::event::IEventReceiver* Receiver;
    int Data1, Data2, Data3;
};

class CTrackEntityEvent : public CHarvestEvent
{
public:
    CTrackEntityEvent(float delay, ox::event::IEventReceiver* receiver, int id, int layer)
        : CHarvestEvent(delay, 5), Receiver(receiver), Id(id), Layer(layer) {}
    virtual void runEvent()
    {
        if (!Receiver) return;
        ox::event::SEvent event;
        event.EventType = ox::event::EET_USER_EVENT;
        event.UserEvent.UserData1 = 28;
        event.UserEvent.UserData2 = Id;
        event.UserEvent.UserData3 = Layer;
        Receiver->OnEvent(event);
    }
    ox::event::IEventReceiver* Receiver;
    int Id, Layer;
};

class CDropshipStateEvent : public CHarvestEvent
{
public:
    CDropshipStateEvent(float delay, entity::CDropshipEntity* dropship, int state)
        : CHarvestEvent(delay, 6), Dropship(dropship), State(state) {}
    virtual void runEvent() { if (Dropship) Dropship->setDropshipState(State); }
    entity::CDropshipEntity* Dropship;
    int State;
};

class CSpawnEntityEvent : public CHarvestEvent
{
public:
    CSpawnEntityEvent(float delay, entity::CEntity* entity, int layer)
        : CHarvestEvent(delay, 7), Entity(entity), Layer(layer) {}
    virtual ~CSpawnEntityEvent() { delete Entity; }
    virtual void runEvent()
    {
        if (!Entity) return;
        entity::gp_entityManager->appendEntity(Entity, Layer);
        Entity = 0;
    }
    entity::CEntity* Entity;
    int Layer;
};

class CAddDialogueEvent : public CHarvestEvent
{
public:
    CAddDialogueEvent(float delay, ox::event::IEventReceiver* receiver, gui::CDialogueItemInfo* dialogue)
        : CHarvestEvent(delay, 3), Receiver(receiver), Dialogue(dialogue) {}
    virtual ~CAddDialogueEvent() { delete Dialogue; }
    virtual void runEvent()
    {
        if (!Receiver) return;
        ox::event::SEvent event;
        event.EventType = ox::event::EET_USER_EVENT;
        event.UserEvent.UserData1 = 31;
        event.UserEvent.UserPointer = Dialogue;
        Receiver->OnEvent(event);
    }
    ox::event::IEventReceiver* Receiver;
    gui::CDialogueItemInfo* Dialogue;
};

class CAddInfoLineEvent : public CHarvestEvent
{
public:
    CAddInfoLineEvent(float delay, ox::event::IEventReceiver* receiver, const wchar_t* name,
                      const wchar_t* text, const char* portrait, const char* sound)
        : CHarvestEvent(delay, 4), Receiver(receiver)
    {
        Message.Name = name;
        Message.Text = text;
        Message.Portrait = portrait;
        Message.Sound = sound;
    }
    virtual void runEvent()
    {
        if (!Receiver) return;
        ox::event::SEvent event;
        event.EventType = ox::event::EET_USER_EVENT;
        event.UserEvent.UserData1 = 32;
        event.UserEvent.UserPointer = &Message;
        Receiver->OnEvent(event);
    }
    ox::event::IEventReceiver* Receiver;
    SInfoLineMessage Message;
};

class CHarvestEventCondition
{
public:
    CHarvestEventCondition(int value, int comparison) : Value(value), Comparison(comparison) {}
    virtual ~CHarvestEventCondition() {}
    virtual bool testCondition() = 0;
    static bool comparisonTest(int value, int threshold, int comparison);
protected:
    int Value;
    int Comparison;
};

class CMineralsCondition : public CHarvestEventCondition
{
public:
    CMineralsCondition(int value, int comparison) : CHarvestEventCondition(0, 0)
    { Value = value; Comparison = comparison; }
    virtual bool testCondition();
};

class CAlienCountCondition : public CHarvestEventCondition
{
public:
    CAlienCountCondition(int value, int comparison) : CHarvestEventCondition(0, 0)
    { Value = value; Comparison = comparison; }
    virtual bool testCondition();
};

//! Transfers its events to the timed queue once every condition succeeds.
class CConditionalHarvestEvent : public CHarvestEvent
{
public:
    CConditionalHarvestEvent(CScenario* scenario) : CHarvestEvent(0, 2), Scenario(scenario) {}
    virtual ~CConditionalHarvestEvent()
    {
        for (unsigned int i = 0; i < Conditions.size(); ++i) delete Conditions[i];
    }
    virtual void runEvent();
    bool shouldRunEvent()
    {
        for (unsigned int i = 0; i < Conditions.size(); ++i)
            if (!Conditions[i]->testCondition()) return false;
        return true;
    }
    CScenario* Scenario;
    std::list<CHarvestEvent*> Events;
    ox::TArray<CHarvestEventCondition*> Conditions;
};

//! The scripted campaign: timed and conditional events, objectives and starting entities.
class CScenario : public IScenario
{
public:
    CScenario();
    virtual ~CScenario();
    void clearEvents();
    void addEvent(CHarvestEvent* event);
    void update(float frameDelta);
    void skipToNextEvent();
    int getScenarioPlanet() const;
    int getStartingCredits() const;
    virtual int getDoodadSeed() const;
    const ox::core::CString<wchar_t>& getObjective(int index);
    int getNumObjectives();
    virtual void applyInitialExpansions(CWorld* world);
    void addStartingEntities();
    gui::CDialogueItemInfo* getCharacterDialog(const wchar_t* key, int character, char* sound, float time);
    CAddInfoLineEvent* getCharacterInfoLine(float delay, ox::event::IEventReceiver* receiver,
                                          const wchar_t* key, int character, const char* sound);
    void createScenarioEvents(ox::event::IEventReceiver* receiver);
    void notifyDropshipKilledAllAliens(ox::event::IEventReceiver* receiver);
    void notifyDropshipLanded(ox::event::IEventReceiver* receiver);
    void notifyDropshipGoingToSpace(ox::event::IEventReceiver* receiver);
private:
    std::list<CHarvestEvent*> Events;
    std::list<CHarvestEvent*> ConditionalEvents;
    float Time;
    entity::CDropshipEntity* Dropship;
    int TrackedAlienId;
    int StartingCollectorId;
    int TrackedMineralId;
    ox::core::CString<wchar_t> Objectives[2];
    ox::core::CString<wchar_t> CharacterNames[3];
};

inline void CConditionalHarvestEvent::runEvent()
{
    for (std::list<CHarvestEvent*>::iterator it = Events.begin(); it != Events.end(); ++it)
        Scenario->addEvent(*it);
    Events.clear();
}

} // end namespace game
} // end namespace harvest
#endif
