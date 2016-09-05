#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lexer.h"

typedef struct Node Node;

struct Node{
  Token tok;
  Node *child;
  Node *sibling;
};

static const Token LIST={TK_LIST,0,0};
static const Token ERROR={TK_ERROR,0,0};

static int prec[] = {
	[TK_ASSIGN] = 10,
	[TK_EQUAL] = 20,
	[TK_PLUS] = 30,
	[TK_MINUS] = 30,
	[TK_TIMES] = 40,
	[TK_MOD] = 40};

static bool left_assoc[TK_LIST] = {
	[TK_ASSIGN] = true,
	[TK_EQUAL] = true,
	[TK_PLUS] = true,
	[TK_MINUS] = true,
	[TK_TIMES] = true,
	[TK_MOD] = true};

inline
static bool must_pop_op(TokenType cur, TokenType tos){
	if (left_assoc[cur]) {
		return prec[cur] <= prec[tos];
	}
	else{
		return prec[cur] < prec[tos];
	}
}
	
typedef Node *AST;

typedef struct ParserState{
  LexerState* ls;
  Token current;
} ParserState;

static AST make_node(Token tok){
  AST node = malloc(sizeof(Node));
  node->tok = tok;
  node->sibling = 0;
  node->child = 0;
  return node;
}

Token get_filtered(LexerState* ls){
  for(;;){
    Token tok = get_token(ls);
    if(tok.type != TK_COMMENT){
      return tok;
    }
  }
}

Token consume(ParserState* ps, TokenType expect, const char* msg){
  if (ps->current.type == expect) {
    Token tok = ps->current;
    ps->current = get_filtered(ps->ls);
    return tok;
  }
  else{
    fprintf(stderr, "%s: expected %s but got %s.\n",
	    msg, TokenNames[expect], TokenNames[ps->current.type]);
    return ERROR;
  }
}

ParserState* initialize(const char* src){
  ParserState* ps = malloc(sizeof(ParserState));
  ps->ls = lexer_initialize(src);
  ps->current = get_filtered(ps->ls);
  return ps;
}

AST expression(ParserState* ps){
	AST expr_stack[100];
	int expr_stack_free = 0;
	Token op_stack[100];
	int op_stack_free = 0;
	
	bool prefix = true;
	while (ps->current.type != TK_COLON &&
		   ps->current.type != TK_NEWLINE &&
		   ps->current.type != TK_EOF) {
			   
		if (ps->current.type == TK_NUMBER ||
			ps->current.type == TK_IDENTIFIER || 
			ps->current.type == TK_STRING) {
			
			if (expr_stack_free+1 > 100)
				exit(EXIT_FAILURE);
			expr_stack[expr_stack_free] = make_node(ps->current);
			expr_stack_free += 1;
			prefix = false;
		}
		else if (ps->current.type == TK_PLUS ||
				 ps->current.type == TK_MINUS ||
				 ps->current.type == TK_MOD ||
				 ps->current.type == TK_TIMES ||
				 ps->current.type == TK_EQUAL ||
				 ps->current.type == TK_ASSIGN){
			while(op_stack_free && 
				  must_pop_op(ps->current.type, op_stack[op_stack_free].type)){
					  
				if (expr_stack_free < 2)
					exit(EXIT_FAILURE);
				expr_stack_free -= 2;
				AST rhs = expr_stack[expr_stack_free+1];
				AST lhs = expr_stack[expr_stack_free];
				if (op_stack_free < 1)
					exit(EXIT_FAILURE);
				op_stack_free -= 1;
				Token op = op_stack[op_stack_free];
				AST node = make_node(op);
				node->child = lhs;
				lhs->sibling = rhs;
				expr_stack[expr_stack_free] = node;
				expr_stack_free += 1;
			};
			if (op_stack_free+1 > 100)
				exit(EXIT_FAILURE);
			op_stack[op_stack_free] = ps->current;
			op_stack_free += 1;
			prefix = true;
		}
		else if (ps->current.type == TK_LPAREN) {
			if (prefix) {
				/* grouped expression */
				exit(EXIT_FAILURE);
			}
			else {
				if (op_stack_free+1 > 100)
					exit(EXIT_FAILURE);
				op_stack[op_stack_free] = ps->current;
				op_stack_free += 1;
			}
		}
		else if (ps->current.type == TK_RPAREN) {
			while (op_stack_free > 0 && op_stack[op_stack_free-1].type != TK_LPAREN) {
				if (expr_stack_free < 2)
					exit(EXIT_FAILURE);
				expr_stack_free -= 2;
				AST rhs = expr_stack[expr_stack_free+1];
				AST lhs = expr_stack[expr_stack_free];
				if (op_stack_free < 1)
					exit(EXIT_FAILURE);
				op_stack_free -= 1;
				Token op = op_stack[op_stack_free];
				AST node = make_node(op);
				node->child = lhs;
				lhs->sibling = rhs;
				expr_stack[expr_stack_free] = node;
				expr_stack_free += 1;						
			}
			if (op_stack_free <= 0 || op_stack[op_stack_free-1].type != TK_LPAREN){
				exit(EXIT_FAILURE);
			}
			if (expr_stack_free < 2)
				exit(EXIT_FAILURE);
			expr_stack_free -= 2;
			AST rhs = expr_stack[expr_stack_free+1];
			AST lhs = expr_stack[expr_stack_free];
			if (op_stack_free < 1)
				exit(EXIT_FAILURE);
			op_stack_free -= 1;
			Token op = op_stack[op_stack_free];
			AST node = make_node(op);
			node->child = lhs;
			lhs->sibling = rhs;
			expr_stack[expr_stack_free] = node;
			expr_stack_free += 1;						
		}
		else {
			fprintf(stderr, "expected expression, got %s\n", TokenNames[ps->current.type]);
		}
		(void) prefix;
		consume(ps, ps->current.type, "");
	};
	
	while (op_stack_free > 0) {
		if (expr_stack_free < 2)
			exit(EXIT_FAILURE);
		expr_stack_free -= 2;
		AST rhs = expr_stack[expr_stack_free+1];
		AST lhs = expr_stack[expr_stack_free];
		if (op_stack_free < 1)
			exit(EXIT_FAILURE);
		op_stack_free -= 1;
		Token op = op_stack[op_stack_free];
		AST node = make_node(op);
		node->child = lhs;
		lhs->sibling = rhs;
		expr_stack[expr_stack_free] = node;
		expr_stack_free += 1;		
	}
	if (expr_stack_free != 1) {
		fprintf(stderr, "malformed expression\n");
	}
	return expr_stack[0];
}

