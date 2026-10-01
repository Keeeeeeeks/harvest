// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Member names are inferred from the native layouts and updates.

#ifndef OX_ALGO_CREGULATOR_H
#define OX_ALGO_CREGULATOR_H

#include "ox/core/CVector3d.h"

namespace ox {
namespace algo {

class CRegulator
{
public:
    CRegulator();
    CRegulator(float current, float target, float p, float i, float d,
               bool antiWindup, float integralStart, float integralLimit);
    virtual ~CRegulator();

    void restart(float current);
    float update(float time);
    float getCurrent();
    void setCurrentValue(float current);
    void setTargetValue(float target);
    void setP(float p);
    void setI(float i);
    void setD(float d);
    void setAntiWindup(bool enabled, float start, float limit);

private:
    float Target;
    float Current;
    float Integral;
    float PreviousError;
    float Elapsed;
    float P;
    float I;
    float D;
    bool AntiWindup;
    float IntegralLimit;
    float IntegralStart;
};

class C3dRegulator
{
public:
    C3dRegulator();
    C3dRegulator(const core::CVector3d<float>& current, const core::CVector3d<float>& target,
                 float p, float i, float d, bool antiWindup, float integralStart, float integralLimit);
    virtual ~C3dRegulator();

    void restart(const core::CVector3d<float>& current);
    core::CVector3d<float> update(float time);
    core::CVector3d<float> getCurrent();
    void setCurrentValue(const core::CVector3d<float>& current);
    void setTargetValue(const core::CVector3d<float>& target);
    void setP(float p);
    void setI(float i);
    void setD(float d);
    void setAntiWindup(bool enabled, float start, float limit);
    static void fakeSpeedRegulation(core::CVector3d<float>& speed, core::CVector3d<float>& position,
                                    const core::CVector3d<float>& target,
                                    float unused, float factor, float time);

private:
    CRegulator X;
    CRegulator Y;
    CRegulator Z;
};

} // end namespace algo
} // end namespace ox

#endif
