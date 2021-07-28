/**
 * Header auxiliar para testes de envio de frames entre os computadores
 */
#pragma once

#include "types.hpp"

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
void A_B_ttl_andPromC();

/**
 * Método que simula conexão de computadores A, B e C, com A no Switch S1, B e C no switch S2 e ambos switches conectados
 * 
 * Análogo à função acima, porém a comunicação é entre B e C, com A no modo promíscuo (a topologia é a mesma)
 */
void B_C_self_andPromA();

/**
 * Método que simula um erro na transmissão de um frame do computador B para o C, simulando erro
 * 
 * Topologia mais simples que as funções anteriores: B se comunica com C por meio de um switch
 * 
 * O foco dessa simulação é demonstrar a checagem de erro por meio do método especificado.
 */
void B_C_error(ERROR_CONTROL test_error_control);
