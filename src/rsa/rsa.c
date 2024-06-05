#include "rsa.h"
#include "../bigint/bigint.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define TAM_BLOCO_BITS 64
#define TAM_BLOCO TAM_BLOCO_BITS / (sizeof(unsigned char) * 8)  // 64 bits -> 8 caracteres

typedef struct structRsa {
    uint32_t numero_bits;
    big_int p;
    big_int q;
    big_int n;
    big_int e;
    big_int d;
    big_int phi;
} Rsa;

typedef struct structpublicKey {
    uint32_t numero_bits;
    big_int e;
    big_int n;
} publicKey;

void modinv(Rsa *rsa);

Rsa * criarKeys(uint32_t numero_bits) {
    big_int phi_p, phi_q;
    size_t nmemb;
    Rsa *rsa;
    
    
    rsa = (Rsa*) malloc(sizeof(Rsa));
    if (rsa == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    rsa->numero_bits = numero_bits;

    nmemb = numero_bits/(SIZE_INT*8);

    // Inicializando os big_ints do Rsa
    inicializar(&rsa->p, nmemb);
    inicializar(&rsa->q, nmemb);
    inicializar(&rsa->n, nmemb);
    inicializar(&rsa->e, nmemb);
    inicializar(&rsa->d, nmemb);
    inicializar(&rsa->phi, nmemb);
    // Inicializando os big_ints phi(p) e phi(q)
    inicializar(&phi_p, nmemb);
    inicializar(&phi_q, nmemb);

    // Temporário, mudar para uma funcao geradora de números aleatórios depois!!!!!!!!!!!!!!
    atribuirValor(3808879747, &rsa->p, 0);
    atribuirValor(3336254773, &rsa->q, 0);
    // Temporário, mudar para uma funcao geradora de números aleatórios depois!!!!!!!!!!!!!!

    // Cálculo do n
    multiplicar(&rsa->p, &rsa->q, &rsa->n);
    

    // Calculo do phi(p) e phi(q)
    copiar(&phi_p, &rsa->p);
    copiar(&phi_q, &rsa->q);
    decrementar1(&phi_p);
    decrementar1(&phi_q);

    // Calculo do phi(n)
    multiplicar(&phi_p, &phi_q, &rsa->phi);

    // e = 2^16 + 1 = 65537
    //atribuirValor(65537, &rsa->e, 0);
    atribuirValor(65537, &rsa->e, 0);
    

    // Temporário, mudar para uma funcao que calcula o d!!!!!!!!!!!!!!
    modinv(rsa);
    //atribuirValor(791, &rsa->d, 0);
    // Temporário, mudar para uma funcao que calcula o d!!!!!!!!!!!!!!

    printf("p = ");
    printIntHexa(&rsa->p);
    printf("\n");
    printf("q = ");
    printIntHexa(&rsa->q);
    printf("\n");
    printf("n = ");
    printIntHexa(&rsa->n);
    printf("\n");
    printf("d = ");
    printIntHexa(&rsa->d);
    printf("\n");
    printf("e = ");
    printIntHexa(&rsa->e);
    printf("\n");
    printf("phi = ");
    printIntHexa(&rsa->phi);
    printf("\n");

    return rsa;
}

publicKey * getPubKey(Rsa *rsa) {
    size_t nmemb;
    publicKey *pubKey;

    pubKey = (publicKey*) malloc(sizeof(publicKey));
    if (pubKey == NULL) {
        perror("malloc");
        freeRsa(rsa);
        exit(EXIT_FAILURE);
    }

    nmemb = rsa->numero_bits/(SIZE_INT*8);

    // Criação da publicKey
    pubKey->numero_bits = rsa->numero_bits;

    inicializar(&pubKey->n, nmemb);
    inicializar(&pubKey->e, nmemb);

    copiar(&pubKey->n, &rsa->n);
    copiar(&pubKey->e, &rsa->e);

    return pubKey;
}

void freeRsa(Rsa *rsa) {
    freeInt(&rsa->p);
    freeInt(&rsa->q);
    freeInt(&rsa->n);
    freeInt(&rsa->d);
    freeInt(&rsa->e);
    freeInt(&rsa->phi);

    free(rsa);
}

void freePubKey(publicKey *pubKey) {
    freeInt(&pubKey->n);
    freeInt(&pubKey->e);
    
    free(pubKey);
}

void freeCipher(unsigned char *cipher) {
    free(cipher);
}


unsigned char * encrypt(unsigned char *message, Rsa *rsa) {
    big_int intMessage, intCipher, auxiliar;
    size_t tamanhoBig, tamanho_mensagem, tamanhoBytes;
    unsigned char *cipher;
    uint32_t auxiliar2;

    tamanhoBig = rsa->numero_bits / (SIZE_INT*8);
    tamanhoBytes = rsa->numero_bits / 8;
    tamanho_mensagem = strlen((char*)message);

    if (tamanho_mensagem > tamanhoBytes) {
        fprintf(stderr, "Mensagem não pode exceder o tamanho de %zu\n", tamanhoBytes);
        return NULL;
    }

    cipher = (unsigned char*) malloc(tamanhoBytes * sizeof(unsigned char));
    if (cipher == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    inicializar(&intMessage, tamanhoBig);
    inicializar(&intCipher, tamanhoBig);
    inicializar(&auxiliar, tamanhoBig);
    

    // Coloca a mensagem como big_int
    for(size_t i = 0; i < tamanho_mensagem; i++) {
        // Shiftar 8 bits a direita -> 1 caractere
        for(size_t j = 0; j < 8; j++) {
            dobrar(&intMessage);
        }
        atribuirValor((uint8_t)message[i], &auxiliar, 0);
        somar(&intMessage, &auxiliar, &intMessage);
    }

    // Faz a mágia do RSA
    bigPowMod(&intMessage, &rsa->e, &rsa->n, &intCipher);

    // Coloca o cipher para string
    for(size_t i = 0; i < tamanhoBytes; i++) {
        auxiliar2 = (intCipher.array[tamanhoBig-1]) >> 24;
        cipher[i] = (unsigned char)auxiliar2;  // Pega os últimos 8 bits do intCipher

        // Shiftar 8 bits a direita -> 1 caractere
        for(size_t j = 0; j < 8; j++) {
            dobrar(&intCipher);
        }
    }

    freeInt(&intMessage);
    freeInt(&intCipher);
    freeInt(&auxiliar);

    return cipher;
}


unsigned char * decrypt(unsigned char *cipher, Rsa *rsa) {
    big_int intMessage, intCipher, auxiliar;
    size_t tamanhoBig, tamanhoBytes, tamanho_mensagem;
    uint32_t auxiliar2;
    unsigned char *mensagemTemp, *mensagem;

    tamanhoBytes = rsa->numero_bits / 8;
    tamanhoBig = rsa->numero_bits / (SIZE_INT*8);

    inicializar(&intMessage, tamanhoBig);
    inicializar(&intCipher, tamanhoBig);
    inicializar(&auxiliar, tamanhoBig);

    mensagemTemp = (unsigned char*) malloc(tamanhoBytes * sizeof(unsigned char));
    if (mensagemTemp == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }


    for(size_t i = 0; i < tamanhoBytes; i++) {
        // Shiftar 8 bits a direita -> 1 caractere
        for(size_t j = 0; j < 8; j++) {
            dobrar(&intCipher);
        }

        atribuirValor((uint32_t) cipher[i], &auxiliar, 0);
        somar(&intCipher, &auxiliar, &intCipher);
    }

    // Faz a mágia do RSAintCipher
    bigPowMod(&intCipher, &rsa->d, &rsa->n, &intMessage);

    // Coloca o message para string
    for(size_t i = 0; i < tamanhoBytes; i++) {
        auxiliar2 = intMessage.array[tamanhoBig-1] >> 24;
        mensagemTemp[i] = (unsigned char)auxiliar2;  // Pega os últimos 8 bits do intMessage

        // Shiftar 8 bits a direita -> 1 caractere
        for(size_t j = 0; j < 8; j++) {
            dobrar(&intMessage);
        }
    }

    tamanho_mensagem = tamanhoBytes;

    for(size_t i = 0; mensagemTemp[i] == '\0'; i++) {
        tamanho_mensagem--; 
    }

    mensagem = (unsigned char*) malloc(tamanho_mensagem * sizeof(unsigned char) + 1);
    if (mensagem == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }


    for(size_t i = 0; i < tamanho_mensagem; i++) {
        mensagem[tamanho_mensagem-1-i] = mensagemTemp[tamanhoBytes-1-i];
    }
    mensagem[tamanho_mensagem] = '\0';

    free(mensagemTemp);
    freeInt(&intMessage);
    freeInt(&intCipher);
    freeInt(&auxiliar);

    return mensagem;
}

void modinv(Rsa *rsa) {
    big_int a, b, q, temp_a, temp_b, temp_x0, temp_y0, x0, x1, y0, y1, auxiliar;
    size_t tamanho;
    bool x1Negativo, y1Negativo, x0Negativo, y0Negativo, temp_x0Negativo, temp_y0Negativo;
    comparacao c;

    tamanho = rsa->n.nmemb;

    inicializar(&a, tamanho);
    inicializar(&b, tamanho);
    inicializar(&q, tamanho);
    inicializar(&temp_a, tamanho);
    inicializar(&temp_b, tamanho);
    inicializar(&temp_x0, tamanho);
    inicializar(&temp_y0, tamanho);
    inicializar(&x0, tamanho);
    inicializar(&x1, tamanho);
    inicializar(&y0, tamanho);
    inicializar(&y1, tamanho);
    inicializar(&auxiliar, tamanho);

    atribuirValor(1, &x0, 0);
    atribuirValor(0, &x1, 0);
    atribuirValor(0, &y0, 0);
    atribuirValor(1, &y1, 0);

    copiar(&a, &rsa->e);
    copiar(&b, &rsa->phi);

    x1Negativo = false;
    y1Negativo = false;
    x0Negativo = false;
    y0Negativo = false;
    temp_x0Negativo = false;
    temp_y0Negativo = false;

    int i = 0;

    printf("a = ");
    printIntHexa(&a);
    printf("\n");
    printf("b = ");
    printIntHexa(&b);
    printf("\n");
    printf("q = ");
    printIntHexa(&q);
    printf("\n");
    printf("x0 = ");
    printIntHexa(&x0);
    printf("\n");
    printf("x1 = ");
    printIntHexa(&x1);
    printf("\n");
    if (x1Negativo) printf("negativo\n");
    printf("y0 = ");
    printIntHexa(&y0);
    printf("\n");
    printf("y1 = ");
    printIntHexa(&y1);
    printf("\n");
    if (y1Negativo) printf("negativo\n");
    
    while(!eZero(&b)) {
        //printf("i = %d", ++i);
        dividir(&a, &b, &q, &auxiliar);
        printf("oi\n");

        copiar(&temp_a, &a);
        copiar(&temp_b, &b);
        copiar(&a, &temp_b);
        dividir(&temp_a, &temp_b, &auxiliar, &b);

        copiar(&temp_x0, &x0);
        temp_x0Negativo = x0Negativo;

        copiar(&x0, &x1);
        x0Negativo = x1Negativo;

        
        multiplicar(&q, &x1, &auxiliar);
        c = compara(&temp_x0, &auxiliar);
        
        if (c == MENOR) {
            printf("MENOR ");
            if (!x1Negativo && !temp_x0Negativo) {  // POS < POS
                subtrair(&auxiliar, &temp_x0, &x1);
                x1Negativo = true;
                printf("NEGATIVOa\n");
            } else if (x1Negativo && temp_x0Negativo) {  // NEG < NEG
                subtrair(&auxiliar, &temp_x0, &x1);
                x1Negativo = false;
                printf("POSITIVOa\n");
            } else if (x1Negativo && !temp_x0Negativo) {  // NEG < POS
                somar(&auxiliar, &temp_x0, &x1);
                x1Negativo = false;
                printf("NEGATIVO\n");
            } else {                                    // POS < NEG
                somar(&auxiliar, &temp_x0, &x1);
                x1Negativo = true;
                printf("POSITIVO\n");
            }
        } else if (c == MAIOR){
            printf("MAIOR ");
            if (!x1Negativo && !temp_x0Negativo) {  // POS > POS
                subtrair(&temp_x0, &auxiliar, &x1);
                x1Negativo = false;
                printf("POSITIVO\n");
            } else if (x1Negativo && temp_x0Negativo) {  // NEG > NEG
                subtrair(&temp_x0, &auxiliar, &x1);
                x1Negativo = true;
                printf("NEGATIVO\n");
            } else if (x1Negativo && !temp_x0Negativo) {  // NEG > POS
                somar(&auxiliar, &temp_x0, &x1);
                x1Negativo = false;
                printf("NEGATIVO\n");
            } else {                                    // POS > NEG
                somar(&auxiliar, &temp_x0, &x1);
                x1Negativo = true;
                printf("POSITIVO\n");
            }
        } else {
            printf("IGUAL ");
            if (!x1Negativo && !temp_x0Negativo) {  // POS = POS
                subtrair(&temp_x0, &auxiliar, &x1);
                x1Negativo = false;
            } else if (x1Negativo && temp_x0Negativo) {  // NEG = NEG
                subtrair(&temp_x0, &auxiliar, &x1);
                x1Negativo = false;
            } else if (x1Negativo && !temp_x0Negativo) {  // NEG = POS
                somar(&auxiliar, &temp_x0, &x1);
                x1Negativo = true;
            } else {                                    // POS = NEG
                somar(&auxiliar, &temp_x0, &x1);
                x1Negativo = false;
            }
        }


        copiar(&temp_y0, &y0);
        temp_y0Negativo = y0Negativo;

        copiar(&y0, &y1);
        y0Negativo = y1Negativo;

        
        multiplicar(&q, &y1, &auxiliar);

        c = compara(&temp_y0, &auxiliar);
        if (c == MENOR) {
            printf("MENOR ");
            if (!y1Negativo && !temp_y0Negativo) {  // POS < POS
                subtrair(&auxiliar, &temp_y0, &y1);
                y1Negativo = true;
                printf("NEGATIVOa\n");
            } else if (y1Negativo && temp_y0Negativo) {  // NEG < NEG
                subtrair(&auxiliar, &temp_y0, &y1);
                y1Negativo = false;
                printf("POSITIVOa\n");
            } else if (y1Negativo && !temp_y0Negativo) {  // NEG < POS
                somar(&auxiliar, &temp_y0, &y1);
                y1Negativo = false;
                printf("NEGATIVO\n");
            } else {                                    // POS < NEG
                somar(&auxiliar, &temp_y0, &y1);
                y1Negativo = true;
                printf("POSITIVO\n");
            }
        } else if (c == MAIOR){
            printf("MAIOR ");
            if (!y1Negativo && !temp_y0Negativo) {  // POS > POS
                subtrair(&temp_y0, &auxiliar, &y1);
                y1Negativo = false;
                printf("POSITIVO\n");
            } else if (y1Negativo && temp_y0Negativo) {  // NEG > NEG
                subtrair(&temp_y0, &auxiliar, &y1);
                y1Negativo = true;
                printf("NEGATIVO\n");
            } else if (y1Negativo && !temp_y0Negativo) {  // NEG > POS
                somar(&auxiliar, &temp_y0, &y1);
                y1Negativo = true;
                printf("NEGATIVO\n");
            } else {                                    // POS > NEG
                somar(&auxiliar, &temp_y0, &y1);
                y1Negativo = false;
                printf("POSITIVO\n");
            }
        } else {
            printf("IGUAL ");
            if (!y1Negativo && !temp_y0Negativo) {  // POS = POS
                subtrair(&temp_y0, &auxiliar, &y1);
                y1Negativo = false;
            } else if (y1Negativo && temp_y0Negativo) {  // NEG = NEG
                subtrair(&temp_y0, &auxiliar, &y1);
                y1Negativo = false;
            } else if (y1Negativo && !temp_y0Negativo) {  // NEG = POS
                somar(&auxiliar, &temp_y0, &y1);
                y1Negativo = true;
            } else {                                    // POS = NEG
                somar(&auxiliar, &temp_y0, &y1);
                y1Negativo = false;
            }
        }

        if (i++ < 40){printf("a = ");
        printIntHexa(&a);
        printf("\n");
        printf("b = ");
        printIntHexa(&b);
        printf("\n");
        printf("q = ");
        printIntHexa(&q);
        printf("\n");
        printf("x0 = ");
        printIntHexa(&x0);
        printf("\n");
        if (x0Negativo) printf("negativo\n");
        printf("temp_x0 = ");
        printIntHexa(&temp_x0);
        printf("\n");
        if (temp_x0Negativo) printf("negativo\n");
        printf("x1 = ");
        printIntHexa(&x1);
        printf("\n");
        if (x1Negativo) printf("negativo\n");
        printf("y0 = ");
        printIntHexa(&y0);
        printf("\n");
        if (y0Negativo) printf("negativo\n");
        printf("temp_y0 = ");
        printIntHexa(&temp_y0);
        printf("\n");
        if (temp_y0Negativo) printf("negativo\n");
        printf("y1 = ");
        printIntHexa(&y1);
        printf("\n");
        if (y1Negativo) printf("negativo\n");}
    }

    dividir(&x0, &rsa->phi, &auxiliar, &rsa->d);
    if (x0Negativo) {
        subtrair(&rsa->phi, &rsa->d, &rsa->d);
    }

    if (!eUm(&a)) {
        fprintf(stderr, "O inverso modular não existe\n");
        exit(EXIT_FAILURE);
    }

    freeInt(&a);
    freeInt(&b);
    freeInt(&q);
    freeInt(&temp_a);
    freeInt(&temp_b);
    freeInt(&temp_x0);
    freeInt(&x0);
    freeInt(&temp_y0);
    freeInt(&y0);
    freeInt(&x1);
    freeInt(&y1);
    freeInt(&auxiliar);
}