#ifndef OSS_H
#define OSS_H

#define ID_LENGTH 16

typedef enum _symbol {
  null,
  times, div_, mod, and, plus, minus, or, 
  eql, neq, lss, leq, gtr, geq, 
  period, comma, colon, rparen, rbrak, 
  of, then, do_, 
  lparen, lbrak, not, becomes, number, ident,
  semicolon, end, else_, elsif, 
  if_, while_, 
  array, record,
  const_, type, var, procedure, begin, module, eof
} symbol;

typedef char Ident[ID_LENGTH];

int val;
Ident id;
bool error;
  
void Mark(char *msg);
symbol Get(void);
void Init(char* T, int pos);

#endif
