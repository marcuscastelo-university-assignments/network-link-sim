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
void generate_table(uint32_t(&table)[256]);
uint32_t update(uint32_t(&table)[256], uint32_t initial, const void* buf, size_t len);

uint32_t CRC32(const void* buf, size_t len);


/**************************************************** Paridade **********************************/

unsigned int countBits(const void* buf, size_t len);
char* paridadeImpar(const char* buf, size_t len);
char* paridadePar (const char* buf, size_t len);
