#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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
  int indent;
  int open_parens;
  TokenList *tok_head;
  TokenList *tok_tail;
  enum {
    ST_BEGIN_OF_LINE,
    ST_HANDLE_DEDENT,
    ST_MAIN_LINE,
  } state;
};

typedef enum TokenType {
  TK_NONE,
  TK_ERROR,
  TK_NEWLINE,
  TK_INDENT,
  TK_DEDENT,
  TK_NUMBER,
  TK_COMMENT,
  TK_STRING,
  TK_IDENTIFIER,
  TK_DEF,
  TK_LOOP,
  TK_BREAK,
  TK_CONTINUE,
  TK_IF,
  TK_ELIF,
  TK_ELSE,
  TK_RETURN,
  TK_PLUS,
  TK_MINUS,
  TK_TIMES,
  TK_DIV,
  TK_MOD,
  TK_ASSIGN,
  TK_EQUAL,
  TK_COLON,
  TK_DOT,
  TK_LPAREN,
  TK_RPAREN,
  TK_EOF,
} TokenType;

static struct {
  char word[10];
  TokenType type;
} keywords[] = {
  {"def", TK_DEF},
  {"loop", TK_LOOP},
  {"break", TK_BREAK},
  {"continue", TK_CONTINUE},
  {"if", TK_IF},
  {"elif", TK_ELIF},
  {"else", TK_ELSE},
  {"return", TK_RETURN},
};

static const char* TokenNames[] = {
  "TK_NONE",
  "TK_ERROR",
  "TK_NEWLINE",
  "TK_INDENT",
  "TK_DEDENT",
  "TK_NUMBER",
  "TK_COMMENT",
  "TK_STRING",
  "TK_IDENTIFIER",
  "TK_DEF",
  "TK_LOOP",
  "TK_BREAK",
  "TK_CONTINUE",
  "TK_IF",
  "TK_ELIF",
  "TK_ELSE",
  "TK_RETURN",
  "TK_PLUS",
  "TK_MINUS",
  "TK_TIMES",
  "TK_DIV",
  "TK_MOD",
  "TK_ASSIGN",
  "TK_EQUAL",
  "TK_COLON",
  "TK_PERIOD",
  "TK_LPAREN",
  "TK_RPAREN",
  "TK_EOF",
};

struct Token{
  TokenType type;
  int pos;
  char* content;
};

struct TokenList{
  Token value;
  TokenList *next;
};

char ls_current(const LexerState *ls){
  //printf("current is %d: %c\n", ls->pos, ls->src[ls->pos]);
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
  ls->open_parens = 0;
  ls->tok_head = 0;
  ls->tok_tail = 0;
  ls->state = ST_BEGIN_OF_LINE;
}


