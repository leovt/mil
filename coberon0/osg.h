#ifndef OSG_H
#define OSG_H

#include "oss.h"

typedef enum _class{
  Head,
  Var,
  Par,
  Const_,
  Fld,
  Typ,
  Proc,
  SProc,
  Reg,
  Jmp
} Class;

typedef struct _ObjDesc *Object;
typedef struct _TypeDesc *Type;

typedef struct _ObjDesc {
  Class class;
  /*
    int lev;
  */
  Type type;
  Ident name;
  Object next;
  Object dsc;
  int val;
} ObjDesc;

#endif
