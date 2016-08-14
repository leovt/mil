#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#include "oss.h"

static char ch=0;
static int errpos=0;
static symbol sym;

static FILE* reader;

const struct {
  symbol sym;
  char id[12];
} 
keyTab[] = {
  {null, "BY"},
  {do_, "DO"},
  {if_, "IF"},
  {null, "IN"},
  {null, "IS"},
  {of, "OF"},
  {or, "OR"},
  {null, "TO"},
  {end, "END"},
  {null, "FOR"},
  {mod, "MOD"},
  {null, "NIL"},
  {var, "VAR"},
  {null, "CASE"},
  {else_, "ELSE"},
  {null, "EXIT"},
  {then, "THEN"},
  {type, "TYPE"},
  {null, "WITH"},
  {array, "ARRAY"},
  {begin, "BEGIN"},
  {const_, "CONST"},
  {elsif, "ELSIF"},
  {null, "IMPORT"},
  {null, "UNTIL"},
  {while_, "WHILE"},
  {record, "RECORD"},
  {null, "REPEAT"},
  {null, "RETURN"},
  {null, "POINTER"},
  {procedure, "PROCEDURE"},
  {div_, "DIV"},
  {null, "LOOP"},
  {module, "MODULE"}
};
static const int nkw=sizeof(keyTab)/sizeof(keyTab[0]);

const char* symbol_name[number_of_symbols] = {
  "null",
  "times", "div", "mod", "and", "plus", "minus", "or",
  "eql", "neq", "lss", "leq", "gtr", "geq",
  "period", "comma", "colon", "rparen", "rbrak",
  "of", "then", "do",
  "lparen", "lbrak", "not", "becomes", "number", "ident",
  "semicolon", "end", "else", "elsif",
  "if", "while", 
  "array", "record",
  "const", "type", "var", "procedure", "begin", "module", "eof"};


void Mark(char* msg){
  int p = ftell(reader) - 1;
  if(p > errpos){
    fprintf(stderr, "  pos %d %s\n", p, msg);
  }
  errpos = p;
  error = true;
}

static void scan_ident(void){
  int i=0;
  do {
    if (i<ID_LENGTH){
      id[i] = ch;
      i++;
    }
    ch = fgetc(reader);
  } while(isalnum(ch));
  id[i] = '\0';
  int k=0;
  while (k<nkw && strncmp(id, keyTab[k].id, ID_LENGTH)) k++;
  if (k<nkw) sym = keyTab[k].sym;
  else sym = ident;
}

static void scan_number(void){
  val = 0;
  do {
    if (val <= (INT_MAX - (ch - '0')) / 10){
      val = 10*val + (ch - '0');
    }
    else{
      Mark("number too large");
      val = 0;
    }
    ch = fgetc(reader);
  } while(isdigit(ch));
}

static void scan_comment(void){
}

symbol Get(void){
  while (!feof(reader) && ch <= ' ') ch = fgetc(reader);
  
  if (feof(reader))
    return eof;

  switch(ch){
  case '&': ch = fgetc(reader); return and;
  case '*': ch = fgetc(reader); return times;
  case '+': ch = fgetc(reader); return plus;
  case '-': ch = fgetc(reader); return minus;
  case '=': ch = fgetc(reader); return eql;
  case '#': ch = fgetc(reader); return neq;
  case '<': ch = fgetc(reader); 
    if (ch == '=') {
      ch = fgetc(reader);
      return leq;
    } else 
      return lss;
  case '>': ch = fgetc(reader); 
    if (ch == '=') {
      ch = fgetc(reader);
      return geq;
    } else 
      return gtr;
  case ';': ch = fgetc(reader); return semicolon;  
  case ',': ch = fgetc(reader); return comma;  
  case ':': ch = fgetc(reader);
    if (ch == '=') {
      ch = fgetc(reader);
      return becomes;
    } else 
      return colon;
  case '.': ch = fgetc(reader); return period;  
  case '(': ch = fgetc(reader);
    if (ch == '*'){
      scan_comment();
      return Get();
    }
    else
      return lparen;
  case ')': ch = fgetc(reader); return rparen;  
  case '[': ch = fgetc(reader); return lbrak;  
  case ']': ch = fgetc(reader); return rbrak;  
  case '~': ch = fgetc(reader); return not;  
  }
  if(isdigit(ch)){
    scan_number();
    return number;
  }

  if(isalpha(ch)){
    scan_ident();
    return sym;
  }

  ch = fgetc(reader);
  return null;
}

void Init(FILE* r){
  assert(0 == strncmp(symbol_name[eof], "eof", 4));
  reader = r;
}
