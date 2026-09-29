// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#ifndef OX_ALGO_CTIMECOUNTER_H
#define OX_ALGO_CTIMECOUNTER_H

namespace ox {
namespace algo {

//! Counts a delay down to zero and reports how far along it is.
class CTimeCounter
{
public:
    CTimeCounter();
    virtual ~CTimeCounter();

    //! Adds to the remaining time and sets the delay the progress is measured against.
    void putDelay(float time, float delay);
    //! Restarts the count with a new delay.
    void setDelay(float delay);
    //! Sets the delay the progress is measured against without touching the remaining time.
    void overrideStartingValue(float delay);

    //! Counts down; true once the delay has run out.
    bool updateCounter(float frameDelta);

    bool isStarted();
    bool isDelayed();
    float getRemainingTime();
    //! 0 when started, 1 when the delay has run out.
    float getProgress();
    //! 1 when started, 0 when the delay has run out.
    float getInvertedProgress();

private:
    float Remaining;
    float Delay;
};

} // end namespace algo
} // end namespace ox

#endif
