#include "common.h"
#include <arpa/inet.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char * constroiMsg(char * idMensagem, char * idRemetente, char * idDestinatario, char * conteudoMensagem) {
    char * mensagemConcatenada = malloc(TAMANHO_BUFFER);
    mensagemConcatenada[0] = '\0';

    strcat(mensagemConcatenada, idMensagem);
    strcat(mensagemConcatenada, " ");
    strcat(mensagemConcatenada, idRemetente);
    strcat(mensagemConcatenada, " ");
    strcat(mensagemConcatenada, idDestinatario);
    strcat(mensagemConcatenada, " ");
    strcat(mensagemConcatenada, conteudoMensagem);
    strcat(mensagemConcatenada, " ");

    return mensagemConcatenada;
}

void desconstroiMsg(const char * stringConcatenada, char ** idMensagem, char ** idRemetente, char ** idDestinatario, char ** conteudoMensagem) {
    char * copiaString = strdup(stringConcatenada);
    char * fragmento = strtok(copiaString, " ");

    if (fragmento != NULL) {
        * idMensagem = strdup(fragmento);
        fragmento = strtok(NULL, " ");

        if (fragmento != NULL) {
            * idRemetente = strdup(fragmento);
            fragmento = strtok(NULL, " ");

            if (fragmento != NULL) {
                * idDestinatario = strdup(fragmento);
                fragmento = strtok(NULL, "");

                if (fragmento != NULL) {
                    * conteudoMensagem = strdup(fragmento);
                }
            }
        }
    }

    free(copiaString);
}

int analisarEndereco(const char * endereco, const char * portaStr, struct sockaddr_storage * armazenamento) {
    if (endereco == NULL || portaStr == NULL) {
        return -1;
    }

    uint16_t porta = (uint16_t) atoi(portaStr);
    if (porta == 0) {
        return -1;
    }

    porta = htons(porta);

    struct in_addr enderecoIPv4;
    if (inet_pton(AF_INET, endereco, &enderecoIPv4)) {
        struct sockaddr_in * endereco4 = (struct sockaddr_in * ) armazenamento;
        endereco4 -> sin_family = AF_INET;
        endereco4 -> sin_port = porta;
        endereco4 -> sin_addr = enderecoIPv4;
        return 0;
    }

    struct in6_addr enderecoIPv6;
    if (inet_pton(AF_INET6, endereco, &enderecoIPv6)) {
        struct sockaddr_in6 * endereco6 = (struct sockaddr_in6 * ) armazenamento;
        endereco6 -> sin6_family = AF_INET6;
        endereco6 -> sin6_port = porta;
        memcpy(&endereco6 -> sin6_addr, &enderecoIPv6, sizeof(enderecoIPv6));
        return 0;
    }

    return -1;
}

void enderecoParaString(const struct sockaddr * endereco, char * str, size_t tamanhoStr) {
    int versao;
    char addstr[INET6_ADDRSTRLEN + 1] = "";
    uint16_t porta;

    if (endereco -> sa_family == AF_INET) {
        versao = 4;
        struct sockaddr_in * endereco4 = (struct sockaddr_in * ) endereco;

        if (!inet_ntop(AF_INET, &endereco4 -> sin_addr, addstr, INET6_ADDRSTRLEN + 1)) {
            EXIT_FAILURE;
        }
        porta = ntohs(endereco4 -> sin_port);
    } else if (endereco -> sa_family == AF_INET6) {
        versao = 6;
        struct sockaddr_in6 * endereco6 = (struct sockaddr_in6 * ) endereco;

        if (!inet_ntop(AF_INET6, &endereco6 -> sin6_addr, addstr, INET6_ADDRSTRLEN + 1)) {
            EXIT_FAILURE;
        }
        porta = ntohs(endereco6 -> sin6_port);
    } else {
        EXIT_FAILURE;
    }

    if (str) {
        snprintf(str, tamanhoStr, "IPv%d %s %hu", versao, addstr, porta);
    }
}

int inicializarEndServidor(const char * protocolo, const char * portaStr, struct sockaddr_storage * armazenamento) {
    uint16_t porta = (uint16_t) atoi(portaStr);
    if (porta == 0) {
        return -1;
    }

    porta = htons(porta);

    memset(armazenamento, 0, sizeof(*armazenamento));
    if (0 == strcmp(protocolo, "v4")) {
        struct sockaddr_in * endereco4 = (struct sockaddr_in *) armazenamento;
        endereco4 -> sin_family = AF_INET;
        endereco4 -> sin_addr.s_addr = INADDR_ANY;
        endereco4 -> sin_port = porta;
        return 0;
    } else if (0 == strcmp(protocolo, "v6")) {
        struct sockaddr_in6 * endereco6 = (struct sockaddr_in6 *) armazenamento;
        endereco6 -> sin6_family = AF_INET6;
        endereco6 -> sin6_addr = in6addr_any;
        endereco6 -> sin6_port = porta;
        return 0;
    } else {
        return -1;
    }
}
