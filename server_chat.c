/* ------ Trabalho de Sistemas Operacionais -----
   ------------ CHAT CLIENTE/SERVIDOR -----------
   ---------- Equipe: Andressa Andrade ----------
   --------------- Renata Antunes --------------- */
/* ------------------ SERVIDOR ------------------ */

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

// Variaveis constantes

#define IP "127.0.0.1"          // IP definido
#define PORTA 31337             // Porta utilizada definida

#define MAX_CLIENTE 10
#define TAM_NOME 15
#define TAM_BUFFER 3000

// Estrutura

struct PACOTE {
    char comando[TAM_NOME]; // instruction
    char nome[TAM_NOME]; // client's nome
    char buff[TAM_BUFFER]; // payload
};

struct THREADINFO {
    pthread_t thread_ID; // thread's pointer
    int socket_servidor; // socket file descriptor
    char nome[TAM_NOME]; // client's nome
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
void imprimirTempo();

// Variaveis globais
int err_ret, socket_tamanho;
int socket_servidor, socket_cliente, porta, fds[2];
struct THREADINFO thread_info[MAX_CLIENTE];
struct LISTA listaCliente;
pthread_mutex_t listaCliente_mutex;
struct sockaddr_storage cliente_endereco;

// Main
int main(int argc, char **argv) {
    int rv, yes =1;
    struct sockaddr_in servidor_endereco;
    //struct addrinfo hints, *servinfo;
    pthread_t interrupt, T1, T2;

    /*
    if (argc != 2) {
        printf ("\nUso: server_chat <PORT>\n");
        return   (1);
    }

    // convert string to int
    porta = atoi(argv[1]);
    */

    /* initialize linked list */
    iniciarLista(&listaCliente);

    /* initiate mutex */
    pthread_mutex_init(&listaCliente_mutex, NULL);
    /*
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, IP, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // specifies that the rules used in validating addresses supplied to bind() should allow reuse of local addresses
        if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
    printf("setsockopt\n");
    */

    socket_servidor = socket(AF_INET, SOCK_STREAM, 0);             // Criando o socket local para o servidor

    if (socket_servidor == -1){
        perror("Socket falhou!\n");
        exit(1);
    }
    printf("Socket criado com sucesso!\n");

    servidor_endereco.sin_family = AF_INET;
    servidor_endereco.sin_port = htons(PORTA);
    servidor_endereco.sin_addr.s_addr = inet_addr(IP);

    if (bind(socket_servidor, (struct sockaddr *) &servidor_endereco, sizeof(servidor_endereco)) == -1){     // interligando o socket com o endereço (local)
        perror("Bind falhou!\n");
        close(socket_servidor);
        exit(1);
    }
    printf("Bind feito com sucesso!\n");


    /*
    // first, load up address structs with getaddrinfo():
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
	printf("1\n");

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {

        // create socket
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }
	printf("socket ok\n");

        // specifies that the rules used in validating addresses supplied to bind() should allow reuse of local addresses
        if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }
	printf("setsockopt\n");

        // bind to address and porta
        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
            close(socket_servidor);
            perror("server: bind");
            continue;
        }
	printf("bind ok\n");
        break;
	
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }
    */

    /* start listening for connection */
    if(listen(socket_servidor, MAX_CLIENTE) == -1) {
        err_ret = errno;
        fprintf(stderr, "listen() failed...\n");
        return err_ret;
    }
	printf("listen ok\n");

    if (pipe(fds)){
      fprintf (stderr, "Pipe failed.\n");
      return err_ret;
    }
	printf("pipe ok\n");


    /* initiate interrupt handler for IO controlling (T1) */
    if(pthread_create(&interrupt, NULL, io, NULL) != 0) {
        err_ret = errno;
        fprintf(stderr, "pthread_create() failed...\n");
        return err_ret;
    }
	printf("interrupt ok\n");

    /* initiate T1 to handler connections */
    if(pthread_create(&T1, NULL, conexoes, NULL) != 0){
        err_ret = errno;
        fprintf(stderr, "pthread_create() failed\n");
        return err_ret;
    }
	printf("t1 ok\n");

    /* initiate T2 to handler opened connections */
    if(pthread_create(&T2, NULL, cliente, NULL) != 0){
        err_ret = errno;
        fprintf(stderr, "pthread_create() failed\n");
        return err_ret;
    }
	printf("t2 ok\n");

    while(1){
    }       
   return 0;
}

//################### Function #################

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
    if(lista->tamanho == MAX_CLIENTE) return -1;
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
    struct NOS *aux1, *temp;
    if(lista->topo == NULL) return -1;
    if(comparar(thr_info, &lista->topo->threadinfo) == 0) {
        temp = lista->topo;
        lista->topo = lista->topo->proximo;
        if(lista->topo == NULL) lista->fim = lista->topo;
        free(temp);
        lista->tamanho--;
        return 0;
    }
    for(aux1 = lista->topo; aux1->proximo != NULL; aux1 = aux1->proximo) {
        if(comparar(thr_info, &aux1->proximo->threadinfo) == 0) {
            temp = aux1->proximo;
            if(temp == lista->fim) lista->fim = aux1;
            aux1->proximo = aux1->proximo->proximo;
            free(temp);
            lista->tamanho--;
            return 0;
        }
    }
    return -1;
}

void lixoLista(struct LISTA *lista) {
    struct NOS *aux1;
    struct THREADINFO *thr_info;
    printf("Connection count: %d\n", lista->tamanho);
    for(aux1 = lista->topo; aux1 != NULL; aux1 = aux1->proximo) {
        thr_info = &aux1->threadinfo;
        printf("[%d] %s\n", thr_info->socket_servidor, thr_info->nome);
    }
}

