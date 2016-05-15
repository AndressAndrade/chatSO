/* ------ Trabalho de Sistemas Operacionais -----
   ------------ CHAT CLIENTE/SERVIDOR -----------
   ---------- Equipe: Andressa Andrade ----------
   --------------- Renata Antunes --------------- */
/* ------------------ SERVIDOR ------------------ */
 
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
 
// Variaveis constantes

#define IP "127.0.0.1"          // IP definido
#define PORTA 31337             // Porta utilizada definida

#define TAM_BUFFER 3000         // tamanho do buffer
#define TAM_NOME 15             // tamanho do nome
#define BUFF 2048               

// Estrutura

struct PACOTE {
    char comando[TAM_NOME];     // comando para instrução
    char nome[TAM_NOME];        // nome do cliente
    char buff[TAM_BUFFER];      // mensagem
};
 
struct CLIENTE {
        int socket_servidor;    // socket servidor
        char nome[TAM_NOME];    // nome do cliente
};
 
struct THREADINFO {
    pthread_t thread_ID;        // ponteiro do thread
    int socket_servidor;        // socket servidor
};
 
// Variaveis globais

char servidor[] = "";
int porta;
int estaConectado, socket_servidor;
char comando[BUFF];
struct CLIENTE cli;

// Funções

int connect_with_server();
void inserirNome(struct CLIENTE *cli);
void logout(struct CLIENTE *cli);
void login(struct CLIENTE *cli);
void *receber(void *parametro);
void enviarParaTodos(struct CLIENTE *cli, char *msg);
void enviarParaCliente(struct CLIENTE *cli, char * target, char *msg);
void mostrarClientes(struct CLIENTE *cli);

// Main

