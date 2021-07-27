#pragma once

#define __nezumi_debugger_on__ 1
#define D(x) { if (__nezumi_debugger_on__) { x } }
#define L(x) { std::cout << x << std::endl; }

struct MAC;
struct Ether2Frame;
struct Data;

class DataReceiver;
class PC;
class Switch;

using MAC_PARTS = uint16_t[6];

enum class ERROR_CONTROL {
    EVEN, 
    ODD,
    CRC
};

template <typename T>
using Ref = std::shared_ptr<T>;
