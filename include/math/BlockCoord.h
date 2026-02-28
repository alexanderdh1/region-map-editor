#pragma once

#include <cstdint>

struct BlockCoord
{
    int32_t x;
    int32_t z;

    bool operator==(const BlockCoord& other) const
    {
        return x == other.x && z == other.z;
    }

    bool operator!=(const BlockCoord& other) const
    {
        return !(*this == other);
    }
};