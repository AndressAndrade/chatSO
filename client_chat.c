/* ------ Trabalho de Sistemas Operacionais -----
   ------------ CHAT CLIENTE/SERVIDOR -----------
   ---------- Equipe: Andressa Andrade ----------
   --------------- Renata Antunes --------------- */
/* ------------------ CLIENTE ------------------- */
 
// Biblioteca 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
 
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

#define TAM_BUFFER 3000         // Tamanho do buffer
#define TAM_NOME 30             // Tamanho do nome
#define BUFF 2048               //

// Estrutura
struct PACOTE {
    char comando[TAM_NOME];     // Comando para instrução
    char nome[TAM_NOME];        // Número do nome do clientes
    char buff[TAM_BUFFER];      // Mensagem
};
 
struct CLIENTE {
        int socket_servidor;    // Socket servidor
        char nome[TAM_NOME];    // Nome do cliente
};
 
struct THREADINFO {
    pthread_t thread_ID;        // Ponteiro do Thread
    int socket_servidor;        // Socket servidor
};
 
// Variáveis globais
int estaConectado;
int socket_servidor;
char comando[BUFF];
struct CLIENTE cli;

// Funções
int conectarAoServidor();
void inserirNome(struct CLIENTE *cli);
void logout(struct CLIENTE *cli);
void login(struct CLIENTE *cli);
void *receber(void *parametro);
void enviarParaTodos(struct CLIENTE *cli, char *msg);
void enviarParaCliente(struct CLIENTE *cli, char * target, char *msg);
void mostrarClientes(struct CLIENTE *cli);

// Main
int main(int argc, char **argv) {
    int socket_servidor;
    int tamanho_nome;

    if (argc != 2){
        printf ("O programa client_chat.c deve ser executado com o argumento nome do cliente.\nSegue exemplo abaixo:\nclient_chat <NOME DO CLIENTE> \n");
        return (1);
    }

    strcpy(cli.nome, argv[1]);
    login(&cli);

    while(estaConectado) {
        gets(comando);
        if(!strncmp(comando, "SAIR", 4)) {
            logout(&cli);
            break;
        }
        if(!strncmp(comando, "AJUDA", 5)) {
            FILE *fin = fopen("ajuda_cliente.txt", "r");
            if(fin != NULL) {
                while(fgets(comando, BUFF-1, fin)) puts(comando);
                fclose(fin);
            }
            else {
                fprintf(stderr, "Arquivo não encontrado.\n");
            }
        }
        else if(!strncmp(comando, "ENVIARPARA", 10)) {
            char *ptr = strtok(comando, " ");
            char temp[TAM_NOME];
            ptr = strtok(0, " ");
            memset(temp, 0, sizeof(char) * TAM_NOME);
            if(ptr != NULL) {
                tamanho_nome =  strlen(ptr);
                if(tamanho_nome > TAM_NOME) ptr[TAM_NOME] = 0;
                strcpy(temp, ptr);
                while(*ptr) ptr++; ptr++;
                while(*ptr <= ' ') ptr++;
                enviarParaCliente(&cli, temp, ptr);
            }
        }
        else if(!strncmp(comando, "CONECTADO", 9)) {
            mostrarClientes(&cli);
        }
        else if(!strncmp(comando, "ENVIAR", 6)) {
            enviarParaTodos(&cli, &comando[6]);
        }
        else fprintf(stderr,"Comando -%s- desconhecido.\nPara mais informações, acessar o comando AJUDA.\n", comando);
    }
    return 0;
}

// Descrição das funções
void login(struct CLIENTE *cli) {
    int recvd;
    socket_servidor = conectarAoServidor();
    if(socket_servidor >= 0) {
        estaConectado = 1;
        cli->socket_servidor = socket_servidor;
        struct THREADINFO threadinfo;
        int enviar;
        struct PACOTE pacote;
    
        memset(&pacote, 0, sizeof(struct PACOTE));
        strcpy(pacote.comando, "LOGIN");
        strcpy(pacote.nome, cli->nome);
    
        enviar = send(socket_servidor, (void *)&pacote, sizeof(struct PACOTE), 0);

        pthread_create(&threadinfo.thread_ID, NULL, receber, (void *)&threadinfo);
    }
    else {
        fprintf(stderr, "Conexão rejeitada.\n");
    }
}
 
