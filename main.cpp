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
#include <thread>
#include "tui.hpp"

using namespace std::chrono_literals;
using namespace tui::text_literals;
using namespace tui::text;

using TT = tui::text::Text;

#include "types.hpp"
#include "crc_32.hpp"

template <typename T>
using Ref = std::shared_ptr<T>;

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
        : dst(dst.bytes), src(src.bytes)
    {
        memcpy(this->data, data, data_size);
        CRC = CRC32(data, data_size);
    }
};

class EthernetPeer
{

protected:
    std::vector<Ref<EthernetPeer>> interfaces;
    ERROR_CONTROL m_ErrorControlType;

public:
    EthernetPeer(ERROR_CONTROL error_control_type, unsigned port_count)
        : interfaces(port_count), m_ErrorControlType(error_control_type)
    {
    }

    static void connect(const Ref<EthernetPeer> &A, const Ref<EthernetPeer> &B, unsigned portA, unsigned portB)
    {
        //Check if port already has a valid pointer
        if (A->interfaces[portA] == nullptr)
            A->interfaces[portA] = B;
        else if (A->interfaces[portA] != B)
            throw std::runtime_error("In A, portA is already connected to a different peer");

        if (B->interfaces[portB] == nullptr)
            B->interfaces[portB] = A;
        else if (B->interfaces[portB] != A)
            throw std::runtime_error("In B, portB is already connected to a different peer");

        //Raise exception if both error checking methods are not the same
        if (A->m_ErrorControlType != B->m_ErrorControlType)
            throw std::runtime_error("Error control type of peers are different");

        //TODO: in case A and B are already connected, it will cause a reconnection (is this desirable?)
        A->interfaces[portA] = B;
        B->interfaces[portB] = A;
    }

    static void disconnect(const Ref<EthernetPeer> &A, const Ref<EthernetPeer> &B)
    {
        //Get in which port B is connected
        unsigned portA = 0;
        for (; portA < A->interfaces.size(); portA++)
            if (A->interfaces[portA] == B)
                break;

        //Get in which port a is connected
        unsigned portB = 0;
        for (; portB < B->interfaces.size(); portB++)
            if (B->interfaces[portB] == A)
                break;

        //If not found portA or portB, throw an exception
        if (portA == A->interfaces.size() || portB == B->interfaces.size())
            throw std::runtime_error("Peers are not connected");

        //Disconect peers
        A->interfaces[portA] = nullptr;
        B->interfaces[portB] = nullptr;
    }

    virtual void sendFrame(uint16_t interface, Ether2Frame &frame)
    {
        interfaces[interface]->receiveFrame(this, frame);
    }

    virtual void receiveFrame(const EthernetPeer *const sender_ptr, Ether2Frame &frame) = 0;
    virtual ~EthernetPeer() {}
};

class Host : public EthernetPeer
{
private:
    bool m_PromiscuousMode = false;

public:
    MAC m_MAC;
    virtual void receiveFrame(const EthernetPeer *const sender_ptr, Ether2Frame &frame) override
    {
        L("");
        //Announce that this host has received the frame
        L("(Host) Received frame from "_fblu << MAC(frame.src).to_string());
        L("(Host) Frame destination: "_fblu << MAC(frame.dst).to_string());

        L("(Host) CurrentMAC: "_fblu << m_MAC.to_string());
        //If the destination is not this host, drop the frame and return
        if (m_PromiscuousMode)
        {
            L("(Host) WARNING: Promiscuous mode enabled!!"_fyel);
        }
        else if (frame.dst != this->m_MAC.bytes)
        {
            L("The frame was not destinated to this host, dropping it"_fwhi);
            return;
        }

        L("(Host) Frame accepted!"_fgre);
        L("(Host) Frame content: "_fgre << TT{(const char *)frame.data}.FWhite().Bold());
    }

    inline void setPromiscuousMode(bool promiscuous) { m_PromiscuousMode = promiscuous; }

    Host(const MAC &mac, ERROR_CONTROL error_control_type, unsigned int port_count = 1)
        : EthernetPeer(error_control_type, port_count), m_MAC(mac)
    {
    }
};

struct SwitchTableEntry
{
    uint16_t interface;
    uint64_t lastUpdate;
};

