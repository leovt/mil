#include <string.h>
#include <stdlib.h>

#include "osp.h"
#include "osg.h"
#include "oss.h"

static Object topScope;
static Object universe;
static Object guard;


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
    if (x != guard) return x;
    if (s == universe) {
      Mark("undef");
      return x;
    }
    s = s->dsc;
  }
}
