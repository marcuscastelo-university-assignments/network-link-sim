#include "frame.hpp"

#include <iostream>
#include <stdint.h>
#include <string.h>

#include "mac.hpp"
#include "crc_32.hpp"
#include "tui.hpp"

using namespace tui::text_literals;

Ether2Frame::Ether2Frame(
    const MAC &dst,
    const MAC &src,
    const char *const data,
    size_t data_size,
    ERROR_CONTROL errorType)
    : dst(dst.bytes),
      src(src.bytes)
{
    if (data_size > 1500) data_size = 1500;
    memset(this->data, '\0', 1500);
    memcpy(this->data, data, data_size);
    if (errorType == ERROR_CONTROL::CRC)
    {
        verifyContent = CRC32(this->data, 1500);
    }
    else if (errorType == ERROR_CONTROL::EVEN)
    {
        verifyContent = paridadePar((void *)this->data, 1500);
    }
    else
    {
        verifyContent = paridadeImpar((void *)this->data, 1500);
    }
}

void Ether2Frame::prettyPrint()
{
    std::cout << " [ dst: " << std::hex << std::setfill('0') << std::setw(2) << (uint64_t)dst;
    std::cout << " | src: " << std::hex << std::setfill('0') << std::setw(2) << (uint64_t)src;
    std::cout << " | type: " << std::setw(2) << type;
    std::cout << " | payload: " << data;
    std::cout << " | verificador: " << verifyContent;
    std::cout << " ] " << std::endl;
}

void Ether2Frame::_simulation_fake_noise(float probability, size_t randomize_below)
{
    if (randomize_below == (size_t)-1)
        randomize_below = strlen((const char *)data);

    if (rand() % 100 <= probability * 100)
    {
        size_t byteToRandomize = rand() % randomize_below;
        size_t bitToRandomize = rand() % 8;
        std::cout << std::endl;
        std::cout << "*** Simulating ERROR!!! *** "_fred << std::endl;
        std::cout << "  Flipping bit "_fred << bitToRandomize << " of byte "_fred << byteToRandomize << std::endl;
        std::cout << "  Data before: "_fblu << data << std::endl;
        data[byteToRandomize] ^= 0b00000001 << (bitToRandomize);
        std::cout << "  Data after: "_fblu << data << std::endl;
        std::cout << "*** Simulated error *** "_fred << std::endl;

        std::cout << std::endl;
    }
}

bool Ether2Frame::checkCRC()
{
    return verifyContent == CRC32(this->data, 1500);
}

bool Ether2Frame::checkEven()
{
    return verifyContent == paridadePar((void *)this->data, 1500);
}

bool Ether2Frame::checkOdd()
{
    return verifyContent == paridadeImpar((void *)this->data, 1500);
}