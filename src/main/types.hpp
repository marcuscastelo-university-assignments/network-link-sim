/**
 * Header auxiliar para definição de tipos criados e usados no trabalho
 */
#pragma once

#include <memory>

template <typename T>
using Ref = std::shared_ptr<T>;

#define __nezumi_debugger_on__ 1
#define D(x) { if (__nezumi_debugger_on__) { x } }
#define L(x) { std::cout << x << std::endl; }


// Declarações (forward declarations), para evitar problemas com dependencias cíclicas
struct MAC;
struct Ether2Frame;
struct Data;

class DataReceiver;
class PC;
class Switch;

// Partes de um mac PART1 :: PART2 :: etc...
using MAC_PARTS = uint16_t[6];

enum class ERROR_CONTROL {
    EVEN, 
    ODD,
    CRC
};