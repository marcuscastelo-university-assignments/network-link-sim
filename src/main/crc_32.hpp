/**
 * Header auxiliar que contém as definições das checagens de consistência da mensagem enviada
 * 
 * Possui 3 (três) tipos de checage,:
 * 	- CRC-32
 *  - Paridade de bits par
 *  - Paridade de bits impar
 */
#pragma once

#include <iostream>
#include <cstring>

typedef uint32_t crc;

/****************************************** CRC-32 ****************************************/

/**
 * Para um dado valor de entrada e um polinômio gerador,
 * o valor do resto é sempre o mesmo.
 * Por isso, é possível pré-computar o resto para cada
 * input de qualquer tamanho e armazenar em uma tabela
*/

/**
 * Método que gera a tabela para os valores de CRC.
 * 
 * Parâmetros: uint32_t (&table)	=>	Tabela passada por referência para calcular os valores de CRC possíveis
 * 
 * Retorno: void
 */
void generate_table(uint32_t(&table)[1500]);

/**
 * Método que gera um valor CRC de acordo com os parâmetros passados
 * 
 * Parâmetros:	uint32_t (&table)	=>	Tabela passada por referência para calculas os valores de CRC possíveis
 * 				uint32_t intial		=>	Valor inicial para contagem do CRC
 * 				const void* buf		=>	Buffer do conteúdo enviado para gerar o CRC
 * 				size_t len			=>	Tamanho do conteúdo passado
 * 
 * Retorno:	uint32_t	=>	Valor do CRC gerado
 */
uint32_t update(uint32_t(&table)[1500], uint32_t initial, const void* buf, size_t len);

/**
 * Método que faz uso dos métodos acima para retornar um valor de CRC
 * 
 * Parâmetros:	const void* buf	=>	Buffer do conteúdo enviado para gerar o CRC
 * 				size_t len		=>	Tamanho do conteúdo passado
 * 
 * Retorno:	uint32_t	=>	CRC gerado
 */
uint32_t CRC32(const void* buf, size_t len);


/**************************************************** Paridade **********************************/

/**
 * Método auxiliar que retorna a quantidade de bits '1' contida no buffer dado
 */
unsigned int countBits(const void* buf, size_t len);

/**
 * Método que retorna o valor de bit de paridade par
 * 
 * Parâmetros:	const void* buf	=>	Buffer do conteúdo enviado para gerar o CRC
 * 				size_t len		=>	Tamanho do conteúdo passado
 * 
 * Retorno:	uint32_t	=>	bit de paridade (0 ou 1)
 */
uint8_t paridadePar (const void* buf, size_t len);

/**
 * Método que retorna o valor de bit de paridade impar
 * 
 * Parâmetros:	const void* buf	=>	Buffer do conteúdo enviado para gerar o CRC
 * 				size_t len		=>	Tamanho do conteúdo passado
 * 
 * Retorno:	uint32_t	=>	bit de paridade (0 ou 1)
 */
uint8_t paridadeImpar(const void* buf, size_t len);
