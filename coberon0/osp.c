#include <string.h>
#include <stdlib.h>

#include "osp.h"
#include "osg.h"
#include "oss.h"

static Object topScope;
static Object universe;
static Object guard;

static symbol sym;

Object NewObject(Class class){
  Object x = topScope;
  strncpy(guard->name, id, ID_LENGTH);
  while(strncmp(x->next->name, id, ID_LENGTH)){
    x = x->next;
  }
  if (x->next == guard){
    Object new = malloc(sizeof(ObjDesc));
    strncpy(new->name, id, ID_LENGTH);
    new->class = class;
    new->next = guard;
    x->next = new;
    return new;
  }
  else{
    Mark("mult def");
    return x->next;
  }
}

static Object find(void){
  Object s = topScope;
  strncpy(guard->name, id, ID_LENGTH);
  for(;;){
    Object x = s->next;
    while(strncmp(x->name, id, ID_LENGTH)){
      x = x->next;
    }
    if (x != guard) {
      return x;
    }
    if (s == universe) {
      Mark("undef");
      return x;
    }
    s = s->dsc;
  }
}


static Object FindField(Object list){
  strncpy(guard->name, id, ID_LENGTH);
  while(strncmp(list->name, id, ID_LENGTH)){
    list = list->next;
  }
  return list;
}


static bool IsParam(Object obj){
  return (obj->class == Par) || (obj->class == Var && obj->val > 0);
}


static void OpenScope(void){
  Object s = malloc(sizeof(ObjDesc));
  s->class = Head;
  s->dsc = topScope;
  s->next = guard;
  topScope = s;
}


static void CloseScope(void){
  topScope = topScope->dsc;
}

/* ----------- Parser ------------ */

Item expression(void);

void selector(const Item x){
  while(sym == lbrak || sym == period) {
    if (sym == lbrak) {
      sym = Get();
      Item y = expression();

      if (x.type->form == Array){
	Index(x, y);
      }
      else { 
	Mark("not an array"); 
      }
      
      if (sym == rbrak) {
	sym = Get();
      }
      else{
	Mark("]?");
      }
    }
    else{
      sym = Get();
      if (sym == ident) {
	if (x.type->form == Record){
	  Object obj = FindField(x.type->fields);
	  sym = Get();
	  if (obj != guard){
	    Field(x, obj);
	  }
	  else{
	    Mark("undef");
	  }
	}
	else{
	  Mark("not a record");
	}
      }
    }
  }
}


Item factor(void){
  if (sym < lparen){
    Mark("ident?");
    while (sym < lparen) sym = Get();
  }

  switch(sym){
  case ident: {
    Object obj = find();
    sym = Get();
    Item x = MakeItem(obj);
    selector(x);
    return x;
  }
  case number: {
    int value = val;
    sym = Get();
    return MakeConstItem(intType, value);
  }
  case lparen: {
    sym = Get();
    Item x = expression();
    if (sym == rparen) {
      sym = Get();
    }
    else {
      Mark(")?");
    }
    return x;
  }
  case not: {
    sym = Get();
    Item x = factor();
    Op1(not, x);
    return x;
  }
  default:
    Mark("factor?");
    return MakeItem(guard);
  }
}