void ls_emit_token(LexerState *ls, Token tok){
  if (tok.type == TK_NONE) return;
  
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

Token simple_token(LexerState *ls, TokenType toktyp){
  return (Token){.type = toktyp, .pos = ls->pos, .content = 0};
}
  
void ls_emit_simple(LexerState *ls, TokenType toktyp){
  ls_emit_token(ls, simple_token(ls, toktyp));
}


Token comment(LexerState *ls){
  int start = ls->pos;
  int length = 0;
  while(ls_consume_exclude(ls, "\n")){
    length += 1;
  };
  ls_consume(ls, "\n");
  Token tok = {TK_COMMENT, start, malloc(length+1)};
  memcpy(tok.content, ls->src+start, length);
  tok.content[length] = 0;
  ls->state = ST_BEGIN_OF_LINE;
  return tok;
}

Token string(LexerState *ls){
  int start = ls->pos;
  int length = 0;
  ls_consume(ls, "\"");
  while(ls_consume_exclude(ls, "\"\n")){
    length += 1;
  }
  ls_consume(ls, "\"");
  Token tok = {TK_STRING, start, malloc(length+1)};
  memcpy(tok.content, ls->src+start+1, length);
  tok.content[length] = 0;
  ls->state = ST_MAIN_LINE;
  return tok;
}

Token identifier(LexerState *ls){
  Token tok = {TK_IDENTIFIER, ls->pos, 0};
  unsigned length = 0;
  while(isalnum(ls->src[ls->pos]) || ls->src[ls->pos] == '_'){
    length += 1;
    ls->pos += 1;
  }

  if(length<sizeof(keywords[0].word)){
    for (unsigned i = 0; i<sizeof(keywords)/sizeof(keywords[0]); i+=1){
      if (!strncmp(ls->src+tok.pos, keywords[i].word, length)){
	tok.type = keywords[i].type;
	break;
      }
    }
  }

  if (tok.type == TK_IDENTIFIER) {
    tok.content = malloc(length+1);
    memcpy(tok.content, ls->src+tok.pos, length);
    tok.content[length] = 0;  
  }

  ls->state = ST_MAIN_LINE;
  return tok;
}

Token number(LexerState *ls){
  int start = ls->pos;
  int length = 0;
  while(isdigit(ls->src[ls->pos])){
    length += 1;
    ls->pos += 1;
   }
  Token tok = {TK_NUMBER, start, malloc(length+1)};
  memcpy(tok.content, ls->src+start, length);
  tok.content[length] = 0;
  ls->state = ST_MAIN_LINE;
  return tok;
}

Token main_line(LexerState *ls){
  for(;;){
    char cur = ls_current(ls);
    switch(cur){
    case ' ':
      ls->pos += 1;
      break;
    case '#':
      return comment(ls);
    case '\"':
      return string(ls);
    case '+':
      ls->pos += 1;
      return simple_token(ls, TK_PLUS);
    case '-':
      ls->pos += 1;
      return simple_token(ls, TK_MINUS);
    case '*':
      ls->pos += 1;
      return simple_token(ls, TK_TIMES);
    case '/':
      ls->pos += 1;
      return simple_token(ls, TK_DIV);
    case '%':
      ls->pos += 1;
      return simple_token(ls, TK_MOD);
    case '\n':
      ls->pos += 1;
      if (!ls->open_parens) {
	ls->state = ST_BEGIN_OF_LINE;
	return simple_token(ls, TK_NEWLINE);
      }
      break;
    case ':':
      ls->pos += 1;
      return simple_token(ls, TK_COLON);
    case '(':
      ls->open_parens += 1;
      ls->pos += 1;
      return simple_token(ls, TK_LPAREN);
    case ')':
      ls->open_parens -= 1;
      ls->pos += 1;
      return simple_token(ls, TK_RPAREN);
    case '=':
      ls->pos += 1;
      if(ls_current(ls) == '='){
	ls->pos += 1;
	return simple_token(ls, TK_EQUAL);
      }
      return simple_token(ls, TK_ASSIGN);      
    default:
      if (isalpha(cur) || cur == '_'){
	return identifier(ls); 
      }
      else if (isdigit(cur)){
	return number(ls);
      }
      else {
	fprintf(stderr, "illegal character \'%c\' 0x%02x\n", cur, cur);
	ls->pos += 1;
	return simple_token(ls, TK_ERROR);
      }
    }
  }
}


Token get_token(LexerState *ls){
  while (ls_current(ls)){
    switch(ls->state){
    case ST_BEGIN_OF_LINE:
      ls->indent = 0;
      while(ls_consume(ls, " ")){
	ls->indent += 1;
      }
      if (ls_consume(ls, "\n")){
	break;
      }
      if (ls->indent > ls->indentations->value){
	ls->indentations = push(ls->indentations, ls->indent);
	ls->state = ST_MAIN_LINE;
	return simple_token(ls, TK_INDENT);
      }
      if (ls->indent < ls->indentations->value){
	ls->state = ST_HANDLE_DEDENT;
	break;
      }
      else {
	ls->state = ST_MAIN_LINE;
      }
      break;
    case ST_HANDLE_DEDENT:
      ls->indentations = pop(ls->indentations);
      if (ls->indent == ls->indentations->value) {
	ls->state = ST_MAIN_LINE;    
	return simple_token(ls, TK_DEDENT);
      }
      else if (ls->indent > ls->indentations->value){
	return simple_token(ls, TK_ERROR);
      }
      break;
    case ST_MAIN_LINE:
      return main_line(ls);
    default:
      exit(EXIT_FAILURE);
    }
  }
  return simple_token(ls, TK_EOF);
}

void lexer(LexerState *ls){
  while (ls_current(ls)){
    ls_emit_token(ls, get_token(ls));
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

#ifdef LEXER_MAIN
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
    //lexer(&ls);

    //TokenList *tl = ls.tok_head;
   
    do{
      Token tok=get_token;
      if (tok.content) {
      printf("Token{type = %s, pos = %d, content = \"%s\"}\n",
	     TokenNames[tok.type], tok.pos, tok.content);
      }
      else{
      printf("Token{type = %s, pos = %d}\n",
	     TokenNames[tok.type], tok.pos);
      }
      tl = tl->next;
    } while(tok.type != TK_EOF);
  }
  return EXIT_SUCCESS;
  
}
#endif
