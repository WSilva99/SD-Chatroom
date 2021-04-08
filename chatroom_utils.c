#include "./chatroom_utils.h"

#include <string.h>

// Tratamento da entrada de string
// Removendo a quebra de linha do final
void trim_newline(char *text) {
  int len = strlen(text) - 1;
  if (text[len] == '\n') {
    text[len] = '\0';
  }
}

// Limpando buffer de entrada
void clear_stdin_buffer() {
  int c;
  while((c = getchar()) != '\n' && c != EOF);
}
