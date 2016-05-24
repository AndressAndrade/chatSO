/* ------ Trabalho de Sistemas Operacionais -----
   ------------ CHAT CLIENTE/SERVIDOR -----------
   ---------- Equipe: Andressa Andrade ----------
   --------------- Renata Antunes --------------- */
/* ------------------ SERVIDOR ------------------ */

// Biblioteca
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>

// Variáveis constantes
#define IP "127.0.0.1"          // IP definido
#define PORTA 31337             // Porta utilizada definida

#define MAX_CLIENTE 10          // Máximo de cliente
#define TAM_NOME 30             // Tamanho do nome
#define TAM_BUFFER 3000         // Tamanho do Buffer
#define BUFF 2048               //

// Estrutura
struct PACOTE {
    char comando[TAM_NOME];     // Comando
    char nome[TAM_NOME];        // Nome do Cliente
    char buff[TAM_BUFFER];      // Descrição
};

struct THREADINFO {
    pthread_t thread_ID;        // Ponteiro do thread
    int socket_servidor;        // Socket servidor
    char nome[TAM_NOME];        // Nome do Cliente
};

struct NOS {
    struct THREADINFO threadinfo;
    struct NOS *proximo;
};

struct LISTA {
    struct NOS *topo, *fim;
    int tamanho;
};

// Funções
void *io(void *parametro);
void *conexoes();
void *cliente();
void *mensagem(void *fd);
int comparar(struct THREADINFO *a, struct THREADINFO *b); 
int compararCliente(struct THREADINFO *a, struct THREADINFO *b);
void iniciarLista(struct LISTA *lista);
int inserirLista(struct LISTA *lista, struct THREADINFO *thr_info);
int deletarLista(struct LISTA *lista, struct THREADINFO *thr_info);
void lixoLista(struct LISTA *lista);
void imprimirHorario();

// Variaveis globais
int err_ret;
int socket_tamanho;
int socket_servidor;
int socket_cliente;
int fds[2];
struct THREADINFO thread_info[MAX_CLIENTE];
struct LISTA listaCliente;
pthread_mutex_t listaCliente_mutex;
struct sockaddr_storage cliente_endereco;

// Main
int main(int argc, char **argv) {

    struct sockaddr_in servidor_endereco;

    pthread_t interrupcao;
    pthread_t T1;
    pthread_t T2;

    // Iniciando a Lista
    iniciarLista(&listaCliente);

    // Iniciando o Mutex
    pthread_mutex_init(&listaCliente_mutex, NULL);

    // Criando o socket do servidor
    socket_servidor = socket(AF_INET, SOCK_STREAM, 0);             

    if (socket_servidor == -1){
        perror("Socket falhou!\n");
        exit(1);
    }
    printf("Socket ok!\n");

    servidor_endereco.sin_family = AF_INET;
    servidor_endereco.sin_port = htons(PORTA);
    servidor_endereco.sin_addr.s_addr = inet_addr(IP);

    // interligando o socket com o endereço (local)
    if (bind(socket_servidor, (struct sockaddr *) &servidor_endereco, sizeof(servidor_endereco)) == -1){     
        perror("Bind falhou!\n");
        close(socket_servidor);
        exit(1);
    }
    printf("Bind ok!\n");

    //
    if(listen(socket_servidor, MAX_CLIENTE) == -1) {
        err_ret = errno;
        fprintf(stderr, "Listen falhou!\n");
        return err_ret;
    }
	printf("Listen ok!\n");

    // Criando o Pipe para as Threads
    if (pipe(fds)){
        err_ret = errno;
        fprintf (stderr, "Pipe falhou!\n");
        return err_ret;
    }
	printf("Pipe ok!\n");

    // Inicializando a thread interrupção para controlar as entradas e saídas (IO) - T1
    if(pthread_create(&interrupcao, NULL, io, NULL) != 0) {
        err_ret = errno;
        fprintf(stderr, "THreand Interrupção falhou!\n");
        return err_ret;
    }
	printf("Thread Interrupção ok!\n");

    // Inicializando a thread T1 para as conexões 
    if(pthread_create(&T1, NULL, conexoes, NULL) != 0){
        err_ret = errno;
        fprintf(stderr, "Thread T1 falhou!\n");
        return err_ret;
    }
	printf("Thread T1 ok!\n");

    // Inicializando a thread T2 para as conexões abertas em T1
    if(pthread_create(&T2, NULL, cliente, NULL) != 0){
        err_ret = errno;
        fprintf(stderr, "Thread T2 falhou!\n");
        return err_ret;
    }
	printf("Thread T2 ok!\n");

    while(1){
    }  

    return 0;
}

