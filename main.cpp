#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>
#include <string.h>
#include <vector>
#include <memory>
#include <iostream>
#include <unistd.h>
#include <chrono>

using namespace std::chrono_literals;

#include "types.hpp"
#include "crc_32.hpp"

template <typename T>
using Ref = std::shared_ptr<T>;

struct MAC
{
    MAC_PARTS parts;

    uint64_t to_bytes() const
    {
        uint64_t bytes = 0;
        for (int i = 0; i < 6; i++)
        {
            bytes = bytes << 8;
            bytes |= parts[i];
        }
        return bytes;
    }

    std::string to_string() const
    {
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
        ss >> std::hex >> parts[0] >> parts[1] >> parts[2] >> parts[3] >> parts[4] >> parts[5];
    }

    MAC(const MAC_PARTS &parts)
    {
        for (int i = 0; i < 6; i++)
            this->parts[i] = parts[i];
    }

    MAC(uint64_t bytes)
    {
        for (int i = 5; i >= 0; i--)
        {
            parts[i] = bytes & 0xFF;
            bytes >>= 8;
        }
    }

    MAC(const MAC &other)
    {
        memcpy(parts, other.parts, 6 * sizeof(uint16_t));
    }
};

template <>
struct std::hash<MAC>
{
    size_t operator()(const MAC &mac) const
    {
        return std::hash<uint64_t>()(mac.to_bytes());
    }
};


struct Ether2Frame
{
    uint64_t
        dst : 48,
        src : 48,
        type : 16;

    //TODO: comentar uso de 1500 bytes (limitado pela linguagem (alternativa é usar a heap, mas perde fidedignidade))
    uint8_t data[1500];
    uint32_t CRC;

    Ether2Frame(const MAC &dst, const MAC &src, const char *const data, unsigned int data_size)
        : dst(dst.to_bytes()), src(src.to_bytes())
    {
        memcpy(this->data, data, data_size);
        CRC = CRC32(data, data_size);
    }
};


class EthernetPeer
{

protected:
    std::vector<Ref<EthernetPeer>> ports;

public:
    EthernetPeer(unsigned port_count) : ports(port_count) {}

    static void connect(const Ref<EthernetPeer> &A, const Ref<EthernetPeer> &B, unsigned portA, unsigned portB)
    {
        //Check if port already has a valid pointer
        if (A->ports[portA] == nullptr)
            A->ports[portA] = B;
        else if (A->ports[portA] != B)
            throw std::runtime_error("In A, portA is already connected to a different peer");

        if (B->ports[portB] == nullptr)
            B->ports[portB] = A;
        else if (B->ports[portB] != A)
            throw std::runtime_error("In B, portB is already connected to a different peer");

        //TODO: in case A and B are already connected, it will cause a reconnection (is this desirable?)

        A->ports[portA] = B;
        B->ports[portB] = A;

        // A->onConnected(portA);
        // B->onConnected(portB);
    }

    static void disconnect(const Ref<EthernetPeer> &A, const Ref<EthernetPeer> &B)
    {
        //Get in which port B is connected
        unsigned portA = 0;
        for (; portA < A->ports.size(); portA++)
            if (A->ports[portA] == B)
                break;

        //Get in which port a is connected
        unsigned portB = 0;
        for (; portB < B->ports.size(); portB++)
            if (B->ports[portB] == A)
                break;

        //If not found portA or portB, throw an exception
        if (portA == A->ports.size() || portB == B->ports.size())
            throw std::runtime_error("Peers are not connected");

        //Disconect peers
        A->ports[portA] = nullptr;
        B->ports[portB] = nullptr;
    }

    virtual void sendFrame(Ether2Frame &frame)
    {
        //Send frame to all ports with a valid peer
        for (unsigned int i = 0; i < ports.size(); i++)
            if (ports[i] != nullptr)
                ports[i]->receiveFrame(this, frame);
    }

    virtual void receiveFrame(const EthernetPeer *const sender_ptr, Ether2Frame &frame) = 0;
    // virtual void onConnected(unsigned port) = 0;
    // virtual void onDisconnected(unsigned port) = 0;
    virtual ~EthernetPeer() {}
};

