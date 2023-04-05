#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024

void zeramatriz();
void iniciaOrdenacao();
void separaFrase(char mensagem[BUFSZ]);
void listSensor();
void addSensor();
void imprimeMatriz();
void removeSensor();
int confereSensor(int sensor);
void reordenaEstrutura(int contador);
void readSensor();


char frase[10][20];
int equipamentos[6][5];
char mensagem[100];
int quantas_palavras;
int equipamento;
int sensor;
int total_sensores = 0;
char buffer[200];

typedef struct {
    int qtd;
    char add_ordem[6][5];
} Ordenacao;

Ordenacao Ordem[5];

void usage(int argc, char **argv) {
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) {
        usage(argc, argv);
    }

    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("bind");
    }

    if (0 != listen(s, 10)) {
        logexit("listen");
    }

    zeramatriz(4,4, equipamentos);
    iniciaOrdenacao();

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr);
    struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logexit("accept");
        }
        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);
        printf("[log] connection from %s\n", caddrstr);

        memset(frase, 0, sizeof frase);

    while (1) {
        quantas_palavras = 0;
        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);
        size_t count = recv(csock, buf, BUFSZ - 1, 0);
        printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);

        separaFrase(buf);

        if (strcmp(frase[0],"add") == 0)
        {
            printf("adicionando sensor");
            addSensor();
            memset(buf, 0, BUFSZ);
            strcpy(buf,buffer);
            count = send(csock, buf, strlen(buf) + 1, 0);
            printf("%s", buffer);
        }
        else if(strcmp(frase[0],"remove") == 0)
        {
            removeSensor();
            memset(buf, 0, BUFSZ);
            strcpy(buf,buffer);
            count = send(csock, buf, strlen(buf) + 1, 0);
            printf("%s", buffer);
        }
        else if(strcmp(frase[0],"list") == 0)
        {
            listSensor();
            memset(buf, 0, BUFSZ);
            strcpy(buf,buffer);
            count = send(csock, buf, strlen(buf) + 1, 0);
            printf("%s", buffer);
        }
        else if(strcmp(frase[0],"read") == 0)
        {
            readSensor();
            memset(buf, 0, BUFSZ);
            strcpy(buf,buffer);
            count = send(csock, buf, strlen(buf) + 1, 0);
            printf("%s", buffer);
        }
        else if(strcmp(frase[0], "kill") == 0){
            break;
        }
        else {
            break;
        }
        memset(frase, 0, sizeof frase);
        quantas_palavras = 0;
        imprimeMatriz();
    }
    close(csock);

    exit(EXIT_SUCCESS);
}

void zeramatriz(){
    for (int i = 0; i <= 4; i++)
    {
        for (int j = 0; j <= 4; j++)
        {
            equipamentos[i][j] = 0;
        }
    }
    return;
};

void iniciaOrdenacao(){
    for(int i = 0; i<=4; i++){
        Ordem[i].qtd = 0;
    }
    return;
};

void separaFrase(char mensagem[BUFSZ]){
    char *pt;
    pt = strtok(mensagem, " ");
    while(pt)
    {
        strcpy(frase[quantas_palavras], pt);
        pt = strtok(NULL, "!. ");
        quantas_palavras++;
    }
    return;
};
void addSensor(){
    char aux[100];
    memset(buffer, 0, 100);
    char erro_sensor[5][5];
    char sensores_add[5][5];
    int erro = 0;
    int adicionados = 0;
    int certo = 1;
    equipamento = atoi(frase[quantas_palavras-1]);
    if(total_sensores > 15 || (total_sensores + quantas_palavras-4)>15){
        sprintf(buffer, "limit exceeded\n");
    }
    else if(equipamento > 4 || equipamento < 1){
        sprintf(buffer, "invalid equipment\n");
    }
    else{
        for(int contador = 2; contador < quantas_palavras-2; contador ++)
        {
            sensor = atoi(frase[contador]);
            if(sensor < 0 || sensor > 4){
                certo = 0;
            }
            else if(confereSensor(sensor)){
                equipamentos[equipamento][sensor] = 1;
                strcpy(Ordem[equipamento].add_ordem[Ordem[equipamento].qtd], frase[contador]);
                Ordem[equipamento].qtd++;
                strcpy(sensores_add[adicionados], frase[contador]);
                adicionados++;
                total_sensores++;
            }
            else{
                strcpy(erro_sensor[erro], frase[contador]);
                erro++;
            }
        }
        if(certo){
            if(erro > 0 && adicionados>0){
            sprintf(buffer, "sensor ");
            for(int i = 0; i < adicionados; i++){
                strcat(buffer, sensores_add[i]);
                strcat(buffer, " ");
            }
            strcat(buffer,"added ");
            for(int i = 0; i < erro; i++){
                strcat(buffer, erro_sensor[i]);
                strcat(buffer, " ");
            }
            sprintf(aux, "alredy exists in 0%d\n", equipamento);
            strcat(buffer, aux);
            }
            else if (erro > 0){
                for(int i = 0; i < erro; i++){
                    strcat(buffer, erro_sensor[i]);
                    strcat(buffer," ");
                }
                sprintf(aux, "alredy exists in 0%d\n", equipamento);
                strcat(buffer, aux);
            }
            else{
                sprintf(buffer,"sensor ");
                for(int i = 0; i < adicionados; i++){
                    strcat(buffer, sensores_add[i]);
                    strcat(buffer, " ");
                }
                strcat(buffer,"added\n");
            }
        }
        else{
           sprintf(buffer, "invalid sensor\n"); 
        }
    }
};

