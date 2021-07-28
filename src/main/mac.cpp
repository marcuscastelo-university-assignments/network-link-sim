/**
 * Header criado para simular o endereço MAC de uma máquina
 */
#include "mac.hpp"

#include <stdint.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <memory>

#include "types.hpp"

std::string MAC::to_string() const
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

MAC::MAC(const std::string &str)
{
    std::string m(str);
    std::replace(m.begin(), m.end(), ':', ' ');
    std::stringstream ss(m);
    MAC_PARTS parts;
    ss >> std::hex >> parts[0] >> parts[1] >> parts[2] >> parts[3] >> parts[4] >> parts[5];
    this->bytes = partsToBytes(parts);
}

MAC::MAC(const MAC_PARTS &parts) : MAC(partsToBytes(parts)) {}

MAC::MAC(uint64_t bytes) : bytes(bytes) {}

MAC::MAC(const MAC &other) : MAC(other.bytes) {}

bool MAC::operator==(const MAC &other) const { return bytes == other.bytes; }

uint64_t MAC::partsToBytes(const MAC_PARTS &parts)
{
    uint64_t bytes = 0;
    for (int i = 0; i < 6; i++)
    {
        bytes = bytes << 8;
        bytes |= parts[i];
    }
    return bytes;
}

Ref<MAC_PARTS> MAC::bytesToParts(uint64_t bytes)
{
    Ref<MAC_PARTS> parts = std::make_shared<MAC_PARTS>(6);
    for (int i = 5; i >= 0; i--)
    {
        parts[i] = bytes & 0xFF;
        bytes >>= 8;
    }
    return parts;
}
