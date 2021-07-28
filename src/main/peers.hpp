/**
 * Header criado para auxiliar no fluxo da rede, transmitindo os frames
 */
#include <vector>
#include <stdexcept>
#include <unordered_map>
#include <chrono>

#include "frame.hpp"
#include "types.hpp"
#include "mac.hpp"

using namespace std::chrono_literals;

class EthernetPeer
{

protected:
    std::vector<Ref<EthernetPeer>> interfaces;
    ERROR_CONTROL m_ErrorControlType;

public:
    EthernetPeer(ERROR_CONTROL error_control_type, unsigned port_count);

    /**
	 * Método auxiliar que simula a conexão entre 2 computadores, utilizando suas portas
	 * 
	 * Parâmetros:	const Ref<EthernetPeer> &A	=>	Computador A
	 * 				const Ref<EthernetPeer> &B	=>	Computador B
	 * 				unsigned portA				=>	Porta do computador A
	 * 				unsigned portA				=>	Porta do computador B
	 * 
	 * Retorno: void
	 */
    static void connect(const Ref<EthernetPeer> &A, const Ref<EthernetPeer> &B, unsigned portA, unsigned portB);

    /**
	 * Método auxiliar que simula a desconexão entre dois computadores
	 * 
	 * Parâmetros:	const Ref<EthernetPeer> &A	=>	Computador A
	 * 				const Ref<EthernetPeer> &B	=>	Computador B
	 * 
	 * Retorno: void
	 */
    static void disconnect(const Ref<EthernetPeer> &A, const Ref<EthernetPeer> &B);

    /**
	 * Método que simula o envio de um frame pela interface
	 */
    virtual void sendFrame(uint16_t interface, Ether2Frame &frame);
    /**
	 * Método que simula o recebimento de uma frame pela interface
	 */
    virtual void receiveFrame(const EthernetPeer *const sender_ptr, Ether2Frame &frame) = 0;
    virtual ~EthernetPeer() {}
};

class Host : public EthernetPeer
{
private:
    bool m_PromiscuousMode = false;

public:
    MAC m_MAC;

    /**
	 * Método que simula o envio de um frame pela interface
	 */
    virtual void receiveFrame(const EthernetPeer *const sender_ptr, Ether2Frame &frame) override;

    void setPromiscuousMode(bool promiscuous);

    Host(const MAC &mac, ERROR_CONTROL error_control_type, unsigned int port_count = 1);
};

struct SwitchTableEntry
{
    uint16_t interface;
    uint64_t lastUpdate;
};

using SwitchTable = std::unordered_map<MAC, SwitchTableEntry>;

class Switch : public EthernetPeer
{
private:
    const uint8_t MAX_TABLE_SIZE = 4;

    SwitchTable m_SwitchTable;

    /**
	 * Método que simula o envio de um frame a todos as interfaces conectadas
	 */
    void sendToAllExceptSender(uint16_t senderInterface, Ether2Frame &frame);

    /**
	 * Método devolve a interface do remetente
	 */
    size_t getSenderInterface(const EthernetPeer *const sender_ptr);

public:
    /**
	 * Método que simula o envio de um frame pela interface
	 */
    virtual void receiveFrame(const EthernetPeer *const sender_ptr, Ether2Frame &frame) override;

    Switch(ERROR_CONTROL error_control_type, unsigned int port_count = 32);
};