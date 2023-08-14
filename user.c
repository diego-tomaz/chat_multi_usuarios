#include "common.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int s;
int meuId = 0;
int usuarios[LIMITE_USUARIOS];

int tratamentoMensagem(char * buf) {
    char * idMsg = NULL;
    char * idRemetente = NULL;
    char * idDestinatario = NULL;
    char * mensagem = NULL;

    desconstroiMsg(buf, &idMsg, &idRemetente, &idDestinatario, &mensagem);
    int id = atoi(idMsg);
    int destinatario = atoi(idDestinatario);
    int remetente = atoi(idRemetente);
    if (id == 6) {
        if (remetente == destinatario) {
            if (meuId == 0) {
                meuId = remetente;
            }
            usuarios[remetente - 1] = remetente;
        } else {
            time_t tempo;
            struct tm * horario;
            time( & tempo);
            horario = localtime( & tempo);
            if (destinatario == 0) {
                char cpyMsg[TAMANHO_BUFFER];
                strcpy(cpyMsg, mensagem);
                if (remetente == meuId)
                    sprintf(mensagem, "[%02d:%02d] -> all: %s", horario -> tm_hour, horario -> tm_min, cpyMsg);
                else
                    sprintf(mensagem, "[%02d:%02d] %02d: %s", horario -> tm_hour, horario -> tm_min, remetente, cpyMsg);
            } else {
                char cpyMsg[TAMANHO_BUFFER];
                strcpy(cpyMsg, mensagem);
                if (remetente == meuId)
                    sprintf(mensagem, "P [%02d:%02d] -> %02d: %s", horario -> tm_hour, horario -> tm_min, destinatario, cpyMsg);
                else
                    sprintf(mensagem, "P [%02d:%02d] %02d: %s", horario -> tm_hour, horario -> tm_min, remetente, cpyMsg);
            }
        }
        puts(mensagem);
    } else if (id == 4) {

        if (mensagem == NULL) {
            printf("%02d\n", meuId);
        } else {
            puts(mensagem);
        }
    } else if (id == 7) {
        int erro = atoi(mensagem);
        if (erro == 1) {
            puts("User limit exceeded");
            return 1;
        } else if (erro == 3) {
            puts("Receiver not found");
        }
    } else if (id == 8) {
        if (meuId == remetente) {
            puts("Removed Successfully");
            return 1;
        } else {
            printf("User %02d left the group!\n", remetente);
        }
    }
    return 0;
}

void * receiveThread(void * arg) {
    char buf[TAMANHO_BUFFER];
    memset(buf, 0, TAMANHO_BUFFER);

    while (recv(s, buf, TAMANHO_BUFFER, 0) > 0) {
        int exitThread = tratamentoMensagem(buf);
        memset(buf, 0, TAMANHO_BUFFER);
        if (exitThread == 1) pthread_exit(NULL);
    }
    close(s);
    return NULL;
}

int main(int argc, char ** argv) {
    struct sockaddr_storage armazenamento;
    analisarEndereco(argv[1], argv[2], & armazenamento);
    s = socket(armazenamento.ss_family, SOCK_STREAM, 0);
    struct sockaddr * endereco = (struct sockaddr * )( & armazenamento);
    connect(s, endereco, sizeof(armazenamento));
    char buf[TAMANHO_BUFFER];
    memset(buf, 0, TAMANHO_BUFFER);
    strcpy(buf, "01");
    send(s, buf, TAMANHO_BUFFER, 0);

    pthread_t recv_thread;
    pthread_create( & recv_thread, NULL, receiveThread, NULL);

    while (1) {
        fgets(buf, TAMANHO_BUFFER - 1, stdin);

        if (strncmp(buf, "close connection", 16) == 0) {
            char msg[TAMANHO_BUFFER];
            sprintf(msg, "02 %02d", meuId);
            send(s, msg, TAMANHO_BUFFER, 0);

            pthread_join(recv_thread, NULL);

            close(s);
            exit(EXIT_SUCCESS);
        } else if (strncmp(buf, "list users", 10) == 0) {
            char msg[TAMANHO_BUFFER];
            sprintf(msg, "04");
            send(s, msg, TAMANHO_BUFFER, 0);
        } else if (strncmp(buf, "send all", 8) == 0) {
            char mensagem[TAMANHO_BUFFER - 10];
            char sendmsg[TAMANHO_BUFFER];
            sscanf(buf, "send all \"%[^\"]\"", mensagem);
            sprintf(sendmsg, "06 %02d 00 %s", meuId, mensagem);
            send(s, sendmsg, TAMANHO_BUFFER, 0);
        } else if (strncmp(buf, "send to", 7) == 0) {
            char idDestinatario[3];
            char mensagem[TAMANHO_BUFFER - 10];
            char sendmsg[TAMANHO_BUFFER];
            sscanf(buf, "send to %2s \"%[^\"]\"", idDestinatario, mensagem);
            if (atoi(idDestinatario) != meuId) {
                sprintf(sendmsg, "06 %02d %s %s", meuId, idDestinatario, mensagem);
                send(s, sendmsg, TAMANHO_BUFFER, 0);
            }
        }
    }
    close(s);
    exit(EXIT_SUCCESS);
}