/* ------ Trabalho de Sistemas Operacionais -----
   ------------ CHAT CLIENTE/SERVIDOR -----------
   ---------- Equipe: Andressa Andrade ----------
   --------------- Renata Antunes --------------- */
/* ------------------ SERVIDOR ------------------ */

// Bibliotecas

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock.h>

#define TAM_BUFFER 128
#define MAX_CLIENTES 20
#define MAX_MENSAGEM 3000
#define EXIT_CALL_STRING "#quit"

int socket_local = 0;
int socket_remoto = 0;
unsigned short porta_local = 0;
unsigned short porta_remota = 0;

char mensagem[MAX_MENSAGEM];

struct sockaddr_in servidor_local;
struct sockaddr_in servidor_remoto;

WSADATA wsa_data;

void msg_erro(char *msg){                                                   // Exibe uma mensagem de erro e termina o programa
    fprintf(stderr, msg);
    system("PAUSE");
    exit(EXIT_FAILURE);
}

void recebe_porta_local(){
    printf("Informe a porta local: ");
    scanf("%d", &porta_local);
    fflush(stdin);
}


int main(int argc, char **argv){
    // Inicia o WinSock, isso é realizado por causa do Windows
    if (WSAStartup(MAKEWORD(2, 0), &wsa_data) != 0){
        msg_erro("WSAStartup() falhou! \n");
    }

    socket_local = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);               // Criando o socket local para o servidor

    if (socket_local == SOCKET_ERROR){
        WSACleanup();
        msg_erro("Xii.... O Socket não foi criado com sucesso! :/ \n");
    }
    printf("\o/ Ebaaa!! Socket criado com sucesso!");

    recebe_porta();

    memset(&servidor_local, 0, sizeof(servidor_local));                     // zera a estrutura servidor_local

    servidor_local.sin_family = AF_INET;
    servidor_local.sin_port = htons(porta_local);
    servidor_local.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(socket_local, (struct sockaddr *) &servidor_local, sizeof(servidor_local)) == SOCKET_ERROR){     // interligando o socket com o endereço (local)
        WSACleanup();
        closesocket(socket_local);
        msg_erro("Xii... A conexão do endereço com o Socket falhou! :/ \n");
    }

    if (listen(socket_local, MAX_CLIENTES) == SOCKET_ERROR){            // SOCKET_ERROR = -1
        WSACleanup();
        closesocket(socket_local);
        msg_erro("Xii... A comunicação falhou! :/ \n");
    }

    printf("Calminhaa aee.... Estamos aguardando a conexão!");

    socket_remoto = accept(socket_local, (struct sockaddr *) &servidor_remoto, &sizeof(servidor_remoto));
    if(socket_remoto == INVALID_SOCKET){                                // INVALID_SOCKET = -1
        WSACleanup();
        closesocket(socket_local);
        msg_erro("Xii... A conexão falhou! :/ \n");
    }

    printf("\o/ Aeee! Conexão estabelecida com %s \n", inet_ntoa(servidor_remoto.sin_addr));
    printf("...Aguardando novas conexões e mensagens...");

    do{
        memset(&mensagem, 0, TAM_BUFFER);                                           // Limpa o BUFFER

        if(recv(socket_remoto, mensagem, TAM_BUFFER, 0) == SOCKET_ERROR){           // recebe a mensagem do cliente
            msg_erro("O recebimento da mensagem falhou! \n");
        }
        printf("%s: %s\n", inet_ntoa(servidor_remoto.sin_addr), mensagem);
    }   while(strcmp(mensagem, EXIT_CALL_STRING));                                   // sai quando receber um "#quit" do cliente

    printf("Encerrando o Servidor....\n");
    WSACleanup();
    closesocket(socket_local);
    closesocket(socket_remoto);

    system("PAUSE");
    return 0;

}




