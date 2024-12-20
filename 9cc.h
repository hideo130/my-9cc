#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// tokenize.c
//
//
typedef enum
{
    TK_RESERVED,
    TK_IDENT, // identifiers
    TK_NUM,
    TK_RETURN, // return
    TK_IF,     // if
    TK_ELSE,   // else
    TK_WHILE,  // while
    TK_FOR,    // for
    TK_TYPE,   // type  
    TK_EOF,
} TokenKind;

// Token type
typedef struct Token Token;
struct Token
{
    TokenKind kind; // Token kind
    Token *next;    // Next token
    int val;        // If kind is TK_NUM, its value
    char *str;
    int len; // Token length
};
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
Token *tokenize(char *input);
//
// parse.c
//
typedef enum
{
    ND_ADD,    // +
    ND_SUB,    // -
    ND_MUL,    // *
    ND_DIV,    // /
    ND_EQ,     // ==
    ND_NE,     // !=
    ND_LT,     // <
    ND_LE,     // <=
    ND_NUM,    // Integer
    ND_ASSIGN, // =
    ND_LVAR,   // local variable
    ND_RETURN, // return
    ND_IF,     // if
    ND_FOR,    // for or while
    ND_BLOCK,  // { ... }
    ND_FUNC,   // function
    ND_ADDR,   // reference *
    ND_DEREF,  // &
} NodeKind;
// AST node type
typedef struct Node Node;
struct Node
{
    NodeKind kind; // Node kind
    Node *next;    // next node of compound statement or next argument
    Node *lhs;     // Left-hand side
    Node *rhs;     // Right-hand side
    int val;       // Used if kind == ND_NUM
    int offset;    // Used if kind == ND_LVAR
    
    // function
    char* func_name;
    // if kind == ND_FUNC has args.
    // if function has multiple argument, we traverse by next variable.
    Node* args;
    int arg_num;

    // "if" or "while" statement
    Node *cond;
    Node *then;
    Node *els;
    Node *init;
    Node *inc;

    // body for compound statement
    Node *body;    
};

typedef struct LVar LVar;

// type of local variable
struct LVar
{
    LVar *next; // next variable or NULL
    char *name; // variable name
    int len;    // length of name
    int offset; // offset from RBP
};

// local variable
extern LVar *locals;

typedef struct Function Function;

struct Function
{
    Function *next;
    char* func_name;
    Node* body;
    LVar* vars;
    Node* args;

};

Function* program(Token *tok);

//
// codegen.c
//
void codegen(Function *fn);