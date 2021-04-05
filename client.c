#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>


#include <sys/socket.h>
#include <arpa/inet.h>

#include "./chatroom_utils.h"
#include "./chatroom_utils.c"

void get_username(char *username) {
  while(true) {
    printf("Digite seu nick: ");
    fflush(stdout);
    memset(username, 0, 1000);
    fgets(username, 22, stdin);
    trim_newline(username);

    if(strlen(username) > 20 || strlen(username) < 4) {
      puts("O nick deve ter de 4 a 20 caracteres.");
    } else {
      break;
    }
  }
}

void set_username(connection_info *connection) {
  message msg;
  msg.type = SET_USERNAME;
  strncpy(msg.username, connection->username, 20);

  if(send(connection->socket, (void*)&msg, sizeof(msg), 0) < 0) {
    perror("Falha no envio.");
    exit(1);
  }
}

void connect_to_server(connection_info *connection, char *address, char *port) {
  while(true) {
    get_username(connection->username);

    //Create socket
    if ((connection->socket = socket(AF_INET, SOCK_STREAM , IPPROTO_TCP)) < 0) {
        perror("Não foi possível criar o socket");
    }

    connection->address.sin_addr.s_addr = inet_addr(address);
    connection->address.sin_family = AF_INET;
    connection->address.sin_port = htons(atoi(port));

    //Connect to remote server
    if (connect(connection->socket, (struct sockaddr *)&connection->address , sizeof(connection->address)) < 0) {
        perror("Falha na conexão.");
        exit(1);
    }

    set_username(connection);

    message msg;
    ssize_t recv_val = recv(connection->socket, &msg, sizeof(message), 0);
    if(recv_val < 0) {
        perror("Falha na recepção.");
        exit(1);
    } else if(recv_val == 0) {
      close(connection->socket);
      printf("O nick \"%s\" está em uso, tente outro.\n", connection->username);
      continue;
    }
    break;
  }
  puts("Conectado ao chatroom.");
  puts("Digite /help ou /h para informações de ajuda..");
}

void stop_client(connection_info *connection) {
  close(connection->socket);
  exit(0);
}

void handle_user_input(connection_info *connection) {
  char input[255];
  fgets(input, 255, stdin);
  trim_newline(input);

  if(strcmp(input, "/q") == 0 || strcmp(input, "/quit") == 0) {
    stop_client(connection);
  } else if(strcmp(input, "/l") == 0 || strcmp(input, "/list") == 0) {
    message msg;
    msg.type = GET_USERS;

    if(send(connection->socket, &msg, sizeof(message), 0) < 0) {
        perror("Falha no envio.");
        exit(1);
    }

  } else if(strcmp(input, "/h") == 0 || strcmp(input, "/help") == 0) {
    puts("/quit ou /q: Encerrar conexão.");
    puts("/help ou /h: Mostrar informações de ajuda.");
    puts("/version ou /v: Mostrar informações de versão.");
    puts("/list ou /l: Mostrar usuários online.");
    puts("/m \"message\": Enviar mensagem publica.");
    puts("/m \"message\" nick: Enviar mensagem privada a usuário.");
  } else if(strncmp(input, "/m", 2) == 0 || strncmp(input, "/message", 8) == 0) {
    message msg;
    msg.type = PUBLIC_MESSAGE;
    strncpy(msg.username, connection->username, 20);

    char *chatMsg, *toUsername;

    char delimiter[2] = { 34, '\0'};
    int incr = 3;

    if(strncmp(input, "/message", 8) == 0) {
      incr = 9;
    }

    chatMsg = strtok(input+incr, "\"");

    toUsername = strtok(NULL, " ");

    if(chatMsg == NULL) {
      puts(KRED "Você deve inserir uma mensagem." RESET);
      return;
    }

    strncpy(msg.data, chatMsg, 255);

    if(!(toUsername == NULL)) {
      msg.type = PRIVATE_MESSAGE;
      if(strlen(toUsername) > 20 || strlen(toUsername) < 4) {
        puts(KRED "Você deve inserir um nick válido " RESET);
        puts(KRED "O nick deve ter de 4 a 20 caracteres." RESET);
        return;
      }
      strncpy(msg.username, toUsername, 20);
    }

    if(send(connection->socket, &msg, sizeof(message), 0) < 0) {
        perror("Falha no envio.");
        exit(1);
    }

  }
}

void handle_server_message(connection_info *connection) {
  message msg;

  //Receive a reply from the server
  ssize_t recv_val = recv(connection->socket, &msg, sizeof(message), 0);
  if(recv_val < 0) {
      perror("Falha na recepção.");
      exit(1);
  } else if(recv_val == 0) {
    close(connection->socket);
    puts("Chatroom encerrado.");
    exit(0);
  }

  switch(msg.type) {

    case CONNECT:
      printf(KGRN "%s entrou." RESET "\n", msg.username);
    break;

    case DISCONNECT:
      printf(KYEL "%s saiu." RESET "\n" , msg.username);
    break;

    case GET_USERS:
      printf("%s", msg.data);
    break;

    case PUBLIC_MESSAGE:
      printf(KGRN "%s" RESET ": %s\n", msg.username, msg.data);
    break;

    case PRIVATE_MESSAGE:
      printf(KCYN "De %s:" KWHT " %s\n" RESET, msg.username, msg.data);
    break;

    case TOO_FULL:
      fprintf(stderr, KRED "O chatroom está cheio." RESET "\n");
      exit(0);
    break;

    case USERNAME_ERROR:
      fprintf(stderr, KRED "%s" RESET "\n", msg.data);
    break;

    default:
      printf("%d\n", msg.type);
      fprintf(stderr, KRED "Mensagem recebida de tipo desconhecido." RESET "\n");
    break;
  }
}


int main(int argc, char *argv[]) {
  connection_info connection;
  fd_set file_descriptors;

  if (argc != 2) {
    fprintf(stderr,"Uso: %s <porta>\n", argv[0]);
    exit(1);
  }

  connect_to_server(&connection, "127.0.0.1", argv[1]);

  //keep communicating with server
  while(true) {
    FD_ZERO(&file_descriptors);
    FD_SET(STDIN_FILENO, &file_descriptors);
    FD_SET(connection.socket, &file_descriptors);
    fflush(stdin);

    if(select(connection.socket+1, &file_descriptors, NULL, NULL, NULL) < 0) {
      perror("Falha na seleção.");
      exit(1);
    }

    if(FD_ISSET(STDIN_FILENO, &file_descriptors)) {
      handle_user_input(&connection);
    }

    if(FD_ISSET(connection.socket, &file_descriptors))
    {
      handle_server_message(&connection);
    }
  }

  close(connection.socket);
  return 0;
}