int conectarAoServidor() {
    int socket_cliente, err_ret;
    struct sockaddr_in servidor_endereco;
    struct hostent *para;
 
 	// Criando o socket do cliente
    socket_cliente = socket(AF_INET, SOCK_STREAM, 0);

    // Abrindo o socket do cliente
    if(socket_cliente == -1) {
        err_ret = errno;
        fprintf(stderr, "Socket falhou!\n");
        return err_ret;
    }
    printf("Socket ok!\n");
 
    // Inserindo os valores
    servidor_endereco.sin_family = AF_INET;
    servidor_endereco.sin_port = htons(PORTA);
    servidor_endereco.sin_addr.s_addr = inet_addr(IP);
 
    // Tentando a conexão com o servidor
    if(connect(socket_cliente, (struct sockaddr *)&servidor_endereco, sizeof(struct sockaddr)) == -1) {
        err_ret = errno;
        fprintf(stderr, "Connect falhou!\n");
        return err_ret;
    }
    else {
        printf("Conectado com sucesso...\n");
        return socket_cliente;
    }
}
 
void logout(struct CLIENTE *cli) {
    int enviar;
    struct PACOTE pacote;
    
    if(!estaConectado) {
        fprintf(stderr, "Você não está conectado!\n");
        return;
    }
    
    memset(&pacote, 0, sizeof(struct PACOTE));
    strcpy(pacote.comando, "SAIR");
    strcpy(pacote.nome, cli->nome);

    enviar = send(socket_servidor, (void *)&pacote, sizeof(struct PACOTE), 0);
    estaConectado = 0;
    close(socket_servidor);
}

void inserirNome(struct CLIENTE *cli) {
    int enviar;
    struct PACOTE pacote;
    
    memset(&pacote, 0, sizeof(struct PACOTE));
    strcpy(pacote.nome, cli->nome);

    enviar = send(socket_servidor, (void *)&pacote, sizeof(struct PACOTE), 0);
}
 
void *receber(void *parametro) {
    int recvd;
    struct PACOTE pacote;
    
    while(estaConectado) {
        recvd = recv(socket_servidor, (void *)&pacote, sizeof(struct PACOTE), 0);
        if(!strcmp(pacote.comando, "SAIR")){
            printf("Você está sendo desconectado!\n");
            logout(&cli);
	    exit(0);
            break; 
        }
        else if(!strcmp(pacote.comando, "CONECTADO")){
            printf("Lista de clientes conectados: %s \n", pacote.buff);
        }
        else if (recvd > 0) {
            printf("%s: %s\n", pacote.nome, pacote.buff);
        }
        else if (!recvd){
            fprintf(stderr, "Conexão com o servidor perdida!\n");
            estaConectado = 0;
            close(socket_servidor);
            break;
        }
        else{
            fprintf(stderr, "O recebimento da mensagem falhou!\n");
            break;
        }
        memset(&pacote, 0, sizeof(struct PACOTE));
    }
    return NULL;
}
 
void enviarParaTodos(struct CLIENTE *cli, char *msg) {
    int enviar;
    struct PACOTE pacote;
    
    if(!estaConectado) {
        fprintf(stderr, "Você não está conectado!\n");
        return;
    }
    
    msg[TAM_BUFFER] = 0;
    
    memset(&pacote, 0, sizeof(struct PACOTE));
    strcpy(pacote.comando, "ENVIAR");
    strcpy(pacote.nome, cli->nome);
    strcpy(pacote.buff, msg);
    
    enviar = send(socket_servidor, (void *)&pacote, sizeof(struct PACOTE), 0);
}
 
void enviarParaCliente(struct CLIENTE *cli, char *target, char *msg) {
    int enviar, targetlen;
    struct PACOTE pacote;

    if(target == NULL) {
        return;
    }

    if(msg == NULL) {
        return;
    }

    if(!estaConectado) {
        fprintf(stderr, "Você não está conectado!\n");
        return;
    }

    msg[TAM_BUFFER] = 0;
    targetlen = strlen(target);
    
    memset(&pacote, 0, sizeof(struct PACOTE));
    strcpy(pacote.comando, "ENVIARPARA");
    strcpy(pacote.nome, cli->nome);
    strcpy(pacote.buff, target);
    strcpy(&pacote.buff[targetlen], " ");
    strcpy(&pacote.buff[targetlen+1], msg);

    enviar = send(socket_servidor, (void *)&pacote, sizeof(struct PACOTE), 0);
}

void mostrarClientes(struct CLIENTE *cli) {
    int enviar, targetlen;
    struct PACOTE pacote;
    
    if(!estaConectado) {
        fprintf(stderr, "Você não está conectado.\n");
        return;
    }
    
    memset(&pacote, 0, sizeof(struct PACOTE));
    strcpy(pacote.comando, "CONECTADO");
    strcpy(pacote.nome, cli->nome);
    enviar = send(socket_servidor, (void *)&pacote, sizeof(struct PACOTE), 0);
}
