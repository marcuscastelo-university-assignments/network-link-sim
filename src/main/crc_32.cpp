#include <iostream>
#include <cstring>

#include "crc_32.hpp"

using namespace std;

typedef uint32_t crc;

/****************************************** CRC-32 ****************************************/

/**
 * Para um dado valor de entrada e um polinômio gerador,
 * o valor do resto é sempre o mesmo.
 * Por isso, é possível pré-computar o resto para cada
 * input de qualquer tamanho e armazenar em uma tabela
*/
void generate_table(uint32_t(&table)[1500]){
	uint32_t polynomial = 0xEDB88320;
	for (uint32_t i = 0; i < 1500; i++) 
	{
		uint32_t c = i;
		for (size_t j = 0; j < 8; j++) 
		{
			if (c & 1) {
				c = polynomial ^ (c >> 1);
			}
			else {
				c >>= 1;
			}
		}
		table[i] = c;
	}
}

uint32_t update(uint32_t (&table)[1500], uint32_t initial, const void* buf, size_t len)
{
	uint32_t c = initial ^ 0xFFFFFFFF;
	const uint8_t* u = static_cast<const uint8_t*>(buf);
	for (size_t i = 0; i < len; ++i) 
	{
		c = table[(c ^ u[i]) & 0xFF] ^ (c >> 8);
	}
	return c ^ 0xFFFFFFFF;
}

uint32_t CRC32(const void* buf, size_t len) {
	uint32_t table[1500];
	generate_table(table);
	return update(table, 0, buf, len);
}

/**************************************************** Paridade **********************************/

unsigned int countBits(const void* buf, size_t len){
	unsigned int num_bits = 0;
	const uint8_t* message = static_cast<const uint8_t*> (buf);
	for(unsigned int i = 0; i < len; i++){
		
		int value = message[i];
		while(value != 0){
			num_bits += value%2;
			value /= 2;
		}
	}

	return num_bits;
}

uint8_t paridadePar (const void* buf, size_t len){

	int num_bits = countBits(buf, len);
	//Par
	if(num_bits%2 == 0)
	{
		return (uint8_t) 0;
	}

	//Impar
	else
	{
		return (uint8_t) 1;
	}
}

uint8_t paridadeImpar(const void* buf, size_t len){

	int num_bits = countBits(buf, len);

	//Par
	if(num_bits%2 == 0)
	{
		return (uint8_t) 1; 
	}

	//Impar
	else
	{
		return (uint8_t) 0;
	}
}
