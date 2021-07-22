#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>
#include <string.h>
#include <vector>
#include <memory>
#include <iostream>

#include "types.hpp"
#include "crc_32.hpp"

template <typename T>
using Ref = std::shared_ptr<T>;



struct MAC
{
    MAC_PARTS parts;

    unsigned int to_bytes() const
    {
        unsigned int bytes = 0;
        for (int i = 0; i < 6; i++)
        {
            bytes |= parts[i];
            bytes <<= 8;
        }
        return bytes;
    }

    std::string to_string() const
    {
        std::stringstream ss;
        ss << std::hex << std::setfill('0')
           << std::setw(2) << (int)parts[0] << ":"
           << std::setw(2) << (int)parts[1] << ":"
           << std::setw(2) << (int)parts[2] << ":"
           << std::setw(2) << (int)parts[3] << ":"
           << std::setw(2) << (int)parts[4] << ":"
           << std::setw(2) << (int)parts[5];
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

    MAC(unsigned int bytes)
    {
        for (int i = 0; i < 6; i++)
        {
            parts[i] = bytes & 0xFF;
            bytes >>= 8;
        }
    }

    MAC(const MAC &other)
    {
        memcpy(parts, other.parts, 6);
    }
};

struct Ether2Frame
{
    unsigned int dst : 6;
    unsigned int src : 6;
    unsigned int type : 2;
    char *data;
    unsigned int CRC;

    Ether2Frame(const MAC &dst, const MAC &src, const char *const data, unsigned int data_size)
        : dst(dst.to_bytes()), src(src.to_bytes())
    {
        this->data = (char *)malloc(sizeof(char) * data_size);
        memcpy(this->data, data, data_size);
        CRC = CRC32(data, data_size);
    }
};

template<>
struct std::hash<MAC>
{
    size_t operator()(const MAC &mac) const
    {
        return std::hash<unsigned int>()(mac.to_bytes());
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

    virtual void sendFrame(Ether2Frame &frame) {
        //Send frame to all ports with a valid peer
        for (unsigned int i = 0; i < ports.size(); i++)
            if (ports[i] != nullptr)
                ports[i]->receiveFrame(frame);
    }

    virtual void receiveFrame(Ether2Frame &frame) = 0;
    // virtual void onConnected(unsigned port) = 0;
    // virtual void onDisconnected(unsigned port) = 0;
    virtual ~EthernetPeer() {}
};

class Host : public EthernetPeer
{
public:
    MAC m_MAC;
    virtual void receiveFrame(Ether2Frame &frame) override
    {
        std::cout << "CurrentMAC: " << m_MAC.to_string() << std::endl;

        //Announce that this host has received the frame
        std::cout << "Received frame from " << MAC(frame.src).to_string() << ": " << frame.data << std::endl;
        std::cout << "Frame destination: " << MAC(frame.dst).to_string() << std::endl;

        //If the destination is not this host, drop the frame and return
        if (frame.dst != this->m_MAC.to_bytes()) {
            std::cout << "Dropping frame" << std::endl;
            return;
        }
        
        std::cout << "Accepting frame" << std::endl;
    }

    Host(const MAC &mac, unsigned int port_count = 1) : EthernetPeer(port_count), m_MAC(mac) {}
};

class Switch : public EthernetPeer
{
private:
    std::unordered_map<MAC, unsigned int> m_CAMTable;
    // std::unordered_map<IP, MAC> m_ARPTable;
public:
    virtual void receiveFrame(Ether2Frame &frame) override
    {
        //Send frame to all peers in m_CAMTable except the one who originally sent the frame
        for (auto it = m_CAMTable.begin(); it != m_CAMTable.end(); it++)
        {
            if (it->first.to_bytes() == frame.src)
                continue;

            this->ports[it->second]->receiveFrame(frame);
        }
    }

    Switch(unsigned int port_count = 32) : EthernetPeer(port_count) {}
};

int main(int argc, char const *argv[])
{
    //Create two computers with a random MAC address
    Ref<Host> A = std::make_shared<Host>(MAC(rand()));
    Ref<Host> B = std::make_shared<Host>(MAC(rand()));

    //Create a switch with 2 ports
    Ref<Switch> S = std::make_shared<Switch>(2);

    //Connect A and B through switch
    EthernetPeer::connect(A, S, 0, 0);
    EthernetPeer::connect(B, S, 0, 1);
    
    //TODO: change to a function

    //Create a frame from A to B with the message "Hello World"
    Ether2Frame frame(A->m_MAC.to_bytes(), B->m_MAC.to_bytes(), "Hello world!", 13);
    
    //Send the frame
    A->sendFrame(frame);

    return 0;
}