// Descrição das funções
int comparar(struct THREADINFO *a, struct THREADINFO *b) {
    return a->socket_servidor - b->socket_servidor;
}

int compararCliente(struct THREADINFO *a, struct THREADINFO *b) {
    if(!strcmp(a->nome, b->nome)) { 
        return 0;
    }
    return 1;
}

void iniciarLista(struct LISTA *lista) {
    lista->topo = lista->fim = NULL;
    lista->tamanho = 0;
}

int inserirLista(struct LISTA *lista, struct THREADINFO *thr_info) {
    if(lista->tamanho == MAX_CLIENTE) {
        return -1;
    } 
    if(lista->topo == NULL) {
        lista->topo = (struct NOS *)malloc(sizeof(struct NOS));
        lista->topo->threadinfo = *thr_info;
        lista->topo->proximo = NULL;
        lista->fim = lista->topo;
    }
    else {
        lista->fim->proximo = (struct NOS *)malloc(sizeof(struct NOS));
        lista->fim->proximo->threadinfo = *thr_info;
        lista->fim->proximo->proximo = NULL;
        lista->fim = lista->fim->proximo;
    }
    lista->tamanho++;
    return 0;
}

int deletarLista(struct LISTA *lista, struct THREADINFO *thr_info) {
    struct NOS *aux1;       // aux1 é a auxiliar atual
    struct NOS *aux2;       // aux2 é a auxiliar temporal
    if(lista->topo == NULL) {
        return -1;
    }
    if(comparar(thr_info, &lista->topo->threadinfo) == 0) {
        aux2 = lista->topo;
        lista->topo = lista->topo->proximo;
        if(lista->topo == NULL) lista->fim = lista->topo;
        free(aux2);
        lista->tamanho--;
        return 0;
    }
    for(aux1 = lista->topo; aux1->proximo != NULL; aux1 = aux1->proximo) {
        if(comparar(thr_info, &aux1->proximo->threadinfo) == 0) {
            aux2 = aux1->proximo;
            if(aux2 == lista->fim) lista->fim = aux1;
            aux1->proximo = aux1->proximo->proximo;
            free(aux2);
            lista->tamanho--;
            return 0;
        }
    }
    return -1;
}

// 
void lixoLista(struct LISTA *lista) {
    struct NOS *aux1;                   // aux1 é a auxiliar atual
    struct THREADINFO *thr_info;
    printf("Clientes conectados: %d\n", lista->tamanho);
    for(aux1 = lista->topo; aux1 != NULL; aux1 = aux1->proximo) {
        thr_info = &aux1->threadinfo;
        printf("%s\n", thr_info->nome);
    }
}

// Função para imprimir o horário em que a mensagem foi enviada
void imprimirHorario() {
    char hora[20];
    time_t t;
    time(&t);
    localtime(&t);
    strftime (hora,80,"%R",localtime(&t));
    printf("%s\t", hora);
}

