#pragma once

#include <cstdint>

struct MapCoord
{
    int32_t x;
    int32_t y;

    bool operator==(const MapCoord& other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator!=(const MapCoord& other) const
    {
        return !(*this == other);
    }
};
