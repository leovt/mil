#include <stdlib.h>
#include <stdio.h>

#include "lexer.h"

char* read(char* filename){
  FILE * f = fopen (filename, "rb");

  if (!f)
    exit(EXIT_FAILURE);

  int length;
  fseek(f, 0, SEEK_END);
  length = ftell(f);
  fseek(f, 0, SEEK_SET);
  char* buffer = malloc(length);
  if (!buffer)
    exit(EXIT_FAILURE);
  int length_read = fread(buffer, 1, length, f);
  if (length_read != length)
    exit(EXIT_FAILURE);
  fclose(f);
  return buffer;
}

int main(int argc, char** argv){
  if(argc != 2){
    if(argc){
      printf("Usage: %s filename\n", argv[0]);
    }else{
      printf("Usage: need one filename argument\n");
    }
  }else{
   
    char* src = read(argv[1]);
    
    LexerState *ls = lexer_initialize(src);

    Token tok;
    do{
      tok = get_token(ls);
      if (tok.content) {
      printf("Token{type = %s, pos = %d, content = \"%s\"}\n",
	     TokenNames[tok.type], tok.pos, tok.content);
      }
      else{
      printf("Token{type = %s, pos = %d}\n",
	     TokenNames[tok.type], tok.pos);
      }
    } while(tok.type != TK_EOF);

    lexer_close(ls);
    free(src);
  }
  return EXIT_SUCCESS;
  
}
