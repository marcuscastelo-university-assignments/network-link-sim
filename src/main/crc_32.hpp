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
void generate_table(uint32_t(&table)[1500]);
uint32_t update(uint32_t(&table)[1500], uint32_t initial, const void* buf, size_t len);

uint32_t CRC32(const void* buf, size_t len);


/**************************************************** Paridade **********************************/

unsigned int countBits(const void* buf, size_t len);
uint8_t paridadePar (const void* buf, size_t len);
uint8_t paridadeImpar(const void* buf, size_t len);
