#pragma once

#include "types.hpp"

#include "peers.h"
#include <memory>

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

void B_C_errorCRC()
{
    ERROR_CONTROL test_error_control = ERROR_CONTROL::CRC;

    // Ref<Host> A = std::make_shared<Host>(MAC("AA:AA:AA:AA:AA:AA"), test_error_control);
    Ref<Host> B = std::make_shared<Host>(MAC("BB:BB:BB:BB:BB:BB"), test_error_control);
    Ref<Host> C = std::make_shared<Host>(MAC("CC:CC:CC:CC:CC:CC"), test_error_control);

    // A->setPromiscuousMode(true);

    // Ref<Switch> S1 = std::make_shared<Switch>(test_error_control, 2);
    Ref<Switch> S2 = std::make_shared<Switch>(test_error_control, 3);

    // EthernetPeer::connect(A, S1, 0, 1);
    // EthernetPeer::connect(S1, S2, 0, 0);
    EthernetPeer::connect(B, S2, 0, 1);
    EthernetPeer::connect(C, S2, 0, 2);

    L("\n[MAIN] B sends 'Hello' to C"_fmag);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    {
        Ether2Frame frame(C->m_MAC, B->m_MAC, "Hello", 6);
        B->sendFrame(0, frame);
    }
}
