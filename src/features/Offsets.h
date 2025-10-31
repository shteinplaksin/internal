#ifndef FEATURES_OFFSETS_H_
#define FEATURES_OFFSETS_H_

#include <cstdint>

namespace Offsets
{
    constexpr std::uintptr_t ViewMatrix = 0x17DFD0;
    constexpr std::uintptr_t LocalPlayer = 0x17E0A8;
    constexpr std::uintptr_t EntityList = 0x18AC04;
    constexpr std::uintptr_t MaxPlayers = 0x18AC0C;

    namespace Entity
    {
        constexpr std::uintptr_t PosX = 0x2C;
        constexpr std::uintptr_t PosY = 0x30;
        constexpr std::uintptr_t PosZ = 0x28;
        constexpr std::uintptr_t Health = 0xEC;
        constexpr std::uintptr_t Team = 0x32C;
        constexpr std::uintptr_t Name = 0x205;
        constexpr std::uintptr_t State = 0x338;
    }
}

#endif
