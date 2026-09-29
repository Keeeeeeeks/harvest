// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: the names are ours, and only the values recovered units send are listed.

#ifndef HARVEST_ECUSTOMEVENTS_H
#define HARVEST_ECUSTOMEVENTS_H

namespace harvest {

//! Game events, sent as ox::event::EET_USER_EVENT with the value in UserData1.
enum ECUSTOM_EVENT
{
    //! The first attack of a threat level has been spawned.
    ECE_ATTACK_STARTED = 7
};

} // end namespace harvest

#endif
