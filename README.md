Programa Chat Servidor/Cliente
Desenvolvido para a disciplina de Sistemas Operacionais ministrada pelo professor Paul Regnier.
Componentes: Andressa Andrade e Renata Antunes
		

 1. Como gerar os binários:

#> make

 2. Sobre o programa server_chat.c

O server_chat não recebe nenhum argumento. 
Uma lista encadeada foi criada contendo como nó uma estrutura de dados:  socket e nome do cliente. 
Para as mensagens trocadas entre servidor e clientes temos uma estrutura de dados chamada PACOTE contendo: comando, nome e mensagem.

No servidor tem duas threads (T1 e T2). T2 recebe as conexões feitas no T1 atraves da chamada pipe. T2 utiliza a chamada select() para gerenciar as conexões.
Além disso, o servidor implementa mutex para proteger a região compartilhada.

3. Sobre o programa client_chat.c

O cliente recebe apenas um argumento que é o seu nome. 
Quando o cliente recebe uma mensagem do servidor, ele a imprime imediatamente na tela. 

