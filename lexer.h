typedef struct LexerState LexerState;
typedef struct Token Token;


typedef enum TokenType {
  TK_NONE,
  TK_ERROR,
  TK_NEWLINE,
  TK_INDENT,
  TK_DEDENT,
  TK_NUMBER,
  TK_COMMENT,
  TK_STRING,
  TK_IDENTIFIER,
  TK_DEF,
  TK_LOOP,
  TK_BREAK,
  TK_CONTINUE,
  TK_IF,
  TK_ELIF,
  TK_ELSE,
  TK_RETURN,
  TK_PLUS,
  TK_MINUS,
  TK_TIMES,
  TK_DIV,
  TK_MOD,
  TK_ASSIGN,
  TK_EQUAL,
  TK_COLON,
  TK_DOT,
  TK_LPAREN,
  TK_RPAREN,
  TK_EOF,
} TokenType;


struct Token{
  TokenType type;
  int pos;
  char* content;
};


extern const char const * TokenNames[];


LexerState* lexer_initialize(const char* src);
void lexer_close(LexerState* ls);
Token get_token(LexerState *ls);