// get current time
void imprimirTempo() {
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
        if(!strcmp(comando, "exit")) {
            /* clean up */
            printf("Terminating server...\n");
            pthread_mutex_destroy(&listaCliente_mutex);
            close(socket_servidor);
            exit(0);
        }
        else if(!strcmp(comando, "list")) {
            pthread_mutex_lock(&listaCliente_mutex);
            lixoLista(&listaCliente);
            pthread_mutex_unlock(&listaCliente_mutex);
        }
        else {
            fprintf(stderr, "Unknown comando: %s...\n", comando);
        }
    }
    return NULL;
}

void *conexoes(){

    /* keep accepting connections */
    while(1) {
        socket_tamanho = sizeof(struct sockaddr_in);
        if((socket_cliente = accept(socket_servidor, (struct sockaddr *)&cliente_endereco, (socklen_t*)&socket_tamanho)) == -1) {
            err_ret = errno;
            fprintf(stderr, "accept() failed...\n");
        }
        else {
            if(listaCliente.tamanho == MAX_CLIENTE) {
                fprintf(stderr, "Connection full, request rejected...\n");
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
    
    /* Initialize the file descriptor set. */
    FD_ZERO(&set);
    FD_SET(fds[0], &set);

    while(1){
        int ret = select(FD_SETSIZE, &set, NULL, NULL, NULL);
        if (ret < 0){
           printf("error select");// error occurred
	}
	else
	{
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
        if(!bytes || !strcmp(packet.comando, "EXIT")) {
            imprimirTempo();
            printf("%s \thas disconnected...\n", threadinfo.nome);
            pthread_mutex_lock(&listaCliente_mutex);
            deletarLista(&listaCliente, &threadinfo);
            pthread_mutex_unlock(&listaCliente_mutex);
            break;
        }
        if(!strcmp(packet.comando, "LOGIN")) {
 
            int skfd;
            struct PACOTE spacote;
            
            pthread_mutex_lock(&listaCliente_mutex);
            strcpy(threadinfo.nome, packet.nome);
            for(aux1 = listaCliente.topo; aux1 != NULL; aux1 = aux1->proximo) {
                if(compararCliente(&aux1->threadinfo, &threadinfo) == 0) {
                    memset(&spacote, 0, sizeof(struct PACOTE));
                    strcpy(spacote.comando, "EXIT");
                    strcpy(spacote.nome, packet.nome);
                    skfd = threadinfo.socket_servidor;
                    enviar = send(skfd, (void *)&spacote, sizeof(struct PACOTE), 0);           
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
            imprimirTempo();
            printf("%s \t has conected...\n", packet.nome);
        }
        if(!strcmp(packet.comando, "NAME")) {
            imprimirTempo();
            printf("%s \t %s \t executed: No\n", packet.nome,  packet.comando);
            pthread_mutex_lock(&listaCliente_mutex);
            for(aux1 = listaCliente.topo; aux1 != NULL; aux1 = aux1->proximo) {
                if(comparar(&aux1->threadinfo, &threadinfo) == 0) {
                    strcpy(aux1->threadinfo.nome, packet.nome);
                    strcpy(threadinfo.nome, packet.nome);
                    break;
                }
            }
            pthread_mutex_unlock(&listaCliente_mutex);
        }
        else if(!strcmp(packet.comando, "SENDTO")) {
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
                        imprimirTempo();
                        printf("%s \t %s \t executed: No\n", packet.nome,  packet.comando);
                    } else {
                        imprimirTempo();
                        printf("%s \t %s \t executed: Sim\n", packet.nome,  packet.comando);
                    }
                }
            }
            pthread_mutex_unlock(&listaCliente_mutex);
        }
        else if(!strcmp(packet.comando, "SEND")) {
            imprimirTempo();
            printf("%s \t %s \t executed: Sim\n", packet.nome,  packet.comando);
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
                    imprimirTempo();
                    printf("%s \t %s \t executed: No\n", packet.nome,  packet.comando);
                }
            }
            pthread_mutex_unlock(&listaCliente_mutex);
        }
        else if(!strcmp(packet.comando, "WHO")) {
            int skfd;
            struct PACOTE spacote;

            memset(&spacote, 0, sizeof(struct PACOTE));
            strcpy(spacote.comando, "WHO");
            strcpy(spacote.nome, packet.nome);

            pthread_mutex_lock(&listaCliente_mutex);
            for(aux1 = listaCliente.topo; aux1 != NULL; aux1 = aux1->proximo) {
                if(strcmp(packet.nome, aux1->threadinfo.nome) == 0) {
                    skfd = threadinfo.socket_servidor;
                    strcpy(spacote.buff, aux1->threadinfo.nome);
                }
            }
            for(aux1 = listaCliente.topo; aux1 != NULL; aux1 = aux1->proximo) {
                if(strcmp(packet.nome, aux1->threadinfo.nome) != 0) {
                    strcat(spacote.buff, "    ");
                    strcat(spacote.buff, aux1->threadinfo.nome);
                }
            }
            enviar = send(skfd, (void *)&spacote, sizeof(struct PACOTE), 0);
            if (enviar == -1){
                imprimirTempo();
                printf("%s \t %s \t executed: No\n", packet.nome,  packet.comando);
            }
            imprimirTempo();
            printf("%s \t %s \t executed: Sim\n", packet.nome,  packet.comando);
            pthread_mutex_unlock(&listaCliente_mutex);
        }
        else {
//            fprintf(stderr, "Please keep in touch with %s. He doesn't know how to use the client_chat!\n", threadinfo.nome);
        }
    }

    /* clean up */
    close(threadinfo.socket_servidor);

    return NULL;
}
