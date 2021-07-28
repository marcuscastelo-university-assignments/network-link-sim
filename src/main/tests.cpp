/**
 * Header auxiliar para testes de envio de frames entre os computadores
 */
#include "tests.hpp"

#include "peers.hpp"
#include <memory>
#include <thread>

/**
 * Método que simula conexão de computadores A, B e C, com A no Switch S1, B e C no switch S2 e ambos switches conectados
 * Nessa simulação, o Host C está no modo promíscuo (abrindo frames que não devia).
 * 
 * A topologia dos switches é esta S1 <-> S2
 * em cada switch:
 *  S1: A
 *  S2: B e C
 * 
 * ou seja: (A) = S1 <-> S2 = (B, C).
 * 
 * Nessa simulação, uma mensagem é enviada de A para B.
 * Primeiramente, A envia para S1 o frame,
 * S1 não sabe em qual interface B, então ele envia para todas as portas, exceto a de que recebeu o frame.
 * Dessa forma, S1 encaminha o frame para S2, que faz a mesma coisa,
 * enviando o frame para B e C.
 * B vê que ele é o destinatário e recebe o frame.
 * C vê que ele NÃO é o destinatário, mas por estar no modo promíscuo, abre mesmo assim.
 * 
 * Nas próximas comunicações, o switch já aprendeu o caminho e não vai enviar para C 
 * os frames que não são destinados a ele. 
 * 
 * Por fim, após a expiração do TTL, o host C será capaz de espiar novamente os pacotes enviados,
 * pois o switch vai fazer o flood.
 */
