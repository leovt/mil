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

typedef enum _form{
  Boolean,
  Integer,
  Array,
  Record
} Form;

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

typedef struct _Item {
  Class mode;
  int lev;
  Type type;
  int a,b,r,idx;
} Item;

typedef struct _TypeDesc {
  Form form;
  Object fields;
  Type base;
  int size;
  int len;
} TypeDesc;

extern Type intType;
extern Type boolType;
extern int pc;

extern Item MakeItem(Object);
extern Item MakeConstItem(Type, int);

extern void Index(Item, Item);
extern void Field(Item, Object);
extern void Op1(symbol, Item);
extern void Op2(symbol, Item, Item);
extern void Relation(symbol, Item, Item);
extern void Parameter(Item, Type, Class);
extern void Store(Item, Item);
extern void Call(Item);
extern void IOCall(Item, Item);
extern void TestBool(Item);



extern void FJump(int*);
extern void BJump(int);
extern void FixLink(int);
#endif
