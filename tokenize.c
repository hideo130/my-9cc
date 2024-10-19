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

        if ('a' <= *p && *p <= 'z')
        {
            cur = new_token(TK_IDENT, cur, p++, 1);
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    cur = new_token(TK_EOF, cur, p, 0);
    return head.next;
}
