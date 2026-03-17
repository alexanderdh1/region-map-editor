#pragma once

#include <string>

enum class RegionStatus
{
    None,
    InProgress,
    Done
};

inline std::string regionStatusToString(RegionStatus status)
{
    switch (status)
    {
        case RegionStatus::None:       return "None";
        case RegionStatus::InProgress: return "In Progress";
        case RegionStatus::Done:       return "Done";
    }
    return "Unknown";
}

inline RegionStatus regionStatusFromString(const std::string& s)
{
    if (s == "InProgress") return RegionStatus::InProgress;
    if (s == "Done")        return RegionStatus::Done;
    return RegionStatus::None;
}
