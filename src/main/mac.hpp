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
    std::string to_string() const;

    //A partir da string fornecida, cria um MAC
    MAC(const std::string &str);
    //A partir das partes fornecidas (6 partes em hexa), cria um MAC
    MAC(const MAC_PARTS &parts);
    //A partir dos bytes fornecidos, cria um MAC
    MAC(uint64_t bytes);
    //clona um objeto MAC
    MAC(const MAC &other);

    bool operator==(const MAC &other) const;

    //Tranformas suas partes em bytes, facilitando comparação
    static uint64_t partsToBytes(const MAC_PARTS &parts);

    //Tranforma seus bytes em partes, facilitando tratamento
    static Ref<MAC_PARTS> bytesToParts(uint64_t bytes);
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