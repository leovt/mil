#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <assert.h>

#include "lexer.h"

#define MAX_INDENT_LEVELS 20

struct LexerState{
  const char* src;
  int pos;
  int indentations[MAX_INDENT_LEVELS];
  int top_indent;
  int indent;
  int open_parens;
  enum {
    ST_BEGIN_OF_LINE,
    ST_HANDLE_DEDENT,
    ST_MAIN_LINE,
  } state;
};


static const struct {
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

const char const * TokenNames[] = {
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
  "TK_LIST",
};

static char ls_current(const LexerState *ls){
  //printf("current is %d: %c\n", ls->pos, ls->src[ls->pos]);
  return ls->src[ls->pos];
}

static char ls_consume(LexerState *ls, const char* chars){
  char cur = ls_current(ls);
  if (strchr(chars, cur)){
    /* TODO: logic for tracking current line/column */
    ls->pos += 1;
    return cur;
  }
  return 0;
}

static char ls_consume_exclude(LexerState *ls, const char* chars){
  char cur = ls_current(ls);
  if (!strchr(chars, cur)){
    /* TODO: logic for tracking current line/column */
    ls->pos += 1;
    return cur;
  }
  return 0;
}


LexerState* lexer_initialize(const char* src){
  LexerState* ls = malloc(sizeof(LexerState)); 
  ls->src = src;
  ls->pos = 0;
  ls->indentations[0] = 0;
  ls->top_indent = 0;
  ls->open_parens = 0;
  ls->state = ST_BEGIN_OF_LINE;
  return ls;
}

void lexer_close(LexerState* ls){
  free(ls);
}


static Token simple_token(LexerState *ls, TokenType toktyp){
  return (Token){.type = toktyp, .pos = ls->pos, .content = 0};
}


static Token comment(LexerState *ls){
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

static Token string(LexerState *ls){
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

static Token identifier(LexerState *ls){
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

static Token number(LexerState *ls){
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

static Token main_line(LexerState *ls){
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
      if (ls->indent > ls->indentations[ls->top_indent]){
	if (ls->top_indent >= MAX_INDENT_LEVELS-1){
	  return simple_token(ls, TK_ERROR);
	}
	ls->top_indent += 1;
	ls->indentations[ls->top_indent] = ls->indent;
	ls->state = ST_MAIN_LINE;
	return simple_token(ls, TK_INDENT);
      }
      if (ls->indent < ls->indentations[ls->top_indent]){
	ls->state = ST_HANDLE_DEDENT;
	break;
      }
      else {
	ls->state = ST_MAIN_LINE;
      }
      break;
    case ST_HANDLE_DEDENT:
      ls->top_indent -= 1;
      if (ls->indent == ls->indentations[ls->top_indent]) {
	ls->state = ST_MAIN_LINE;    
	return simple_token(ls, TK_DEDENT);
      }
      else if (ls->indent > ls->indentations[ls->top_indent]){
	return simple_token(ls, TK_ERROR);
      }
      break;
    case ST_MAIN_LINE:
      return main_line(ls);
    default:
      exit(EXIT_FAILURE);
    }
  }
  if(ls->top_indent){
    ls->top_indent -= 1;
    return simple_token(ls, TK_DEDENT);
  }
  return simple_token(ls, TK_EOF);
}


#endif
