#include <iostream>

using namespace std;

typedef uint32_t crc;

#define mensagem (const uint8_t*) "aaaaaaaaaaaaaaaa"
#define mensagem2 (const uint8_t*) "aaaaaaaaaaaaaaab"

/**
 * Para um dado valor de entrada e um polinômio gerador,
 * o valor do resto é sempre o mesmo.
 * Por isso, é possível pré-computar o resto para cada
 * input de qualquer tamanho e armazenar em uma tabela
*/
void generate_table(uint32_t(&table)[256]){
	uint32_t polynomial = 0xEDB88320;
	for (uint32_t i = 0; i < 256; i++) 
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

uint32_t update(uint32_t (&table)[256], uint32_t initial, const void* buf, size_t len)
{
	uint32_t c = initial ^ 0xFFFFFFFF;
	const uint8_t* u = static_cast<const uint8_t*>(buf);
	for (size_t i = 0; i < len; ++i) 
	{
		c = table[(c ^ u[i]) & 0xFF] ^ (c >> 8);
	}
	return c ^ 0xFFFFFFFF;
}

int CRC32(const char* data, unsigned int size) {
    // TODO: assert size condiz com CRC32 (mensagem + ? bits)

    // constexpr unsigned int polynomial = 0b00000100110000010001110110110111;
    //TODO: padding right (size - 1)
    //TODO: xor com data
    
    //Return resto para colocar no quadro como checksum.
    return 0;
}

int main(){

	uint32_t table[256];
	generate_table(table);

	uint32_t crc = update(table, 0, mensagem, 16);

	cout << crc << endl;

	return 0;
}