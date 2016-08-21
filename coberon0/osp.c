#include <string.h>
#include <stdlib.h>

#include "osp.h"
#include "osg.h"
#include "oss.h"

static Object topScope;
static Object universe;
static Object guard;

static symbol sym;

static void Consume(symbol expected, const char* msg)
{
  if (sym == expected) {
    sym = Get();
  }
  else {
    Mark(msg);
  }
}


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


Item term(void){
  Item x = factor();
  while (sym >= times && sym <= and) {
    symbol op = sym;
    sym = Get();
    if (op == and){
      Op1(op, x);
    }

    Item y = factor();
    Op2(op, x, y);
  }
  return x;
}

Item SimpleExpression(void){
  Item x;
  if (sym == plus) {
    sym = Get();
    x = term();
  }
  else if (sym == minus) {
    sym = Get();
    x = term();
    Op1(minus, x);
  }

  while (sym >= plus && sym <= or) {
    symbol op = sym;
    sym = Get();
    if (op == or) {
      Op1(op, x);
    }

    Item y = term();
    Op2(op, x, y);
  }
  return x;
}

Item expression(void){
  Item x = SimpleExpression();
  if (sym >= eql && sym <= geq) {
    symbol op = sym;
    sym = Get();
    Item y = SimpleExpression();
    Relation(op, x, y);
  }
  return x;
}

void parameter(Object* fp){
  Item x = expression();
  if (IsParam(*fp)) {
    Parameter(x, (*fp)->type, (*fp)->class);
    *fp = (*fp)->next;
  }
  else {
    Mark("too many parameters");
  }
}


void StatSequence(void){
  Item param(void){
    if (sym == lparen) {
      sym = Get();
    }
    else {
      Mark(")?");
    }
    Item x = expression();
    if (sym == rparen) {
      sym = Get();
    }
    else {
      Mark(")?");
    }
    return x;
  }

  for(;;) {
    if (sym < ident) {
      Mark("statement?");
      while (sym < ident) {
	sym = Get();
      }

      if (sym == ident) {
	Object obj = find();
	sym = Get();
	Item x = MakeItem(obj);
	selector(x);
	if (sym == becomes) {
	  sym = Get();
	  Item y = expression();
	  Store(x,y);
	}
	else if (sym == eql) {
	  Mark(":= ?");
	  sym = Get();
	  (void) expression();
	}
	else if (x.mode == Proc) {
	  Object par = obj->dsc;
	  if (sym == lparen){
	    sym = Get();
	    if (sym == rparen){
	      sym = Get();
	    }
	    else {
	      for(;;) {
		parameter(&par);
		if (sym == comma) {
		  sym = Get();
		}
		else if (sym == rparen){
		  sym = Get();
		  break;
		}
		else if (sym >= semicolon){
		  break;
		}
		else{
		  Mark(") or , ?");
		}
	      }
	    }
	  }

	  if (IsParam(par)){
	    Mark("too few parameters");
	  }
	  else{
	    Call(x);
	  }
	}
	else if (x.mode == SProc) {
	  Item y;
	  if (obj->val <= 3) {
	    y = param();
	  }
	  IOCall(x,y);
	}
	else if (obj->class == Typ) {
	  Mark("Illegal Assignment?");
	}
	else {
	  Mark("statement?");
	}
      }
      else if (sym == if_) {
	sym = Get();
	Item x = expression();
	TestBool(x);
	if (sym == then) {
	  sym = Get();
	}
	else {
	  Mark("THEN?");
	}
	StatSequence();
	int L = 0;
	while (sym == elsif) {
	  sym = Get();
	  FJump(&L);
	  FixLink(x.a);
	  x = expression();
	  TestBool(x);
	  if (sym == then) {
	    sym = Get();
	  }
	  else {
	    Mark("THEN?");
	  }
	  StatSequence();
	}
	if (sym == else_) {
	  sym = Get();
	  FJump(&L);
	  FixLink(x.a);
	  StatSequence();
	}
	else {
	  FixLink(x.a);
	}
	
      }
      /*else if (sym == repeat) {
	}*/
      else if (sym == while_) {
	sym = Get();
	int L = pc;
	Item x = expression();
	TestBool(x);
	Consume(do_, "DO?");
	StatSequence();
	BJump(L);
	FixLink(x.a);
	Consume(end, "END?");
      }

      if (sym == semicolon) {
	sym = Get();
      }
      else if ((sym >= semicolon && sym < if_) || sym >= array){
	return;
      }
      else {
	Mark(";?");
      }
    }
  }
}


Object IdentList(Class class){
  if (sym == ident) {
    Object first = NewObject(class);
    sym = Get();
    while (sym == comma) {
      sym = Get();
      if (sym == ident) {
	(void) NewObject(class);
	sym = Get();
      }
      else {
	Mark("ident?");
      }
    }
    Consume(colon, ":?");
    return first;
  }
  return guard;
}