int main(int argc, char **argv) {
    int socket_servidor, tamanho_nome;

    /*
    if (argc != 4){
        printf ("\nUso: client_chat <CLIENT_NAME> <SERVER_ADDRESS> <SERVER_PORT>\n");
        return (1);
    }


    strcpy(servidor, argv[2]);
    porta = atoi(argv[3]);
    memset(&cli, 0, sizeof(struct CLIENTE));
    */

    strcpy(cli.nome, argv[1]);
    login(&cli);

    while(estaConectado) {
        gets(comando);
        if(!strncmp(comando, "EXIT", 4)) {
            logout(&cli);
            break;
        }
        if(!strncmp(comando, "HELP", 4)) {
            FILE *fin = fopen("help.txt", "r");
            if(fin != NULL) {
                while(fgets(comando, BUFF-1, fin)) puts(comando);
                fclose(fin);
            }
            else {
                fprintf(stderr, "Help file not found...\n");
            }
        }
        else if(!strncmp(comando, "NAME", 4)) {
            char *ptr = strtok(comando, " ");
            ptr = strtok(0, " ");
            memset(cli.nome, 0, sizeof(char) * TAM_NOME);
            if(ptr != NULL) {
                tamanho_nome =  strlen(ptr);
                if(tamanho_nome > TAM_NOME) ptr[TAM_NOME] = 0;
                strcpy(cli.nome, ptr);
                inserirNome(&cli);
            }
        }
        else if(!strncmp(comando, "SENDTO", 6)) {
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
        else if(!strncmp(comando, "WHO", 3)) {
            mostrarClientes(&cli);
        }
        else if(!strncmp(comando, "SEND", 4)) {
            enviarParaTodos(&cli, &comando[5]);
        }
        else fprintf(stderr, "Unknown comando...\n");
    }
    return 0;
}

//################### Function #################

void login(struct CLIENTE *cli) {
    int recvd;
    socket_servidor = connect_with_server();
    if(socket_servidor >= 0) {
        estaConectado = 1;
        cli->socket_servidor = socket_servidor;
        struct THREADINFO threadinfo;
        int enviar;
        struct PACOTE pacote;
    
        memset(&pacote, 0, sizeof(struct PACOTE));
        strcpy(pacote.comando, "LOGIN");
        strcpy(pacote.nome, cli->nome);
    
               
        /* send request to close this connetion */
        enviar = send(socket_servidor, (void *)&pacote, sizeof(struct PACOTE), 0);

        pthread_create(&threadinfo.thread_ID, NULL, receber, (void *)&threadinfo);
 
    }
    else {
        fprintf(stderr, "Connection rejected...\n");
    }
}
 
int connect_with_server() {
    int socket_cliente, err_ret;
    struct sockaddr_in servidor_endereco;
    struct hostent *to;
 
    /*
    if((to = gethostbyname(IP))==NULL) {
        err_ret = errno;
        fprintf(stderr, "gethostbyname() error...\n");
        return err_ret;
    }
    */
 
    socket_cliente = socket(AF_INET, SOCK_STREAM, 0);
    /* open a socket */
    if(socket_cliente == -1) {
        err_ret = errno;
        fprintf(stderr, "socket() error...\n");
        return err_ret;
    }
 
    /* set initial values */
    servidor_endereco.sin_family = AF_INET;
    servidor_endereco.sin_port = htons(PORTA);
    servidor_endereco.sin_addr.s_addr = inet_addr(IP);
 
    /* try to connect with servidor */
    if(connect(socket_cliente, (struct sockaddr *)&servidor_endereco, sizeof(struct sockaddr)) == -1) {
        err_ret = errno;
        fprintf(stderr, "connect() error...\n");
        return err_ret;
    }
    else {
        printf("Sucessfully Connected...\n");
        return socket_cliente;
    }
}
 
void logout(struct CLIENTE *cli) {
    int enviar;
    struct PACOTE pacote;
    
    if(!estaConectado) {
        fprintf(stderr, "You are not connected...\n");
        return;
    }
    
    memset(&pacote, 0, sizeof(struct PACOTE));
    strcpy(pacote.comando, "EXIT");
    strcpy(pacote.nome, cli->nome);
    
    /* send request to close this connetion */
    enviar = send(socket_servidor, (void *)&pacote, sizeof(struct PACOTE), 0);
    estaConectado = 0;
    close(socket_servidor);
}

void inserirNome(struct CLIENTE *cli) {
    int enviar;
    struct PACOTE pacote;
    
    memset(&pacote, 0, sizeof(struct PACOTE));
    strcpy(pacote.comando, "NAME");
    strcpy(pacote.nome, cli->nome);
    
    /* send request to close this connetion */
    enviar = send(socket_servidor, (void *)&pacote, sizeof(struct PACOTE), 0);
}
 
void *receber(void *parametro) {
    int recvd;
    struct PACOTE pacote;
    
    while(estaConectado) {
        recvd = recv(socket_servidor, (void *)&pacote, sizeof(struct PACOTE), 0);
        if(!strcmp(pacote.comando, "EXIT")){
            printf("Client's nome has been used... Try again!\n");
            logout(&cli);
	    exit(0);
            break; 
        }else if(!strcmp(pacote.comando, "WHO")){
            printf("List of clients: %s \n", pacote.buff);
        }else if (recvd > 0) {
            printf("%s: %s\n", pacote.nome, pacote.buff);
        }else if (!recvd){
            fprintf(stderr, "Connection lost from servidor...\n");
            estaConectado = 0;
            close(socket_servidor);
            break;
        }else{
            fprintf(stderr, "Receipt of message has failed...\n");
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
        fprintf(stderr, "You are not connected...\n");
        return;
    }
    
    msg[TAM_BUFFER] = 0;
    
    memset(&pacote, 0, sizeof(struct PACOTE));
    strcpy(pacote.comando, "SEND");
    strcpy(pacote.nome, cli->nome);
    strcpy(pacote.buff, msg);
    
    /* send request to close this connetion */
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
        fprintf(stderr, "You are not connected...\n");
        return;
    }
    msg[TAM_BUFFER] = 0;
    targetlen = strlen(target);
    
    memset(&pacote, 0, sizeof(struct PACOTE));
    strcpy(pacote.comando, "SENDTO");
    strcpy(pacote.nome, cli->nome);
    strcpy(pacote.buff, target);
    strcpy(&pacote.buff[targetlen], " ");
    strcpy(&pacote.buff[targetlen+1], msg);
    
    /* send request to close this connetion */
    enviar = send(socket_servidor, (void *)&pacote, sizeof(struct PACOTE), 0);
}

void mostrarClientes(struct CLIENTE *cli) {
    int enviar, targetlen;
    struct PACOTE pacote;
    
    if(!estaConectado) {
        fprintf(stderr, "You are not connected...\n");
        return;
    }
    
    memset(&pacote, 0, sizeof(struct PACOTE));
    strcpy(pacote.comando, "WHO");
    strcpy(pacote.nome, cli->nome);
    enviar = send(socket_servidor, (void *)&pacote, sizeof(struct PACOTE), 0);
}
