// Recovered for Harvest from the Mac and Linux 1.18 builds; not the original source.
// Partial: Lunar, the Lua binding template from the lua-users wiki (Mac symbols name its push,
// Register, thunk, new_T, gc_T, tostring_T, lunarCheck, weaktable, subtable and pushuserdata);
// only the method table type is declared so far.

#ifndef OX_LUA_LUNAR_H
#define OX_LUA_LUNAR_H

#include "lua.hpp"

//! Binds the C++ class T to a Lua class: T names its methods in `methods` and itself in `className`.
template <typename T>
class Lunar
{
public:
    typedef int (T::*mfp)(lua_State* L);

    typedef struct
    {
        const char* name;
        mfp mfunc;
    } RegType;
};

#endif
