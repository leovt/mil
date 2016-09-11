#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct Object Object;
typedef struct Type Type;

struct Object{
	Type* type;
	void* data;
};

struct Type{
	char* name;
	int size;
	bool (*equal)(Object*, Object*);
};

typedef struct String{
	int length;
	char buffer[];
} String;

bool eq_int(Object* a, Object* b){
	return *(int*)(a->data) == *(int*)(b->data);
}

bool eq_str(Object* a, Object* b){
	String *aa = a->data;
	String *bb = b->data;
	return aa->length == bb->length && memcmp(aa->buffer, bb->buffer, aa->length) == 0;
}

Type tp_type = {.name="type", .size=sizeof(Type)};
Type tp_string = {.name="string", .size=sizeof(String), .equal=eq_str};
Type tp_int = {.name="int", .size=sizeof(int), .equal=eq_int};

Object* new_ob_int(int i){
	Object* obj = malloc(sizeof(Object));
	obj->type = &tp_int;
	obj->data = malloc(sizeof(int));
	*((int*)(obj->data)) = i;
	return obj;
}

Object* new_ob_str(int length, const char* buf){
	Object* obj = malloc(sizeof(Object));
	obj->type = &tp_string;
	obj->data = malloc(sizeof(String) + length);
	String *str = obj->data;
	str->length = length;
	memcpy(str->buffer, buf, length);
	return obj;
}

typedef struct Code{
	int length;
	int buffer_size;
	char* buffer;
	int nb_const;
	Object** constants;
} Code;

void co_emit(Code* code, char bt){
	code->length += 1;
	if (code->buffer_size < code->length){
		if (code->buffer_size){
			code->buffer_size = code->buffer_size * 3 / 2; 
		} 
		else {
			code->buffer_size = 256;
		}
		code->buffer = realloc(code->buffer, code->buffer_size);
	}
	code->buffer[code->length-1] = bt;
}

bool ob_eq(Object* a, Object* b){
	if (a->type != b->type) {
		return false;
	}
	
	return memcmp(a->data, b->data, a->type->size) == 0;
}

int co_const_idx(Code* code, Object* obj){
	for (int i = 0; i<code->nb_const; i += 1){
		if (ob_eq(code->constants[i], obj)){
			return i;
		}
	}
	code->nb_const += 1;
	code->constants = realloc(code->constants, sizeof(Object**) * code->nb_const);
	code->constants[code->nb_const-1] = obj;
	return code->nb_const-1;
}