void A_B_ttl_andPromC()
{
    ERROR_CONTROL test_error_control = ERROR_CONTROL::CRC;

    Ref<Host> A = std::make_shared<Host>(MAC("AA:AA:AA:AA:AA:AA"), test_error_control);
    Ref<Host> B = std::make_shared<Host>(MAC("BB:BB:BB:BB:BB:BB"), test_error_control);
    Ref<Host> C = std::make_shared<Host>(MAC("CC:CC:CC:CC:CC:CC"), test_error_control);

    C->setPromiscuousMode(true);

    Ref<Switch> S1 = std::make_shared<Switch>(test_error_control, 2);
    Ref<Switch> S2 = std::make_shared<Switch>(test_error_control, 3);

    //Nos Hosts, há apenas uma interface (0). Nos switches a convenção usada foi (0) para ligar 2 switches e as restantes para os hosts
    EthernetPeer::connect(A, S1, 0, 1);
    EthernetPeer::connect(S1, S2, 0, 0);
    EthernetPeer::connect(B, S2, 0, 1);
    EthernetPeer::connect(C, S2, 0, 2);

    L("\n[MAIN] A sends 'Hello' to B"_fmag);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    {
        Ether2Frame frame(B->m_MAC, A->m_MAC, "Hello", 6, test_error_control);
        A->sendFrame(0, frame);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    L("\n[MAIN] B sends 'Oh, Hello!' to A"_fmag);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    {
        Ether2Frame frame(A->m_MAC, B->m_MAC, "Oh, Hello!", 11, test_error_control);
        B->sendFrame(0, frame);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    L("\n[MAIN] A sends 'BRB' to B"_fmag);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    {
        Ether2Frame frame(B->m_MAC, A->m_MAC, "BRB", 4, test_error_control);
        A->sendFrame(0, frame);
    }

    //Wait 20s
    L("\n[MAIN] After a long time (TTL has expired)"_fmag);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    L("\n[MAIN] A sends 'I'm Back' to B"_fmag);
    std::this_thread::sleep_for(std::chrono::seconds(16));

    //Create a frame from A to B with the message "Hello World"
    Ether2Frame frame(B->m_MAC, A->m_MAC, "I'm back!", 10, test_error_control);
    //Send the frame
    A->sendFrame(0, frame);
}

/**
 * Método que simula conexão de computadores A, B e C, com A no Switch S1, B e C no switch S2 e ambos switches conectados
 * 
 * Análogo à função acima, porém a comunicação é entre B e C, com A no modo promíscuo (a topologia é a mesma)
 */
void B_C_self_andPromA()
{
    ERROR_CONTROL test_error_control = ERROR_CONTROL::CRC;

    Ref<Host> A = std::make_shared<Host>(MAC("AA:AA:AA:AA:AA:AA"), test_error_control);
    Ref<Host> B = std::make_shared<Host>(MAC("BB:BB:BB:BB:BB:BB"), test_error_control);
    Ref<Host> C = std::make_shared<Host>(MAC("CC:CC:CC:CC:CC:CC"), test_error_control);

    A->setPromiscuousMode(true);

    Ref<Switch> S1 = std::make_shared<Switch>(test_error_control, 2);
    Ref<Switch> S2 = std::make_shared<Switch>(test_error_control, 3);

    //Nos Hosts, há apenas uma interface (0). Nos switches a convenção usada foi (0) para ligar 2 switches e as restantes para os hosts
    EthernetPeer::connect(A, S1, 0, 1);
    EthernetPeer::connect(S1, S2, 0, 0);
    EthernetPeer::connect(B, S2, 0, 1);
    EthernetPeer::connect(C, S2, 0, 2);

    L("\n[MAIN] B sends 'Hello' to C"_fmag);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    {
        Ether2Frame frame(C->m_MAC, B->m_MAC, "Hello", 6, test_error_control);
        B->sendFrame(0, frame);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    L("\n[MAIN] C sends 'Oh, Hello!' to B"_fmag);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    {
        Ether2Frame frame(B->m_MAC, C->m_MAC, "Oh, Hello!", 11, test_error_control);
        C->sendFrame(0, frame);
    }

    L("\n[MAIN] B sends 'Everything ok?' to C"_fmag);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    {
        Ether2Frame frame(C->m_MAC, B->m_MAC, "Everything ok?", 15, test_error_control);
        B->sendFrame(0, frame);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    L("\n[MAIN] C sends 'Yeah, pretty much' to itself by mistake"_fmag);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    {
        Ether2Frame frame(C->m_MAC, C->m_MAC, "Yeah, pretty much", 18, test_error_control);
        C->sendFrame(0, frame);
    }
}

/**
 * Método que simula um erro na transmissão de um frame do computador B para o C, simulando erro
 * 
 * Topologia mais simples que as funções anteriores: B se comunica com C por meio de um switch
 * 
 * O foco dessa simulação é demonstrar a checagem de erro por meio do método CRC.
 */
void B_C_error(ERROR_CONTROL test_error_control)
{
    // Ref<Host> A = std::make_shared<Host>(MAC("AA:AA:AA:AA:AA:AA"), test_error_control);
    Ref<Host> B = std::make_shared<Host>(MAC("BB:BB:BB:BB:BB:BB"), test_error_control);
    Ref<Host> C = std::make_shared<Host>(MAC("CC:CC:CC:CC:CC:CC"), test_error_control);

    // A->setPromiscuousMode(true);

    // Ref<Switch> S1 = std::make_shared<Switch>(test_error_control, 2);
    Ref<Switch> S2 = std::make_shared<Switch>(test_error_control, 3);

    //Nos Hosts, há apenas uma interface (0). Nos switches a convenção usada foi (0) para ligar 2 switches e as restantes para os hosts
    // EthernetPeer::connect(A, S1, 0, 1);
    // EthernetPeer::connect(S1, S2, 0, 0);
    EthernetPeer::connect(B, S2, 0, 1);
    EthernetPeer::connect(C, S2, 0, 2);

    L("\n[MAIN] B sends 'Hello' to C"_fmag);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    {
        Ether2Frame frame(C->m_MAC, B->m_MAC, "Hello", 6, test_error_control);
        B->sendFrame(0, frame);
    }
}
