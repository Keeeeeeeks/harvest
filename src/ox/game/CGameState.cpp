// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#include "CGameState.h"
// The native unit has an iostream static initializer, as do the other ox units.
#include <iostream> // IWYU pragma: keep

namespace ox {
namespace game {

CGameState::CGameState()
    : ErrorMessage(""), Device(0), Driver(0), GUIEnvironment(0), SceneManager(0)
{
}

CGameState::~CGameState()
{
}

int CGameState::daisyInit(IOxDevice* device)
{
    Device = device;
    if (!Device)
    {
        ErrorMessage = "No Daisy device available!";
        return 1;
    }

    Driver = Device->getVideoDriver();
    GUIEnvironment = Device->getGUIEnvironment();
    SceneManager = Device->getSceneManager();
    AudioDriver = Device->getAudioDriver();
    JoystickDriver = Device->getJoystickDriver();

    if (!Driver || !GUIEnvironment || !SceneManager)
    {
        ErrorMessage = "Unable to load Daisy devices!";
        return 1;
    }

    return 0;
}

int CGameState::firstInit(IOxDevice* device)
{
    return daisyInit(device) != 0;
}

int CGameState::returnFailedInit(char* message)
{
    ErrorMessage = message;
    return 1;
}

const char* CGameState::getErrorMessage()
{
    return ErrorMessage;
}

} // end namespace game
} // end namespace ox
