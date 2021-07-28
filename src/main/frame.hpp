/**
 * Header criado para auxiliar na abstração do funcionamento de um frame na rede
 */
#include <iostream>
#include <stdint.h>
#include <string.h>

#include "mac.hpp"
#include "crc_32.hpp"
#include "tui.hpp"

using namespace tui::text_literals;

struct Ether2Frame
{
    uint64_t
        dst : 48,
        src : 48,
        type : 16;

    //TODO: comentar uso de 1500 bytes (limitado pela linguagem (alternativa é usar a heap, mas perde fidedignidade))
    uint8_t data[1500];
    uint32_t verifyContent;

	/**
	 * Construtor da classe Ether2Frame, já settando o tipo de checagem a ser feita (CRC, paridade par, paridade ímpar)
	 */
    Ether2Frame(const MAC &dst, const MAC &src, const char *const data, size_t data_size, ERROR_CONTROL errorType)
        : dst(dst.bytes), src(src.bytes)
    {
        memset(this->data, '\0', 1500);
        memcpy(this->data, data, data_size);
		if(errorType == ERROR_CONTROL::CRC)
		{
        	verifyContent = CRC32(this->data, 1500);
		}
		else if(errorType == ERROR_CONTROL::EVEN)
		{
			verifyContent = paridadePar((void*)this->data, 1500);
		}
		else
		{
			verifyContent = paridadeImpar((void*)this->data, 1500);
		}
    }

public:
	/**
	 * Método auxiliar para imprimir na tela dados do frame
	 */
    void prettyPrint()
    {
        std::cout << " [ dst: " << std::hex << std::setfill('0') << std::setw(2) << (uint64_t)dst;
        std::cout << " | src: " << std::hex << std::setfill('0') << std::setw(2) << (uint64_t)src;
        std::cout << " | type: " << std::setw(2) << type;
        std::cout << " | payload: " << data;
        std::cout << " | verificador: " << verifyContent;
        std::cout << " ] " << std::endl;
    }

	/**
	 * Método que simula um ruído na transmissão do frame, ocasionando na alteração do valor de um bit aleatório
	 * 
	 * Parâmetros: float probability		=>	Probabilidade da alteração ocorrer
	 * 				size_t randomize_below	=>	Valor máximo da posição para se alterar um bit (para efeitos de visualização)
	 * 
	 * Retorno: void
	 */
    void _simulation_fake_noise(float probability = 0.1, size_t randomize_below = -1)
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

	/**
	 * Método de checagem se a verificação do CRC corresponde com o esperado
	 * 
	 * Parâmetros: void
	 * 
	 * Return: bool	=>	true - conteúdo íntegro
	 * 					false - conteúdo alterado
	 */
    bool checkCRC()
    {
        return verifyContent == CRC32(this->data, 1500);
    }

	/**
	 * Método de checagem se a verificação do bit de paridade par corresponde com o esperado
	 * 
	 * Parâmetros: void
	 * 
	 * Return: bool	=>	true - conteúdo íntegro
	 * 					false - conteúdo alterado
	 */
	bool checkEven()
	{
		return verifyContent == paridadePar((void*)this->data, 1500);
	}

	/**
	 * Método de checagem se a verificação do bit de paridade ímpar corresponde com o esperado
	 * 
	 * Parâmetros: void
	 * 
	 * Return: bool	=>	true - conteúdo íntegro
	 * 					false - conteúdo alterado
	 */
	bool checkOdd()
	{
		return verifyContent == paridadeImpar((void*)this->data, 1500);
	}
};