const auto TTL = std::chrono::duration_cast<std::chrono::milliseconds>(15s).count();

using SwitchTable = std::unordered_map<MAC, SwitchTableEntry>;

class Switch : public EthernetPeer
{
private:
    const uint8_t MAX_TABLE_SIZE = 4;

    SwitchTable m_SwitchTable;

    void sendToAllExceptSender(uint16_t senderInterface, Ether2Frame &frame)
    {
        //Announce frame source and destination
        std::cout << "(SWITCH) Sending frame to all interfaces except "_fblu << senderInterface << std::endl;

        //Send frame to all ports with a valid peer
        for (unsigned int i = 0; i < interfaces.size(); i++)
            if (interfaces[i] != nullptr && i != senderInterface)
                interfaces[i]->receiveFrame(this, frame);
    }

    size_t getSenderInterface(const EthernetPeer *const sender_ptr)
    {
        //Gets the port index which matches with sender_ptr
        for (unsigned int i = 0; i < interfaces.size(); i++)
            if (interfaces[i] != nullptr && interfaces[i].get() == sender_ptr)
                return i;

        //If not found, throw an exception
        throw std::runtime_error("Sender not found");
    }

public:
    virtual void receiveFrame(const EthernetPeer *const sender_ptr, Ether2Frame &frame) override
    {
        L("");
        //Announce frame receival
        std::cout << "(SWITCH) Received frame from "_fblu << MAC(frame.src).to_string() << ": " << frame.data << std::endl;
        std::cout << "(SWITCH) Frame destination: "_fblu << MAC(frame.dst).to_string() << std::endl;

        size_t senderInterface = 0;
        try
        {
            senderInterface = getSenderInterface(sender_ptr);
        }
        catch (const std::exception &e)
        {
            std::cerr << e.what() << '\n';
            return;
        }

        //Get current time in milliseconds
        uint64_t currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

        //TODO: check what should happen if the same MAC is presented in another interface before TTL expires
        //If sender not in switch table, add it, else update TTL and interface for MAC
        m_SwitchTable[MAC(frame.src)] = {(uint16_t)senderInterface, currentTime};

        auto findIt = m_SwitchTable.find(MAC(frame.dst));

        //If dest not in table or table is full, just send to all except sender
        if (findIt == m_SwitchTable.end() || m_SwitchTable.size() == MAX_TABLE_SIZE)
        {
            D(L("(SWITCH) Destination not in table, sending to all except sender"_fyel));
            sendToAllExceptSender(senderInterface, frame);
            return;
        }

        //If dest TTL expired, remove from switch table and send to all except sender
        if (currentTime - findIt->second.lastUpdate > TTL)
        {
            D(L("(SWITCH) TTL expired, removing from table and sending to all except sender"_fyel));
            m_SwitchTable.erase(frame.src);
            m_SwitchTable[MAC(frame.src)] = {(uint16_t)senderInterface, currentTime};
            sendToAllExceptSender(senderInterface, frame);
            return;
        }

        //If it's all ok, just send to destination

        if (findIt->second.interface == senderInterface) {
            D(L("(SWITCH) The destination of the packet is in the same interface that sent it, dropping!"_fred));
            return;
        }

        D(L("(SWITCH) Sending to destination (it was in the switch table)"_fgre));
        sendFrame(findIt->second.interface, frame);
    }

    Switch(ERROR_CONTROL error_control_type, unsigned int port_count = 32)
        : EthernetPeer(error_control_type, port_count)
    {
    }
};

