/* ------ Trabalho de Sistemas Operacionais -----
   ------------ CHAT CLIENTE/SERVIDOR -----------
   ---------- Equipe: Andressa Andrade ----------
   --------------- Renata Antunes --------------- */
/* ------------------ CLIENTE ------------------- */

// Bibliotecas

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock.h>

#define TAM_BUFFER  128
#define EXIT_CALL_STRING "#quit"

int socket_remoto = 0;
int remoto_tamanho = 0;

unsigned short porta_remota = 0;

char ip_remoto[32];
char mensagem[TAM_BUFFER];

struct sockaddr_in servidor_remoto;

WSADATA wsa_data;

void msg_erro(char *msg){                                                   // Exibe uma mensagem de erro e termina o programa
    fprintf(stderr, msg);
    system("PAUSE");
    exit(EXIT_FAILURE);
}

void recebe_ip(){
    printf("Informe o IP do Servidor: ");
    scanf("%d", &ip_remoto);
    fflush(stdin);
}

void recebe_porta_remota(){
    printf("Informe a Porta do Servidor: ");
    scanf("%d", &porta_remota);
    fflush(stdin);
}

int main(int argc, char **argv){
    // Inicia o WinSock, isso é realizado por causa do Windows
    if (WSAStartup(MAKEWORD(2, 0), &wsa_data) != 0){
        msg_erro("\nWSAStartup() falhou!\n");
    }

    recebe_ip();
    recebe_porta_remota();

    socket_remoto = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);               // Criando o socket local para o servidor

    if (socket_remoto == SOCKET_ERROR){
        WSACleanup();
        msg_erro("Xii.... O Socket não foi criado com sucesso! :/\n");
    }
    printf("\o/ Ebaaa!! Socket criado com sucesso!\n");


    memset(&servidor_remoto, 0, sizeof(servidor_remoto));                     // zera a estrutura servidor_remoto

    servidor_remoto.sin_family = AF_INET;
    servidor_remoto.sin_port = htons(porta_remota);
    servidor_remoto.sin_addr.s_addr = htonl(ip_remoto);

    printf("Calminhaa aee.... Estamos aguardando a conexão!\n");

    remoto_tamanho = sizeof(servidor_remoto);

    if(connect(socket_remoto, (struct sockaddr *) &servidor_remoto, &remoto_tamanho) == SOCKET_ERROR){                                // INVALID_SOCKET = -1
        WSACleanup();
        msg_erro("Xii... A conexão falhou! :/ \n");
    }

    printf("\o/ Aeee! Conexão estabelecida com %s\n", inet_ntoa(servidor_remoto.sin_addr));

    do{
        memset(&mensagem, 0, TAM_BUFFER);                                           // Limpa o BUFFER

        printf("\nDigite a mensagem: ");
        gets(mensagem);
        fflush(stdin);

        if(send(socket_remoto, mensagem, (strlen(mensagem)), 0) == SOCKET_ERROR){   // Envia a mensagem para o servidor
            WSACleanup();
            closesocket(socket_remoto);
            msg_erro("Xii... O envio falhou! :/ \n");
        }

        printf("Mensagem enviada com sucesso!");
    }   while(strcmp(mensagem, EXIT_CALL_STRING));                                   // sai quando receber um "#quit" do cliente

    printf("...Encerrando sua conexão...\n");
    WSACleanup();
    closesocket(socket_remoto);

    system("PAUSE");
    return 0;

}