AST block(ParserState*);

AST function_definition(ParserState* ps){
  Token def = consume(ps, TK_DEF, "function definition");
  Token name = consume(ps, TK_IDENTIFIER, "function definition");
  consume(ps, TK_LPAREN, "function definition");
  Token arg = consume(ps, TK_IDENTIFIER, "function definition");
  consume(ps, TK_COLON, "function definition");
  Token argtype = consume(ps, TK_IDENTIFIER, "function definition");
  consume(ps, TK_RPAREN, "function definition");
  consume(ps, TK_COLON, "function definition");
  consume(ps, TK_NEWLINE, "function definition");
  consume(ps, TK_INDENT, "function body");
  AST body = block(ps);
  consume(ps, TK_DEDENT, "function body");

  AST function = make_node(def);
  function->child = make_node(name);
  function->child->sibling = body;
  body->sibling = make_node(arg);
  body->sibling->child = make_node(argtype);

  return function;
}

AST conditional(ParserState* ps){
  Token tok = consume(ps, TK_IF, "if statement");
  AST cond=expression(ps);
  consume(ps, TK_COLON, "if statement");
  consume(ps, TK_NEWLINE, "if statement");
  consume(ps, TK_INDENT, "if statement");
  AST true_block = block(ps);
  consume(ps, TK_DEDENT, "if statement");

  AST conditional = make_node(tok);
  conditional->child = cond;
  cond->sibling = true_block;

  return conditional;
}


AST loop_stmt(ParserState* ps){
  Token tok = consume(ps, TK_LOOP, "loop statement");
  consume(ps, TK_COLON, "loop statement");
  consume(ps, TK_NEWLINE, "loop statement");
  consume(ps, TK_INDENT, "loop statement");
  AST body = block(ps);
  consume(ps, TK_DEDENT, "loop statement");

  AST loop = make_node(tok);
  loop->child = body;

  return loop;
}

AST return_stmt(ParserState* ps){
  Token ret = consume(ps, TK_RETURN, "return statement");
  AST expr = expression(ps);
  consume(ps, TK_NEWLINE, "return statement");

  AST return_stmt = make_node(ret);
  return_stmt->child = expr;
  return return_stmt;
}

AST expression_stmt(ParserState* ps){
  AST expr = expression(ps);
  consume(ps, TK_NEWLINE, "expression or assignment statement");

  return expr;
}

AST statement(ParserState* ps){
  switch(ps->current.type){
  case TK_DEF:
    return function_definition(ps);
  case TK_IF:
    return conditional(ps);
  case TK_RETURN:
    return return_stmt(ps);
  case TK_LOOP:
    return loop_stmt(ps);
  case TK_BREAK:
    {
      Token tok = ps->current;
      consume(ps, TK_BREAK, "break statement");
      consume(ps, TK_NEWLINE, "break statement");
      return make_node(tok);
    }
  default:
    return expression_stmt(ps);
  }
}

AST block(ParserState* ps){
  AST head = make_node(LIST);
  AST current_stmt = statement(ps);
  head->child = current_stmt;
  
  while(ps->current.type != TK_DEDENT && ps->current.type != TK_EOF) {
    AST stmt = statement(ps);
    current_stmt->sibling = stmt;
    current_stmt = stmt;
  }

  return head;
}

AST parse(ParserState* ps){
  return block(ps);
}

void dump_ast(AST ast, int level){
  if(ast->child){
    if (ast->tok.content){
      printf("%*s(%s %s\n", level*4, "", TokenNames[ast->tok.type], ast->tok.content);
    }
    else{
      printf("%*s(%s\n", level*4, "", TokenNames[ast->tok.type]);
    }
    dump_ast(ast->child, level+1);
    printf("%*s)\n", level*4, "");
  }
  else{
    if (ast->tok.content){
      printf("%*s(%s %s)\n", level*4, "", TokenNames[ast->tok.type], ast->tok.content);
    }
    else{
      printf("%*s(%s)\n", level*4, "", TokenNames[ast->tok.type]);
    }
  }
  if (ast->sibling){
    dump_ast(ast->sibling, level);
  }
}

void dump(AST ast){
  dump_ast(ast, 0);
}

char* read(char* filename){
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

int main(int argc, char** argv){
  if(argc != 2){
    if(argc){
      printf("Usage: %s filename\n", argv[0]);
    }else{
      printf("Usage: need one filename argument\n");
    }
  }
  else {
    char* src = read(argv[1]);
    ParserState *ps = initialize(src);
    AST ast = parse(ps);
    dump(ast);
  }
}
