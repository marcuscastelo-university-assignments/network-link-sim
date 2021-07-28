#include "peers.hpp"

#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <chrono>

#include "frame.hpp"
#include "types.hpp"
#include "mac.hpp"

using namespace std::chrono_literals;

const static auto TTL = std::chrono::duration_cast<std::chrono::milliseconds>(15s).count();

EthernetPeer::EthernetPeer(ERROR_CONTROL error_control_type, unsigned port_count)
    : interfaces(port_count), m_ErrorControlType(error_control_type)
{
}

void EthernetPeer::connect(const Ref<EthernetPeer> &A, const Ref<EthernetPeer> &B, unsigned portA, unsigned portB)
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

void EthernetPeer::disconnect(const Ref<EthernetPeer> &A, const Ref<EthernetPeer> &B)
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

void EthernetPeer::sendFrame(uint16_t interface, Ether2Frame &frame)
{
    interfaces[interface]->receiveFrame(this, frame);
}

void Host::receiveFrame(const EthernetPeer *const sender_ptr, Ether2Frame &frame)
{
    L("");
    frame._simulation_fake_noise();

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
    frame.prettyPrint();

    if (this->m_ErrorControlType == ERROR_CONTROL::CRC)
    {
        if (!frame.checkCRC())
        {
            L("The frame CRC is invalid, dropping it"_fred);
        }
    }
    else if (this->m_ErrorControlType == ERROR_CONTROL::EVEN)
    {
        if (!frame.checkEven())
        {
            L("The frame parity bit (even) is invalid, dropping it"_fred);
        }
    }
    else
    {
        if (!frame.checkOdd())
        {
            L("The frame parity bit (odd) is invalid, dropping it"_fred);
        }
    }
}

void Host::setPromiscuousMode(bool promiscuous) { m_PromiscuousMode = promiscuous; }

Host::Host(const MAC &mac, ERROR_CONTROL error_control_type, unsigned int port_count)
    : EthernetPeer(error_control_type, port_count), m_MAC(mac)
{
}

void Switch::sendToAllExceptSender(uint16_t senderInterface, Ether2Frame &frame)
{
    //Announce frame source and destination
    std::cout << "(SWITCH) Sending frame to all interfaces except "_fblu << senderInterface << std::endl;

    //Send frame to all ports with a valid peer
    for (unsigned int i = 0; i < interfaces.size(); i++)
        if (interfaces[i] != nullptr && i != senderInterface)
            interfaces[i]->receiveFrame(this, frame);
}

size_t Switch::getSenderInterface(const EthernetPeer *const sender_ptr)
{
    //Gets the port index which matches with sender_ptr
    for (unsigned int i = 0; i < interfaces.size(); i++)
        if (interfaces[i] != nullptr && interfaces[i].get() == sender_ptr)
            return i;

    //If not found, throw an exception
    throw std::runtime_error("Sender not found");
}

void Switch::receiveFrame(const EthernetPeer *const sender_ptr, Ether2Frame &frame)
{
    frame._simulation_fake_noise();

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

    if (findIt->second.interface == senderInterface)
    {
        D(L("(SWITCH) The destination of the packet is in the same interface that sent it, dropping!"_fred));
        return;
    }

    D(L("(SWITCH) Sending to destination (it was in the switch table)"_fgre));
    sendFrame(findIt->second.interface, frame);
}

Switch::Switch(ERROR_CONTROL error_control_type, unsigned int port_count)
    : EthernetPeer(error_control_type, port_count)
{
}