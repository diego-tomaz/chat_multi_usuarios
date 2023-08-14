#pragma once
#include <arpa/inet.h>
#include <stdlib.h>

#define TAMANHO_BUFFER 2048
#define LIMITE_USUARIOS 15

struct MSG {
    char idMsg[10];
    char idSender[10];
    char idReceiver[10];
    char message[2018];
};

int analisarEndereco(const char * endereco, const char * portaStr, struct sockaddr_storage * armazenamento);

void enderecoParaString(const struct sockaddr * endereco, char * str, size_t tamanhoStr);

int inicializarEndServidor(const char * protocolo, const char * portaStr, struct sockaddr_storage * armazenamento);

char * constroiMsg(char * idMensagem, char * idRemetente, char * idDestinatario, char * conteudoMensagem);

void desconstroiMsg(const char * stringConcatenada, char ** idMensagem, char ** idRemetente, char ** idDestinatario, char ** conteudoMensagem);