void *io(void *parametro) {
    char comando[TAM_NOME];
    while(scanf("%s", comando)==1) {
        if(!strcmp(comando, "SAIR")) {
            printf("Encerrando o servidor....\n");
            pthread_mutex_destroy(&listaCliente_mutex);
            close(socket_servidor);
            exit(0);
        }
        else if(!strcmp(comando, "CLIENTES")) {
            pthread_mutex_lock(&listaCliente_mutex);
            lixoLista(&listaCliente);
            pthread_mutex_unlock(&listaCliente_mutex);
        }
        else if(!strncmp(comando, "AJUDA", 4)) {
            FILE *fin = fopen("ajuda_servidor.txt", "r");
            if(fin != NULL) {
                while(fgets(comando, BUFF-1, fin)) puts(comando);
                fclose(fin);
            }
            else {
                fprintf(stderr, "Arquivo não encontrado.\n");
            }
        }
        else {
            fprintf(stderr, "Comando -%s- desconhecido.\nPara mais informações, acessar o comando AJUDA.\n", comando);
        }
    }
    return NULL;
}

// Função para aceitar as conexões
void *conexoes(){
    while(1) {
        socket_tamanho = sizeof(struct sockaddr_in);
        if((socket_cliente = accept(socket_servidor, (struct sockaddr *)&cliente_endereco, (socklen_t*)&socket_tamanho)) == -1) {
            err_ret = errno;
            fprintf(stderr, "Accept falhou!\n");
        }
        else {
            if(listaCliente.tamanho == MAX_CLIENTE) {
                fprintf(stderr, "O servidor está cheio. Tente mais tarde!\n");
                continue;
            }
            write (fds[1], &socket_cliente,1);
        }
    }
}

void *cliente() {
    struct THREADINFO threadinfo;
    fd_set set;
    int socket_cliente;
    FD_ZERO(&set);
    FD_SET(fds[0], &set);

    while(1) {
        int ret;
        ret = select(FD_SETSIZE, &set, NULL, NULL, NULL);
        if (ret < 0){
           printf("ERROR SELECT!");
	   }
	    else {
           read (fds[0], &socket_cliente,1);          
           threadinfo.socket_servidor = socket_cliente;
           pthread_mutex_lock(&listaCliente_mutex);
           inserirLista(&listaCliente, &threadinfo);
           pthread_mutex_unlock(&listaCliente_mutex);  
           pthread_create(&threadinfo.thread_ID, NULL, mensagem, (void *)&threadinfo);
    	}
    }
}

