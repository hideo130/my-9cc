#include "9cc.h"

// 入力プログラム
char *user_input;

void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;

    cur->next = tok;
    return tok;
}

bool is_alnum(char c)
{
    return ('a' <= c && c <= 'z') ||
           ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') ||
           '_' == c;
}

Token *new_ident_token(Token *cur, char **str)
{
    char *head = *str;
    while (is_alnum(*head))
    {
        head++;
    }
    Token *ret = new_token(TK_IDENT, cur, *str, head - *str);
    *str = head;
    return ret;
}

// bool startswith(char *p, char *q)
// {
//     return memcmp(p, q, strlen(q)) == 0;
// }

Token *tokenize(char *p)
{
    user_input = p;
    Token head = {};
    head.next = NULL;
    Token *cur = &head;
    while (*p)
    {
        if (isspace(*p))
        {
            p++;
            continue;
        }

        // 1文字用のパース
        if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == ';')
        {
            // numberと違い、現在のtokenにvalを設定していない
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }
        if ((*p == '=' && *(p + 1) == '=') ||
            (*p == '!' && *(p + 1) == '=') ||
            (*p == '<' && *(p + 1) == '=') ||
            (*p == '>' && *(p + 1) == '='))
        {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (*p == '<' || *p == '>' || *p == '=')
        {
            // numberと違い、現在のtokenにvalを設定していない
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p))
        {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        // return statement
        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6]))
        {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }

        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2]))
        {
            cur = new_token(TK_IF, cur, p, 2);
            p += 2;
            continue;
        }

        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4]))
        {
            cur = new_token(TK_ELSE, cur, p, 4);
            p += 4;
            continue;
        }

        if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5]))
        {
            cur = new_token(TK_WHILE, cur, p, 5);
            p += 5;
            continue;
        }

        if (('a' <= *p && *p <= 'z') || ('A' <= *p && *p <= 'Z') || *p == '_')
        {
            cur = new_ident_token(cur, &p);
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    cur = new_token(TK_EOF, cur, p, 0);
    return head.next;
}
