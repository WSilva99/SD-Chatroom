#ifndef CHATROOM_UTILS_H_
#define CHATROOM_UTILS_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>

// Cores para diferenciar as mensagens
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\033[0m"

// Enum de tipos de mensagens possíveis
typedef enum {
  CONNECT,
  DISCONNECT,
  GET_USERS,
  SET_USERNAME,
  PUBLIC_MESSAGE,
  PRIVATE_MESSAGE,
  TOO_FULL,
  USERNAME_ERROR,
  SUCCESS,
  ERROR
} message_type;

// Estrutura da mensagem
typedef struct{
  message_type type;
  char username[21];
  char data[256];
} message;

// Estrutura para as informações da conexão do client
typedef struct connection_info{
  int socket;
  struct sockaddr_in address;
  char username[20];
} connection_info;

// Removendo a quebra de linha do final
void trim_newline(char *text);

// Limpando buffer de entrada
void clear_stdin_buffer();

#endif
