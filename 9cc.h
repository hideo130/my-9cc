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
    TK_IF, // if
    TK_ELSE,
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
    ND_IF,      // if
} NodeKind;
// AST node type
typedef struct Node Node;
struct Node
{
    NodeKind kind; // Node kind
    Node *lhs;     // Left-hand side
    Node *rhs;     // Right-hand side
    int val;       // Used if kind == ND_NUM
    int offset;    // Used if kind == ND_LVAR

    // "if" statement
    Node *cond;
    Node *then;
    Node *els;
};
void program(Token *tok, Node *code[]);
//
// codegen.c
//
void gen(Node *node);

typedef struct LVar LVar;

// type of local variable
struct LVar
{
    LVar *next; // next variable or NULL
    char *name; // variable name
    int len; // name of length
    int offset; // offset from RBP
};

// local variable
extern LVar *locals;
