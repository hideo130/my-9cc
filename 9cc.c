#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum
{
    TK_RESERVED,
    TK_NUM,
    TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token
{
    TokenKind kind;
    Token *next;
    int val;
    char *str;
};

// 現在着目しているトークン
Token *token;

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 現在のtokenがopならば現在のtokenを更新する。
bool consume(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op)
        return false;
    token = token->next;
    return true;
}

void expect(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op)
        error("'%c'ではありません", op);
    token = token->next;
}

// 現在のtokenが数値ならばその値を返して、tokenを進める
int expect_number()
{
    if (token->kind != TK_NUM)
    {
        error("数ではありません");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof()
{
    return token->kind == TK_EOF;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;

    return tok;
}

Token *tokenize(char *p)
{
    Token head;
    head.next = NULL;
    Token *cur = &head;
    while (*p)
    {
        if (isspace(*p))
        {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-')
        {
            // numberと違い、現在のtokenにvalを設定していない
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error("トークナイズできません");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }
    token = tokenize(argv[1]);

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");
    // tokenを配置する
    printf("    mov rax, %ld\n", expect_number());
    // tokenを連結リストを先頭から見ていく
    while (!at_eof())
    {
        if (consume('+'))
        {
            printf("    add rax, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf("    sub rax, %d\n", expect_number());
    }

    printf("    ret\n");
    return 0;
}