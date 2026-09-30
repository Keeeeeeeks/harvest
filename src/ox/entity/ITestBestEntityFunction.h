// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// The file name is inferred.

#ifndef OX_ENTITY_ITESTBESTENTITYFUNCTION_H
#define OX_ENTITY_ITESTBESTENTITYFUNCTION_H

namespace ox {
namespace entity {
class COxEntity;

//! Accepts a candidate when it is preferable to the current best entity.
//! Like ITestEntityFunction, this interface has no virtual destructor.
class ITestBestEntityFunction
{
public:
    //! Returns 0 to reject, 1 to remember the candidate, or 2 to stop immediately.
    virtual int testEntity(COxEntity* entity, COxEntity* best) = 0;
};

} // end namespace entity
} // end namespace ox

#endif