void A_B_ttl_andPromC()
{
    ERROR_CONTROL test_error_control = ERROR_CONTROL::CRC;

    Ref<Host> A = std::make_shared<Host>(MAC("AA:AA:AA:AA:AA:AA"), test_error_control);
    Ref<Host> B = std::make_shared<Host>(MAC("BB:BB:BB:BB:BB:BB"), test_error_control);
    Ref<Host> C = std::make_shared<Host>(MAC("CC:CC:CC:CC:CC:CC"), test_error_control);

    C->setPromiscuousMode(true);

    Ref<Switch> S1 = std::make_shared<Switch>(test_error_control, 2);
    Ref<Switch> S2 = std::make_shared<Switch>(test_error_control, 3);

    EthernetPeer::connect(A, S1, 0, 1);
    EthernetPeer::connect(S1, S2, 0, 0);
    EthernetPeer::connect(B, S2, 0, 1);
    EthernetPeer::connect(C, S2, 0, 2);

    L("\n[MAIN] A sends 'Hello' to B"_fmag);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    {
        Ether2Frame frame(B->m_MAC, A->m_MAC, "Hello", 6);
        A->sendFrame(0, frame);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    L("\n[MAIN] B sends 'Oh, Hello!' to A"_fmag);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    {
        Ether2Frame frame(A->m_MAC, B->m_MAC, "Oh, Hello!", 11);
        B->sendFrame(0, frame);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    L("\n[MAIN] A sends 'BRB' to B"_fmag);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    {
        Ether2Frame frame(B->m_MAC, A->m_MAC, "BRB", 4);
        A->sendFrame(0, frame);
    }

    //Wait 20s
    L("\n[MAIN] After a long time (TTL has expired)"_fmag);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    L("\n[MAIN] A sends 'I'm Back' to B"_fmag);
    std::this_thread::sleep_for(std::chrono::seconds(16));

    //Create a frame from A to B with the message "Hello World"
    Ether2Frame frame(B->m_MAC, A->m_MAC, "I'm back!", 10);
    //Send the frame
    A->sendFrame(0, frame);
}

void B_C_self_andPromA()
{
    ERROR_CONTROL test_error_control = ERROR_CONTROL::CRC;

    Ref<Host> A = std::make_shared<Host>(MAC("AA:AA:AA:AA:AA:AA"), test_error_control);
    Ref<Host> B = std::make_shared<Host>(MAC("BB:BB:BB:BB:BB:BB"), test_error_control);
    Ref<Host> C = std::make_shared<Host>(MAC("CC:CC:CC:CC:CC:CC"), test_error_control);

    A->setPromiscuousMode(true);

    Ref<Switch> S1 = std::make_shared<Switch>(test_error_control, 2);
    Ref<Switch> S2 = std::make_shared<Switch>(test_error_control, 3);

    EthernetPeer::connect(A, S1, 0, 1);
    EthernetPeer::connect(S1, S2, 0, 0);
    EthernetPeer::connect(B, S2, 0, 1);
    EthernetPeer::connect(C, S2, 0, 2);

    L("\n[MAIN] B sends 'Hello' to C"_fmag);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    {
        Ether2Frame frame(C->m_MAC, B->m_MAC, "Hello", 6);
        B->sendFrame(0, frame);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    L("\n[MAIN] C sends 'Oh, Hello!' to B"_fmag);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    {
        Ether2Frame frame(B->m_MAC, C->m_MAC, "Oh, Hello!", 11);
        C->sendFrame(0, frame);
    }

    L("\n[MAIN] B sends 'Everything ok?' to C"_fmag);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    {
        Ether2Frame frame(C->m_MAC, B->m_MAC, "Everything ok?", 15);
        B->sendFrame(0, frame);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    L("\n[MAIN] C sends 'Yeah, pretty much' to itself by mistake"_fmag);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    {
        Ether2Frame frame(C->m_MAC, C->m_MAC, "Yeah, pretty much", 18);
        C->sendFrame(0, frame);
    }
}

int main(int argc, char const *argv[])
{
    while (true)
    {
        tui::clear();
        tui::printl("Welcome to our data-link layer simulation"_fblu);

        tui::printl("Select a story to run:"_fwhi.Bold());
        tui::printl("  1. A-B with TTL expiring and promiscuous mode on C"_fgre);
        tui::printl("  2. C-B with C sending a frame to itself and A in promiscuous mode"_fgre);
        //TODO: add CRC, EVEN, ODD stories
        tui::printl("");
        tui::printl("  q. quit"_fred);
        auto opt = tui::readline();

        if (opt.size() < 1)
            continue;
        switch (opt[0])
        {
        case '1':
            A_B_ttl_andPromC();
            break;
        case '2':
            B_C_self_andPromA();
            break;
        case 'q':
            return 0;
        }

        tui::printl("End of the story!"_fmag);
        tui::printl("Press enter to restart..."_fwhi.Bold());
        tui::readline();
    }

    return 0;
}
