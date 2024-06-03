#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#define int_usado uint32_t
#define SIZE_INT sizeof(int_usado)

typedef struct {
	int_usado *array;
	size_t nmemb;

} big_int;

void inicializar(big_int *bigInt, size_t nmemb);
void freeInt(big_int *bigInt);
void atribuirValor(int_usado inteiro, big_int *bigInt, size_t pos);
void printIntHexa(big_int *bigInt);
void copiar(big_int *int1, big_int *int2);
void incrementar1(big_int *bigInt);
void decrementar1(big_int *bigInt);
size_t checaTamanho(big_int *int1, big_int *int2, big_int *int3);
bool eZero(big_int *bigInt);
void inverter(big_int *bigInt);
void complemento2(big_int *bigInt);
void somar(big_int *int1, big_int *int2, big_int *resultado);
void subtrair(big_int *int1, big_int *int2, big_int *resultado);
void multiplicar(big_int *int1, big_int *int2, big_int *resultado);
//void dividir(big_int *int1, big_int *int2, big_int *resultado, big_int *resto);
