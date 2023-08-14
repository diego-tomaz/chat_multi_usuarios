#include "common.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>

int contador_usuario = 0;
int usuarios[LIMITE_USUARIOS];

struct dados_cliente {
    int socket_cliente;
    struct sockaddr_storage armazenamento;
};

void enviar_broadcast(char *mensagem) {
    char *idMensagem = NULL;
    char *idRemetente = NULL;
    char *idDestinatario = NULL;
    char *conteudoMensagem = NULL;

    desconstroiMsg(mensagem, &idMensagem, &idRemetente, &idDestinatario, &conteudoMensagem);

    if (atoi(idMensagem) == 6 && atoi(idDestinatario) == 0) {
        time_t tempo;
        struct tm *horario;
        time(&tempo);
        horario = localtime(&tempo);

        printf("[%02d:%02d] %02d: %s\n", horario->tm_hour, horario->tm_min, atoi(idRemetente), conteudoMensagem);
    }

    for (int i = 0; i < LIMITE_USUARIOS; i++) {
        int socket_cliente = usuarios[i];
        if (socket_cliente != 0) {
            send(socket_cliente, mensagem, TAMANHO_BUFFER, 0);
        }
    }
}

int REQ_ADD(int socket_cliente) {
    char enviarMsg[TAMANHO_BUFFER];
    if (contador_usuario < LIMITE_USUARIOS) {
        int id_usuario = 0;
        for (int i = 0; i < LIMITE_USUARIOS; i++) {
            if (usuarios[i] == 0) {
                usuarios[i] = socket_cliente;
                contador_usuario++;
                id_usuario = i + 1;
                break;
            }
        }
        printf("User %02d added\n", id_usuario);
        char msg[TAMANHO_BUFFER - 10];
        sprintf(msg, "User %02d joined the group!", id_usuario);
        sprintf(enviarMsg, "06 %02d %02d %s", id_usuario, id_usuario, msg);
        enviar_broadcast(enviarMsg);
    } else {
        sprintf(enviarMsg, "07 00 00 01");
        send(socket_cliente, enviarMsg, TAMANHO_BUFFER, 0);
        return 0;
    }
    return 1;
}

void REQ_REM(int socket_cliente, int idRemetente) {
    usuarios[idRemetente - 1] = 0;
    printf("User %02d removed\n", idRemetente);
    contador_usuario--;
    char enviarMsg[TAMANHO_BUFFER];
    sprintf(enviarMsg, "08 %02d 00 01", idRemetente);
    send(socket_cliente, enviarMsg, TAMANHO_BUFFER, 0);
    enviar_broadcast(enviarMsg);
}

void RES_LIST(int socket_cliente) {
    char lista[TAMANHO_BUFFER - 10];
    lista[0] = '\0';

    for (int i = 0; i < LIMITE_USUARIOS; i++) {
        if (socket_cliente != usuarios[i] && usuarios[i] > 0) {
            char num[3];
            snprintf(num, sizeof(num), "%02d", i + 1);
            strcat(lista, num);
            strcat(lista, " ");
        }
    }
    char enviarMsg[TAMANHO_BUFFER];
    sprintf(enviarMsg, "04 00 00 %s", lista);
    send(socket_cliente, enviarMsg, TAMANHO_BUFFER, 0);
}

void *thread_cliente(void *data) {
    struct dados_cliente *dCliente = (struct dados_cliente *)data;
    int socket_cliente = dCliente->socket_cliente;

    char buf[TAMANHO_BUFFER];
    memset(buf, 0, TAMANHO_BUFFER);

    char *idMensagem = NULL;
    char *idRemetente = NULL;
    char *idDestinatario = NULL;
    char *mensagem = NULL;

    while (recv(socket_cliente, buf, TAMANHO_BUFFER, 0) > 0) {
        desconstroiMsg(buf, &idMensagem, &idRemetente, &idDestinatario, &mensagem);
        int id = atoi(idMensagem);
        if (id == 1) {
            int  added = REQ_ADD(socket_cliente);
            if (added == 0) {
                break;
            }
        } else if (id == 2) {
            REQ_REM(socket_cliente, atoi(idRemetente));
            break;
        } else if (id == 4) {
            RES_LIST(socket_cliente);
        } else if (id == 6) {
            if (atoi(idDestinatario) == 0) {
                enviar_broadcast(buf);
            } else {
                int socket_receptor = usuarios[atoi(idDestinatario) - 1];
                if (socket_receptor == 0)
                {
                    printf("User %s not found\n", idDestinatario);
                    strcpy(buf, "07 00 00 03");
                    send(socket_cliente, buf, TAMANHO_BUFFER, 0);
                } else {
                    send(socket_cliente, buf, TAMANHO_BUFFER, 0);
                    send(socket_receptor, buf, TAMANHO_BUFFER, 0);
                }
            }
        }

        memset(buf, 0, TAMANHO_BUFFER);
    }

    close(dCliente->socket_cliente);
    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv) {
    struct sockaddr_storage armazenamento;
    inicializarEndServidor(argv[1], argv[2], &armazenamento);
    int s;
    s = socket(armazenamento.ss_family, SOCK_STREAM, 0);
    int habilitar = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &habilitar, sizeof(int));
    struct sockaddr *endereco = (struct sockaddr *)&armazenamento;
    bind(s, endereco, sizeof(armazenamento));
    listen(s, 10);
    char addstr[TAMANHO_BUFFER];
    enderecoParaString(endereco, addstr, TAMANHO_BUFFER);

    memset(usuarios, 0, sizeof(usuarios));

    while (1) {
        struct sockaddr_storage armazenamento_cliente;
        struct sockaddr *endereco_cliente = (struct sockaddr *)&armazenamento_cliente;
        socklen_t tamanho_endereco_cliente = sizeof(armazenamento_cliente);
        int socket_cliente = accept(s, endereco_cliente, &tamanho_endereco_cliente);
        struct dados_cliente *dCliente = malloc(sizeof(*dCliente));
        dCliente->socket_cliente = socket_cliente;
        memcpy(&(dCliente->armazenamento), &armazenamento_cliente, sizeof(armazenamento_cliente));
        pthread_t idThread;
        pthread_create(&idThread, NULL, thread_cliente, dCliente);
    }
    exit(EXIT_SUCCESS);
}
