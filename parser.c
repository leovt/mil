#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"

typedef struct Node Node;

struct Node{
  Token tok;
  Node *child;
  Node *sibling;
};

typedef Node *AST;

AST parse(LexerState* ls){
  (void) ls;
  return 0;
}

void dump_ast(AST ast, int level){
  if(ast->child){
    if (ast->tok.content){
      printf("%*s(%s %s\n", level*4, "", TokenNames[ast->tok.type], ast->tok.content);
    }
    else{
      printf("%*s(%s\n", level*4, "", TokenNames[ast->tok.type]);
    }
    dump_ast(ast->child, level+1);
    printf("%*s)\n", level*4, "");
  }
  else{
    if (ast->tok.content){
      printf("%*s(%s %s)\n", level*4, "", TokenNames[ast->tok.type], ast->tok.content);
    }
    else{
      printf("%*s(%s)\n", level*4, "", TokenNames[ast->tok.type]);
    }
  }
  if (ast->sibling){
    dump_ast(ast->sibling, level);
  }
}

void dump(AST ast){
  dump_ast(ast, 0);
}

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

Node c = {{TK_IDENTIFIER, 0, "c"},0,0};
Node b = {{TK_IDENTIFIER, 0, "b"},0,&c};
Node bc = {{TK_TIMES, 0, 0},&b,0};
Node a = {{TK_IDENTIFIER, 0, "a"},0,&bc};
Node a_bc = {{TK_PLUS,0,0},&a,0};

dump(&a_bc);




  if(argc != 2){
    if(argc){
      printf("Usage: %s filename\n", argv[0]);
    }else{
      printf("Usage: need one filename argument\n");
    }
  }else{
   
    char* src = read(argv[1]);
    
    LexerState *ls = lexer_initialize(src);

    AST ast = parse(ls);

    lexer_close(ls);

    dump(ast);

  }
}
