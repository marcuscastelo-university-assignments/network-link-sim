#pragma once

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

	/**
	 * Vetor de bytes do frame ethernet 2: varia de 46 a 1500 bytes.
	 * No C, não é possível fazer um vetor dinâmico 
	 * enquanto se mantém a sequencialidade dos campos do frame 
	 * seria necessário usar algo como a heap e pointers.
	 * 
	 * Então aloca-se o tamanho máximo sempre
	*/
	uint8_t data[1500];
	uint32_t verifyContent;

	/**
	 * Construtor da classe Ether2Frame, já settando o tipo de checagem a ser feita (CRC, paridade par, paridade ímpar)
	 */
	Ether2Frame(const MAC &dst, const MAC &src, const char *const data, size_t data_size, ERROR_CONTROL errorType);

public:
	/**
	 * Método auxiliar para imprimir na tela dados do frame
	 */
	void prettyPrint();

	/**
	 * Método que simula um ruído na transmissão do frame, ocasionando na alteração do valor de um bit aleatório
	 * 
	 * Parâmetros: float probability		=>	Probabilidade da alteração ocorrer
	 * 				size_t randomize_below	=>	Valor máximo da posição para se alterar um bit (para efeitos de visualização)
	 * 
	 * Retorno: void
	 */
	void _simulation_fake_noise(float probability = 0.1, size_t randomize_below = -1);

	/**
	 * Método de checagem se a verificação do CRC corresponde com o esperado
	 * 
	 * Parâmetros: void
	 * 
	 * Return: bool	=>	true - conteúdo íntegro
	 * 					false - conteúdo alterado
	 */
	bool checkCRC();

	/**
	 * Método de checagem se a verificação do bit de paridade par corresponde com o esperado
	 * 
	 * Parâmetros: void
	 * 
	 * Return: bool	=>	true - conteúdo íntegro
	 * 					false - conteúdo alterado
	 */
	bool checkEven();

	/**
	 * Método de checagem se a verificação do bit de paridade ímpar corresponde com o esperado
	 * 
	 * Parâmetros: void
	 * 
	 * Return: bool	=>	true - conteúdo íntegro
	 * 					false - conteúdo alterado
	 */
	bool checkOdd();
};