class Host : public EthernetPeer
{
public:
    MAC m_MAC;
    virtual void receiveFrame(const EthernetPeer *const sender_ptr, Ether2Frame &frame) override
    {
        std::cout << "CurrentMAC: " << m_MAC.to_string() << std::endl;
        // std::cout << "CurrentMAC (bytes): " << m_MAC.to_bytes() << std::endl;

        //Announce that this host has received the frame
        std::cout << "Received frame from " << MAC(frame.src).to_string() << ": " << frame.data << std::endl;
        std::cout << "Frame destination: " << MAC(frame.dst).to_string() << std::endl;
        // std::cout << "Frame destination (bytes): " << frame.dst << std::endl;
        // std::cout << "Frame destination (bytes conv): " << MAC(frame.dst).to_bytes() << std::endl;

        //If the destination is not this host, drop the frame and return
        if (frame.dst != this->m_MAC.to_bytes())
        {
            std::cout << "Dropping frame" << std::endl;
            return;
        }

        std::cout << "Accepting frame" << std::endl;
    }

    Host(const MAC &mac, unsigned int port_count = 1) : EthernetPeer(port_count), m_MAC(mac) {}
};

struct SwitchTableEntry {
    uint16_t interface;
    uint64_t lastUpdate;
};

const auto TTL = std::chrono::duration_cast<std::chrono::milliseconds>(10s).count();

using SwitchTable = std::unordered_map<MAC, SwitchTableEntry>;

class Switch : public EthernetPeer
{
private:
    const uint8_t MAX_TABLE_SIZE = 4;
    
    SwitchTable m_SwitchTable;

    void sendToAllExceptSender(const EthernetPeer *const sender_ptr, Ether2Frame &frame)
    {
        std::cout << "Switch received, repassing" << std::endl
                  << std::endl;

        int senderInterface = getSenderInterface(sender_ptr);

        //Send frame to all ports with a valid peer
        for (unsigned int i = 0; i < ports.size(); i++)
            if (ports[i] != nullptr && i != senderInterface)
                ports[i]->receiveFrame(this, frame);
    }

    int getSenderInterface(const EthernetPeer *const sender_ptr)
    {
        //Gets the port index which matches with sender_ptr
        for (unsigned int i = 0; i < ports.size(); i++)
            if (ports[i] != nullptr && ports[i].get() == sender_ptr)
                return i;

        return -1;
    }

public:
    virtual void receiveFrame(const EthernetPeer *const sender_ptr, Ether2Frame &frame) override
    {
        int senderInterface = getSenderInterface(sender_ptr);

        //If not connected to sender, throw an exception
        if (senderInterface < 0)
            throw std::runtime_error("Switch is not connected to sender");

        //Get current time in milliseconds
        uint64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();


        
        //If sender not in switch table
        if (m_SwitchTable.find(MAC(frame.src)) == m_SwitchTable.end()) {
            m_SwitchTable[MAC(frame.src)] = { (uint16_t)senderInterface, currentTime };
        } else {
            // Check if table entry's TTL has expired
            if (currentTime - m_SwitchTable[frame.src].lastUpdate > TTL) {
                m_SwitchTable.erase(frame.src);
                m_SwitchTable[MAC(frame.src)] = { (uint16_t)senderInterface, currentTime };
            }
        }

        //If dest not in table or table is full:
        if (m_SwitchTable.find(MAC(frame.dst)) == m_SwitchTable.end() || m_SwitchTable.size() == MAX_TABLE_SIZE) {
            sendToAllExceptSender(sender_ptr, frame);
        }
        else {
            //TODO: implement table
        }
    }

    Switch(unsigned int port_count = 32) : EthernetPeer(port_count) {}
};

int main(int argc, char const *argv[])
{
    //Create two computers with a random MAC address
    Ref<Host> A = std::make_shared<Host>(MAC("00:00:00:00:00:00"));
    Ref<Host> B = std::make_shared<Host>(MAC("FF:FF:FF:FF:FF:FF"));

    //Print A and B MAC addressess in string and byte form
    std::cout << "A: " << A->m_MAC.to_string() << std::endl;
    std::cout << "A (bytes): " << A->m_MAC.to_bytes() << std::endl;
    std::cout << "B: " << B->m_MAC.to_string() << std::endl;
    std::cout << "B (bytes): " << B->m_MAC.to_bytes() << std::endl;
    std::cout << std::endl
              << std::endl;

    //Create a switch with 2 ports
    Ref<Switch> S1 = std::make_shared<Switch>(2);
    Ref<Switch> S2 = std::make_shared<Switch>(2);

    //Connect A and B through switches
    EthernetPeer::connect(A, S1, 0, 0);
    EthernetPeer::connect(S1, S2, 1, 1);
    EthernetPeer::connect(B, S2, 0, 0);

    //TODO: change to a function

    //Create a frame from A to B with the message "Hello World"
    Ether2Frame frame(B->m_MAC, A->m_MAC, "Hello world!", 13);

    //Send the frame
    A->sendFrame(frame);

    return 0;
}