void imprimeMatriz()
{
    for (int i = 1; i < 5; i++)
    {
        printf("\n  ");
        for (int j = 1; j < 5; j++)
        {
            printf("%d   ",equipamentos[i][j]);
        }
        printf("\n");
    }
};


void removeSensor(){
    char aux[100];
    memset(buffer, 0, 100);
    char erro_sensor[5][5];
    char sensores_removed[5][5];
    int erro = 0;
    int removidos = 0;
    int certo = 1;
    equipamento = atoi(frase[quantas_palavras-1]);
    if(equipamento > 4 || equipamento < 1){
        sprintf(buffer, "invalid equipment\n");
    }
    else{
        for(int contador = 2; contador < quantas_palavras-2; contador ++)
        {
            sensor = atoi(frase[contador]);
            if(sensor < 0 || sensor > 4){
                certo = 0;
            }
            else if(confereSensor(sensor) == 0){
                equipamentos[equipamento][sensor] = 0;
                reordenaEstrutura(contador);
                Ordem[equipamento].qtd--;
                strcpy(sensores_removed[removidos], frase[contador]);
                removidos++;
                total_sensores--;
            }
            else{
                strcpy(erro_sensor[erro], frase[contador]);
                erro++;
            }
        }
        if(certo){
            if(erro > 0 && removidos>0){
                sprintf(buffer, "sensor ");
                for(int i = 0; i < removidos; i++){
                    strcat(buffer, sensores_removed[i]);
                    strcat(buffer, " ");
                strcat(buffer,"removed ");
                for(int i = 0; i < erro; i++){
                    strcat(buffer, erro_sensor[i]);
                    strcat(buffer, " ");
                }
                sprintf(aux," does not exists in 0%d\n", equipamento);
                strcat(buffer, aux);
                }
            }
            else if (erro > 0){
                for(int i = 0; i < erro; i++){
                    strcat(buffer, erro_sensor[i]);
                    strcat(buffer, " ");
                }
                sprintf(aux," does not exists in 0%d\n", equipamento);
                strcat(buffer, aux);
            }
            else{
                sprintf(buffer, "sensor ");
                for(int i = 0; i < removidos; i++){
                    strcat(buffer, sensores_removed[i]);
                    strcat(buffer, " ");
                strcat(buffer,"removed\n");
                }
            }
        }
        else{
           sprintf(buffer, "invalid sensor\n"); 
        }
    }
};


int confereSensor(int sensor){
    int adicionado = 1;
    if (equipamentos[equipamento][sensor] == 1){
        adicionado = 0;
    }
    return adicionado;
};

void listSensor(){
    memset(buffer, 0, 100);
    equipamento = atoi(frase[quantas_palavras-1]);
    if(equipamento > 4 || equipamento < 1){
        sprintf(buffer, "invalid equipment\n");
    }
    else{
        strcat(buffer,Ordem[equipamento].add_ordem[0]);
        for(int i = 1; i <= Ordem[equipamento].qtd+1; i++){
            strcat(buffer, " ");
            strcat(buffer, Ordem[equipamento].add_ordem[i]);
        }
        strcat(buffer, "\n");
    }
};

void reordenaEstrutura(int contador){   
    for(int i = 0; i <= Ordem[equipamento].qtd+1; i++){
        if (strcmp(Ordem[equipamento].add_ordem[i],frase[contador]) == 0){
            for(int j = i; j <= Ordem[equipamento].qtd+1; j++){
                strcpy(Ordem[equipamento].add_ordem[j],Ordem[equipamento].add_ordem[j+1]);
            }
        }
    } 
};

void readSensor(){
    memset(buffer, 0, 100);
    char aux[100];
    srand(time(NULL));
    char erro_sensor[5][5];
    char sensores_add[5][5];
    int erro = 0;
    int adicionados = 0;
    int certo = 1;
    equipamento = atoi(frase[quantas_palavras-1]);
    if(equipamento > 4 || equipamento < 1){
        sprintf(buffer, "invalid equipment\n");
    }
    else{
        for(int contador = 2; contador < quantas_palavras-2; contador ++)
        {
            sensor = atoi(frase[contador]);
            if(sensor < 0 || sensor > 4){
                certo = 0;
            }
            else if(confereSensor(sensor) == 0){
                strcpy(sensores_add[adicionados], frase[contador]);
                adicionados++;
            }
            else{
                strcpy(erro_sensor[erro], frase[contador]);
                erro++;
            }
        }
        if(certo){
            if(erro > 0 && adicionados>0){
                for(int i = 0; i < adicionados; i++){
                    float leitura = rand() % 1000;
                    leitura /= 100;
                    sprintf(aux, "%.2f", leitura);
                    strcat(buffer, aux);
                    strcat(buffer, " ");
                }
                strcat(buffer,"and ");
                for(int i = 0; i < erro; i++){
                    strcat(buffer, erro_sensor[i]);
                    strcat(buffer, " ");
                }
                sprintf(aux, "not installed\n");
                strcat(buffer, aux);
            }
            else if (erro > 0){
                for(int i = 0; i < erro; i++){
                    strcat(buffer, erro_sensor[i]);
                    strcat(buffer," ");
                }
                sprintf(aux, "not installed\n");
                strcat(buffer, aux);
            }
            else{
                float leitura = rand() % 1000;
                leitura /= 100;
                sprintf(aux, "%.2f", leitura);
                strcat(buffer, aux);
                for(int i = 1; i < adicionados; i++){
                    float leitura = rand() % 1000;
                    leitura /= 100;
                    sprintf(aux, " %.2f", leitura);
                    strcat(buffer, aux);
                }
                strcat(buffer,"\n");
            }
        }
        else{
            sprintf(buffer, "invalid sensor\n");
        }
    }
}