void *mensagem(void *fd) {
    struct THREADINFO threadinfo = *(struct THREADINFO *)fd;
    struct PACOTE packet;
    struct NOS *aux1;
    int bytes, enviar;
    while(1) {
        bytes = recv(threadinfo.socket_servidor, (void *)&packet, sizeof(struct PACOTE), 0);
        if(!bytes || !strcmp(packet.comando, "SAIR")) {
            imprimirHorario();
            printf("%s \t desconectou.\n", threadinfo.nome);
            pthread_mutex_lock(&listaCliente_mutex);
            deletarLista(&listaCliente, &threadinfo);
            pthread_mutex_unlock(&listaCliente_mutex);
            break;
        }
        if(!strcmp(packet.comando, "LOGIN")) {
            int sock;
            struct PACOTE spacote;
            pthread_mutex_lock(&listaCliente_mutex);
            strcpy(threadinfo.nome, packet.nome);
            for(aux1 = listaCliente.topo; aux1 != NULL; aux1 = aux1->proximo) {
                if(compararCliente(&aux1->threadinfo, &threadinfo) == 0) {
                    memset(&spacote, 0, sizeof(struct PACOTE));
                    strcpy(spacote.comando, "SAIR");
                    strcpy(spacote.nome, packet.nome);
                    sock = threadinfo.socket_servidor;
                    enviar = send(sock, (void *)&spacote, sizeof(struct PACOTE), 0);           
                    break;
                }
            }
            for(aux1 = listaCliente.topo; aux1 != NULL; aux1 = aux1->proximo) {
                if(comparar(&aux1->threadinfo, &threadinfo) == 0) {
                    strcpy(aux1->threadinfo.nome, packet.nome);
                    strcpy(threadinfo.nome, packet.nome);
                    break;
                }
            }              
            pthread_mutex_unlock(&listaCliente_mutex);
            imprimirHorario();
            printf("%s \t está conectado(a).\n", packet.nome);
        }
        if(!strcmp(packet.comando, "ENVIARPARA")) {
            int i;
            char target[TAM_NOME];
            for(i = 0; packet.buff[i] != ' '; i++); packet.buff[i++] = 0;
            strcpy(target, packet.buff);
            pthread_mutex_lock(&listaCliente_mutex);
            for(aux1 = listaCliente.topo; aux1 != NULL; aux1 = aux1->proximo) {
                if(strcmp(target, aux1->threadinfo.nome) == 0) {
                    struct PACOTE spacote;
                    memset(&spacote, 0, sizeof(struct PACOTE));
                    if(!comparar(&aux1->threadinfo, &threadinfo)) continue;
                    strcpy(spacote.comando, "msg");
                    strcpy(spacote.nome, packet.nome);
                    strcpy(spacote.buff, &packet.buff[i]);
                    enviar = send(aux1->threadinfo.socket_servidor, (void *)&spacote, sizeof(struct PACOTE), 0);
                    if (enviar == -1){
                        imprimirHorario();
                        printf("%s \t %s \t executado: Não\n", packet.nome,  packet.comando);
                    } else {
                        imprimirHorario();
                        printf("%s \t %s \t executado: Sim\n", packet.nome,  packet.comando);
                    }
                }
            }
            pthread_mutex_unlock(&listaCliente_mutex);
        }
        else if(!strcmp(packet.comando, "ENVIAR")) {
            imprimirHorario();
            printf("%s \t %s \t executado: Sim\n", packet.nome,  packet.comando);
            pthread_mutex_lock(&listaCliente_mutex);
            for(aux1 = listaCliente.topo; aux1 != NULL; aux1 = aux1->proximo) {
                struct PACOTE spacote;
                memset(&spacote, 0, sizeof(struct PACOTE));
                if(!comparar(&aux1->threadinfo, &threadinfo)) continue;
                strcpy(spacote.comando, "msg");
                strcpy(spacote.nome, packet.nome);
                strcpy(spacote.buff, packet.buff);
                enviar = send(aux1->threadinfo.socket_servidor, (void *)&spacote, sizeof(struct PACOTE), 0);
                if (enviar == -1){
                    imprimirHorario();
                    printf("%s \t %s \t executado: Não\n", packet.nome,  packet.comando);
                }
            }
            pthread_mutex_unlock(&listaCliente_mutex);
        }
        else if(!strcmp(packet.comando, "CONECTADO")) {
            int sock;
            struct PACOTE spacote;

            memset(&spacote, 0, sizeof(struct PACOTE));
            strcpy(spacote.comando, "CONECTADO");
            strcpy(spacote.nome, packet.nome);

            pthread_mutex_lock(&listaCliente_mutex);
            for(aux1 = listaCliente.topo; aux1 != NULL; aux1 = aux1->proximo) {
                if(strcmp(packet.nome, aux1->threadinfo.nome) == 0) {
                    sock = threadinfo.socket_servidor;
                    strcpy(spacote.buff, aux1->threadinfo.nome);
                }
            }
            for(aux1 = listaCliente.topo; aux1 != NULL; aux1 = aux1->proximo) {
                if(strcmp(packet.nome, aux1->threadinfo.nome) != 0) {
                    strcat(spacote.buff, "    ");
                    strcat(spacote.buff, aux1->threadinfo.nome);
                }
            }
            enviar = send(sock, (void *)&spacote, sizeof(struct PACOTE), 0);
            if (enviar == -1){
                imprimirHorario();
                printf("%s \t %s \t executado: Não\n", packet.nome,  packet.comando);
            }
            imprimirHorario();
            printf("%s \t %s \t executado: Sim\n", packet.nome,  packet.comando);
            pthread_mutex_unlock(&listaCliente_mutex);
        }
    }

    // Encerrando o socket
    close(threadinfo.socket_servidor);

    return NULL;
}
