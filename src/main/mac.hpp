/**
 * Header criado para simular o endereço MAC de uma máquina
 */
#pragma once

#include <stdint.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include "types.hpp"

struct MAC
{
    uint64_t bytes;

	//Transforma o endereço MAC em uma string
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

	//A partir da string fornecida, cria um MAC
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

	//Tranformas suas partes em bytes, facilitando comparação
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

	//Tranforma seus bytes em partes, facilitando tratamento
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
	//Retorna um hash para o MAC
    size_t operator()(const MAC &mac) const
    {
        return std::hash<uint64_t>()(mac.bytes);
    }
};
