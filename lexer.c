#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct IntList IntList;
typedef struct LexerState LexerState;
typedef struct Token Token;
typedef struct TokenList TokenList;

struct IntList{
  int value;
  IntList *next;
};

IntList* push(IntList* stack, int value){
  IntList *top = malloc(sizeof(IntList));
  if (!top) {
    exit(EXIT_FAILURE);
  }
  top->value = value;
  top->next = stack;
  return top;
}

IntList* pop(IntList* stack){
  if (!stack){
    exit(EXIT_FAILURE);
  }
  IntList* next = stack->next;
  free(stack);
  return next;
}

struct LexerState{
  const char* src;
  int pos;
  IntList *indentations;
  IntList *parens;
  TokenList *tok_head;
  TokenList *tok_tail;
  enum {
    ST_BEGIN_OF_LINE,
    ST_MAIN_LINE,
    ST_IDENTIFIER,
    ST_NUMBER,
    ST_COMMENT,
    ST_STRING
  } state;
};

enum TokenType {
  TK_ERROR,
  TK_NEWLINE,
  TK_INDENT,
  TK_DEDENT,
  TK_NUMBER,
  TK_COMMENT,
  TK_STRING,
  TK_IDENTIFIER,
  TK_KW_DEF,
  TK_KW_LOOP,
  TK_KW_BREAK,
  TK_KW_CONTINUE,
  TK_KW_IF,
  TK_KW_ELIF,
  TK_KW_ELSE,
  TK_KW_RETURN
};

struct Token{
  enum TokenType type;
  int pos;
  char* content;
};

struct TokenList{
  Token value;
  TokenList *next;
};

char ls_current(const LexerState *ls){
  return ls->src[ls->pos];
}

char ls_consume(LexerState *ls, const char* chars){
  char cur = ls_current(ls);
  if (strchr(chars, cur)){
    /* TODO: logic for tracking current line/column */
    ls->pos += 1;
    return cur;
  }
  return 0;
}

char ls_consume_exclude(LexerState *ls, const char* chars){
  char cur = ls_current(ls);
  if (!strchr(chars, cur)){
    /* TODO: logic for tracking current line/column */
    ls->pos += 1;
    return cur;
  }
  return 0;
}

void ls_initialize(LexerState* ls, const char* src){
  ls->src = src;
  ls->pos = 0;
  ls->indentations = push(0, 0);
  ls->parens = 0;
  ls->tok_head = 0;
  ls->tok_tail = 0;
  ls->state = ST_BEGIN_OF_LINE;
}

void ls_emit(LexerState *ls, Token tok){
  TokenList *node = malloc(sizeof(TokenList));
  node->value = tok;
  node->next = 0;
  if (!ls->tok_head){
    ls->tok_head = node;
    ls->tok_tail = node;
  }
  else{
    ls->tok_tail->next = node;
    ls->tok_tail = node;
  }
}


void begin_of_line(LexerState *ls){
  int indent = 0;
  while(ls_consume(ls, " ")){
    indent += 1;
  }
  if (indent > ls->indentations->value){
    ls->indentations = push(ls->indentations, indent);
    ls->state = ST_MAIN_LINE;
    ls_emit(ls, (Token){TK_INDENT, ls->pos, 0});
  }
  while (indent < ls->indentations->value){
    ls->indentations = pop(ls->indentations);
    ls_emit(ls, (Token){TK_DEDENT, ls->pos, 0});
  }
  ls->state = ST_MAIN_LINE;
}

void lexer(LexerState *ls){
  while (ls_current(ls)){
    switch(ls->state){
    case ST_BEGIN_OF_LINE:
      begin_of_line(ls);
      break;
    case ST_MAIN_LINE:
      break;
    case ST_IDENTIFIER:
      break;
    case ST_NUMBER:
      break;
    case ST_COMMENT:
      break;
    case ST_STRING:
      break;
    default:
      exit(EXIT_FAILURE);
    }
  }
}

const char* read(char* filename){
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
   
    const char* src = read(argv[1]);
    
    LexerState ls;
    ls_initialize(&ls, src);
    lexer(&ls);

    TokenList *tl = ls.tok_head;
    while(tl){
      Token* tok=&(tl->value);
      printf("%d: %d, %s\n", tok->type, tok->pos, tok->content);
    }
  }
  return EXIT_SUCCESS;
  
}
