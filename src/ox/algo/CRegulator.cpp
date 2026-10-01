// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.

#include "CRegulator.h"
// The native object includes the same iostream initializer as the adjacent ox units.
#include <iostream> // IWYU pragma: keep

namespace ox {
namespace algo {

CRegulator::CRegulator()
    : Target(0), Current(0), Integral(0), PreviousError(0), Elapsed(0), P(1), I(0), D(0),
      AntiWindup(false), IntegralLimit(9999), IntegralStart(0)
{
}

CRegulator::CRegulator(float current, float target, float p, float i, float d,
                     bool antiWindup, float integralStart, float integralLimit)
    : Target(target), Current(current), Integral(0), Elapsed(0),
      P(p), I(i), D(d), AntiWindup(antiWindup), IntegralLimit(integralLimit), IntegralStart(integralStart)
{
    PreviousError = Target - Current;
}

CRegulator::~CRegulator()
{
}

void CRegulator::restart(float current)
{
    Current = current;
    Integral = AntiWindup ? IntegralStart : 0;
    PreviousError = Target - Current;
    Elapsed = 0;
}

float CRegulator::update(float time)
{
    float error = Target - Current;
    float p = P;
    Integral += error;
    float integral = Integral;
    if (AntiWindup)
    {
        if (Integral > IntegralLimit)
            integral = IntegralLimit;
        else
            integral = Integral < -IntegralLimit ? -IntegralLimit : Integral;
        Integral = integral;
    }

    Elapsed += time;
    float i = I;
    float derivative = 0;
    if (Elapsed > 0.01)
    {
        derivative = (PreviousError - error) / Elapsed * D * time;
        PreviousError = error;
        Elapsed = 0;
    }

    Current += p * error * time + integral * i * time + derivative;
    return Current;
}

float CRegulator::getCurrent()
{
    return Current;
}

void CRegulator::setCurrentValue(float current)
{
    Current = current;
}

void CRegulator::setTargetValue(float target)
{
    Target = target;
}

void CRegulator::setP(float p)
{
    P = p;
}

void CRegulator::setI(float i)
{
    I = i;
}

void CRegulator::setD(float d)
{
    D = d;
}

void CRegulator::setAntiWindup(bool enabled, float start, float limit)
{
    AntiWindup = enabled;
    IntegralStart = start;
    IntegralLimit = limit;
}

C3dRegulator::C3dRegulator()
{
}

C3dRegulator::C3dRegulator(const core::CVector3d<float>& current, const core::CVector3d<float>& target,
                         float p, float i, float d, bool antiWindup, float integralStart, float integralLimit)
    : X(current.X, target.X, p, i, d, antiWindup, integralStart, integralLimit),
      Y(current.Y, target.Y, p, i, d, antiWindup, integralStart, integralLimit),
      Z(current.Z, target.Z, p, i, d, antiWindup, integralStart, integralLimit)
{
}

C3dRegulator::~C3dRegulator()
{
}

void C3dRegulator::restart(const core::CVector3d<float>& current)
{
    X.restart(current.X);
    Y.restart(current.Y);
    Z.restart(current.Z);
}

core::CVector3d<float> C3dRegulator::update(float time)
{
    float x = X.update(time);
    float y = Y.update(time);
    float z = Z.update(time);
    return core::CVector3d<float>(x, y, z);
}

core::CVector3d<float> C3dRegulator::getCurrent()
{
    return core::CVector3d<float>(X.getCurrent(), Y.getCurrent(), Z.getCurrent());
}

void C3dRegulator::setCurrentValue(const core::CVector3d<float>& current)
{
    X.setCurrentValue(current.X);
    Y.setCurrentValue(current.Y);
    Z.setCurrentValue(current.Z);
}

void C3dRegulator::setTargetValue(const core::CVector3d<float>& target)
{
    X.setTargetValue(target.X);
    Y.setTargetValue(target.Y);
    Z.setTargetValue(target.Z);
}

void C3dRegulator::setP(float p)
{
    X.setP(p);
    Y.setP(p);
    Z.setP(p);
}

void C3dRegulator::setI(float i)
{
    X.setI(i);
    Y.setI(i);
    Z.setI(i);
}

void C3dRegulator::setD(float d)
{
    X.setD(d);
    Y.setD(d);
    Z.setD(d);
}

void C3dRegulator::setAntiWindup(bool enabled, float start, float limit)
{
    X.setAntiWindup(enabled, start, limit);
    Y.setAntiWindup(enabled, start, limit);
    Z.setAntiWindup(enabled, start, limit);
}

void C3dRegulator::fakeSpeedRegulation(core::CVector3d<float>& speed, core::CVector3d<float>& position,
                                     const core::CVector3d<float>& target,
                                     float unused, float factor, float time)
{
    core::CVector3d<float> desired = (target - position) * factor;
    if (desired.X != 0.0f || desired.Y != 0.0f || desired.Z != 0.0f)
    {
        core::CVector3d<float> direction = desired;
        float length = desired.getLength();
        direction.X /= length;
        direction.Y /= length;
        direction.Z /= length;
        speed += direction * time * 20.0f;

        if ((speed.X > desired.X && desired.X > 0) || (speed.X < desired.X && desired.X < 0))
            speed.X = desired.X;
        if ((speed.Y > desired.Y && desired.Y > 0) || (speed.Y < desired.Y && desired.Y < 0))
            speed.Y = desired.Y;
        if ((speed.Z > desired.Z && desired.Z > 0) || (speed.Z < desired.Z && desired.Z < 0))
            speed.Z = desired.Z;

        position += speed * time;
    }
}

} // end namespace algo
} // end namespace ox
