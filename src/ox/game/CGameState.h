// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Member names, access and string constness are inferred; virtual order follows the Mac state vtables.

#ifndef OX_GAME_CGAMESTATE_H
#define OX_GAME_CGAMESTATE_H

#include "ox/IOxDevice.h"
#include "ox/event/IEventReceiver.h"

namespace ox {
namespace game {

class CGameState : public event::IEventReceiver
{
public:
    CGameState();
    virtual ~CGameState();

    virtual int firstInit(IOxDevice* device);
    virtual void renderFirst() = 0;
    virtual int secondInit() = 0;
    virtual int updateState(float time) = 0;
    virtual void render() = 0;

    const char* getErrorMessage();

protected:
    int daisyInit(IOxDevice* device);
    int returnFailedInit(char* message);

    const char* ErrorMessage;
    IOxDevice* Device;
    video::IVideoDriver* Driver;
    gui::IGUIEnvironment* GUIEnvironment;
    scene::ISceneManager* SceneManager;
    audio::IAudioDriver* AudioDriver;
    input::IJoystickDriver* JoystickDriver;
};

} // end namespace game
} // end namespace ox

#endif
