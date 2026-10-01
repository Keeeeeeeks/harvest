#include "ox/algo/CRegulator.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>

using ox::algo::CRegulator;
using ox::algo::C3dRegulator;
typedef ox::core::CVector3d<float> Vector;

static void require(bool condition, const char* expression)
{
    if (!condition)
    {
        std::fprintf(stderr, "Regulator check failed: %s\n", expression);
        std::exit(EXIT_FAILURE);
    }
}

#define CHECK(expression) require((expression), #expression)

static bool close(float actual, float expected)
{
    return std::fabs(actual - expected) < 0.00001f;
}

static void same(const Vector& actual, const Vector& expected)
{
    CHECK(close(actual.X, expected.X));
    CHECK(close(actual.Y, expected.Y));
    CHECK(close(actual.Z, expected.Z));
}

static void scalar_control()
{
    CRegulator proportional;
    CHECK(proportional.getCurrent() == 0);
    proportional.setTargetValue(10);
    CHECK(proportional.update(0.25f) == 2.5f);
    proportional.setP(2);
    CHECK(proportional.update(0.125f) == 4.375f);

    CRegulator integral(0, 2, 0, 1, 0, false, 0, 9999);
    CHECK(integral.update(0.25f) == 0.5f);
    CHECK(integral.update(0.5f) == 2.25f);

    CRegulator derivative(0, 1, 0, 0, 1, false, 0, 9999);
    derivative.setTargetValue(2);
    CHECK(derivative.update(0.01f) == 0);
    CHECK(close(derivative.update(0.01f), -0.5f));
    derivative.restart(3);
    CHECK(derivative.update(0.02f) == 3);

    CRegulator combined(0, 8, 1, 0, 1, false, 0, 9999);
    CHECK(combined.update(0.0078125f) == 0.0625f);
    CHECK(combined.update(0.0078125f) == 0.15576171875f);

    CRegulator clamp(0, 10, 0, 1, 0, true, 2, 3);
    CHECK(clamp.update(0.25f) == 0.75f);
    clamp.setTargetValue(0);
    clamp.restart(0);
    CHECK(clamp.update(0.25f) == 0.5f);
    clamp.setTargetValue(-10);
    clamp.restart(0);
    CHECK(clamp.update(0.25f) == -0.75f);
    clamp.setAntiWindup(false, 7, 2);
    clamp.restart(0);
    CHECK(clamp.update(0.25f) == -2.5f);

    CRegulator setters;
    setters.setCurrentValue(3);
    setters.setTargetValue(5);
    setters.setP(0);
    setters.setI(1);
    setters.setD(0);
    CHECK(setters.update(0) == 3);
    CHECK(setters.update(0.5f) == 5);

    CRegulator backwards(0, 4, 1, 0, 0, false, 0, 9999);
    CHECK(backwards.update(-0.25f) == -1);
}

static void vector_control()
{
    C3dRegulator defaults;
    same(defaults.getCurrent(), Vector(0, 0, 0));
    defaults.setTargetValue(Vector(4, -8, 12));
    same(defaults.update(0.25f), Vector(1, -2, 3));

    Vector current(1, -2, 3), target(5, 6, -1);
    C3dRegulator vector(current, target, 2, 0.5f, 0.25f, true, 1, 4);
    CRegulator x(current.X, target.X, 2, 0.5f, 0.25f, true, 1, 4);
    CRegulator y(current.Y, target.Y, 2, 0.5f, 0.25f, true, 1, 4);
    CRegulator z(current.Z, target.Z, 2, 0.5f, 0.25f, true, 1, 4);
    for (int step = 0; step < 64; ++step)
    {
        float time = step % 2 ? 0.0078125f : 0.03125f;
        float expectedX = x.update(time);
        float expectedY = y.update(time);
        float expectedZ = z.update(time);
        same(vector.update(time), Vector(expectedX, expectedY, expectedZ));
    }

    vector.setCurrentValue(Vector(1, 2, 3));
    vector.setTargetValue(Vector(3, 4, 5));
    vector.setP(0);
    vector.setI(1);
    vector.setD(0);
    vector.setAntiWindup(true, 1, 2);
    vector.restart(Vector(1, 2, 3));
    same(vector.update(0.25f), Vector(1.5f, 2.5f, 3.5f));
    same(vector.getCurrent(), Vector(1.5f, 2.5f, 3.5f));
}

static void speed_control()
{
    Vector speed(0, 0, 0), position(0, 0, 0);
    C3dRegulator::fakeSpeedRegulation(speed, position, Vector(10, 0, 0), 1, 1, 0.25f);
    same(speed, Vector(5, 0, 0));
    same(position, Vector(1.25f, 0, 0));

    for (int sign = -1; sign <= 1; sign += 2)
    {
        speed = Vector(0, 0, 0);
        position = Vector(0, 0, 0);
        C3dRegulator::fakeSpeedRegulation(speed, position, Vector(sign, 0, 0), 999, 1, 0.25f);
        same(speed, Vector(sign, 0, 0));
        same(position, Vector(sign * 0.25f, 0, 0));
    }

    speed = Vector(0, 0, 0);
    position = Vector(0, 0, 0);
    C3dRegulator::fakeSpeedRegulation(speed, position, Vector(3, 4, 0), 0, 1, 0.1f);
    same(speed, Vector(1.2f, 1.6f, 0));
    same(position, Vector(0.12f, 0.16f, 0));

    speed = Vector(1, 2, 3);
    position = Vector(4, 5, 6);
    C3dRegulator::fakeSpeedRegulation(speed, position, position, 10, 1, 0.25f);
    same(speed, Vector(1, 2, 3));
    same(position, Vector(4, 5, 6));
    C3dRegulator::fakeSpeedRegulation(speed, position, Vector(10, 20, 30), 10, 0, 0.25f);
    same(speed, Vector(1, 2, 3));
    same(position, Vector(4, 5, 6));

    speed = Vector(2, 9, 0);
    position = Vector(0, 0, 0);
    C3dRegulator::fakeSpeedRegulation(speed, position, Vector(1, 0, 0), 0, 1, 0);
    same(speed, Vector(1, 9, 0));
    same(position, Vector(0, 0, 0));
    C3dRegulator::fakeSpeedRegulation(speed, position, Vector(10, 0, 0), 0, 1, 0.25f);
    same(speed, Vector(6, 9, 0));
    same(position, Vector(1.5f, 2.25f, 0));

    Vector a(0, 0, 0), b(0, 0, 0), pa(0, 0, 0), pb(0, 0, 0);
    C3dRegulator::fakeSpeedRegulation(a, pa, Vector(3, -4, 5), -1000, 0.5f, 0.125f);
    C3dRegulator::fakeSpeedRegulation(b, pb, Vector(3, -4, 5), 1000, 0.5f, 0.125f);
    same(a, b);
    same(pa, pb);
}

int main()
{
    CHECK(sizeof(CRegulator) == 56);
    CHECK(sizeof(C3dRegulator) == 176);
    scalar_control();
    vector_control();
    speed_control();
    CRegulator* scalar = new CRegulator;
    C3dRegulator* vector = new C3dRegulator;
    delete scalar;
    delete vector;
    std::puts("Regulator smoke passed: scalar PID, anti-windup, derivative timing, three-axis forwarding, speed regulation and destruction");
}
