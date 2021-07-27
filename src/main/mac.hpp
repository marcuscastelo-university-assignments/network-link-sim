#pragma once

#include "types.hpp"
#include <stdint.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

struct MAC
{
    uint64_t bytes;

    std::string to_string() const
    {
        auto parts = bytesToParts(bytes);
        std::stringstream ss;
        ss << std::hex << std::setfill('0')
           << std::setw(2) << (uint64_t)parts[0] << ":"
           << std::setw(2) << (uint64_t)parts[1] << ":"
           << std::setw(2) << (uint64_t)parts[2] << ":"
           << std::setw(2) << (uint64_t)parts[3] << ":"
           << std::setw(2) << (uint64_t)parts[4] << ":"
           << std::setw(2) << (uint64_t)parts[5];
        return ss.str();
    }

    MAC(const std::string &str)
    {
        std::string m(str);
        std::replace(m.begin(), m.end(), ':', ' ');
        std::stringstream ss(m);
        MAC_PARTS parts;
        ss >> std::hex >> parts[0] >> parts[1] >> parts[2] >> parts[3] >> parts[4] >> parts[5];
        this->bytes = partsToBytes(parts);
    }

    MAC(const MAC_PARTS &parts) : MAC(partsToBytes(parts))
    {
    }

    MAC(uint64_t bytes)
        : bytes(bytes)
    {
    }

    MAC(const MAC &other) : MAC(other.bytes)
    {
    }

    bool operator==(const MAC &other) const { return bytes == other.bytes; }

    static uint64_t partsToBytes(const MAC_PARTS &parts)
    {
        uint64_t bytes = 0;
        for (int i = 0; i < 6; i++)
        {
            bytes = bytes << 8;
            bytes |= parts[i];
        }
        return bytes;
    }

    static Ref<MAC_PARTS> bytesToParts(uint64_t bytes)
    {
        Ref<MAC_PARTS> parts = std::make_shared<MAC_PARTS>(6);
        for (int i = 5; i >= 0; i--)
        {
            parts[i] = bytes & 0xFF;
            bytes >>= 8;
        }
        return parts;
    }
};

template <>
struct std::hash<MAC>
{
    size_t operator()(const MAC &mac) const
    {
        return std::hash<uint64_t>()(mac.bytes);
    }
};
