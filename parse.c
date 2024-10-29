#include "9cc.h"

Token *token;

bool equal(char *op)
{
    return token->kind != TK_RESERVED || token->len != strlen(op) ||
           memcmp(token->str, op, token->len);
}

// 現在のtokenがopならば現在のtokenを更新する。
// this function is only for TK_RESERVED
bool consume(char *op)
{
    if (equal(op))
        return false;
    token = token->next;
    return true;
}

Token *consume_ident()
{
    if (token->kind == TK_IDENT)
    {
        Token *tmp = token;
        token = token->next;
        return tmp;
    }
    return NULL;
}

void expect(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(token->str, op, token->len))
        error_at(token->str, "'%c'ではありません", op);
    token = token->next;
}

// 現在のtokenが数値ならばその値を返して、tokenを進める
int expect_number()
{
    if (token->kind != TK_NUM)
    {
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof()
{
    return token->kind == TK_EOF;
}

LVar *find_lvar(Token *ident_token)
{
    for (LVar *var = locals; var; var = var->next)
    {
        if (var->len == ident_token->len && !memcmp(ident_token->str, var->name, var->len))
        {
            return var;
        }
    }
    return NULL;
}

Node *stmt();
Node *compound_stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

void program(Token *tok, Node *code[])
{
    token = tok;
    int i = 0;
    while (!at_eof())
    {
        code[i++] = stmt();
    }
    code[i] = NULL;
}

Node *new_node(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = new_node(kind);
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val)
{
    Node *node = new_node(ND_NUM);
    node->val = val;
    return node;
}

Node *new_ident_node(Token *ident_token)
{
    Node *node = new_node(ND_LVAR);
    LVar *lvar = find_lvar(ident_token);
    if (lvar)
    {
        node->offset = lvar->offset;
    }
    else
    {
        lvar = calloc(1, sizeof(LVar));
        lvar->next = locals;
        lvar->name = ident_token->str;
        lvar->len = ident_token->len;

        lvar->offset = locals->offset + 8;
        node->offset = lvar->offset;
        locals = lvar;
    }
    return node;
}

Node *new_func_node(Token *func_token)
{
    Node *node = new_node(ND_FUNC);
    node->func_name = calloc(func_token->len + 1, sizeof(char));
    strncpy(node->func_name, func_token->str, func_token->len);
    expect(")");
    return node;
}

bool skip_token(TokenKind kind)
{
    if (token->kind == kind)
    {
        token = token->next;
        return true;
    }
    return false;
}

Node *stmt()
{
    Node *node;
    if (skip_token(TK_RETURN))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = expr();
    }
    else if (skip_token(TK_IF))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        if (skip_token(TK_ELSE))
        {
            node->els = stmt();
        }
        return node;
    }
    else if (skip_token(TK_WHILE))
    {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        node->cond = expr();
        expect(")");
        node->then = stmt();
        return node;
    }
    else if (skip_token(TK_FOR))
    {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        // init
        if (!consume(";"))
        {
            node->init = expr();
            // expr is not consume ";"
            expect(";");
        }

        // loop condition
        if (!consume(";"))
        {
            node->cond = expr();
            // expr is not consume ";"
            expect(";");
        }

        // inc
        if (!consume(")"))
        {
            node->inc = expr();
            // expr is not consume ";"
            expect(")");
        }
        node->then = stmt();
        return node;
    }
    else if (consume("{"))
    {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        node->body = compound_stmt();
        return node;
    }
    else
    {
        node = expr();
    }

    expect(";");
    return node;
}

Node *compound_stmt()
{
    Node head = {};
    Node *cur = &head;
    while (!consume("}"))
    {
        cur->next = stmt();
        cur = cur->next;
    }
    return head.next;
}

Node *expr()
{
    return assign();
}

Node *assign()
{
    Node *node = equality();
    if (consume("="))
        node = new_binary(ND_ASSIGN, node, assign());
    return node;
}

Node *equality()
{
    Node *node = relational();
    for (;;)
    {
        if (consume("=="))
        {
            node = new_binary(ND_EQ, node, relational());
        }
        else if (consume("!="))
        {
            node = new_binary(ND_NE, node, relational());
        }
        else
        {
            return node;
        }
    }
}

Node *relational()
{
    Node *node = add();
    for (;;)
    {
        if (consume("<"))
        {
            node = new_binary(ND_LT, node, add());
        }
        else if (consume("<="))
        {
            node = new_binary(ND_LE, node, add());
        }
        else if (consume(">"))
        {
            node = new_binary(ND_LT, add(), node);
        }
        else if (consume(">="))
        {
            node = new_binary(ND_LE, add(), node);
        }
        else
        {
            return node;
        }
    }
}

Node *add()
{
    Node *node = mul();
    for (;;)
    {
        if (consume("+"))
            node = new_binary(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_binary(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul()
{
    Node *node = unary();
    for (;;)
    {
        if (consume("*"))
            node = new_binary(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_binary(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *unary()
{
    if (consume("+"))
    {
        return primary();
    }
    else if (consume("-"))
    {
        return new_binary(ND_SUB, new_node_num(0), primary());
    }
    else
    {
        return primary();
    }
}

// primary = num
//         | ident ("(" ")")?
//         | "(" expr ")"
Node *primary()
{
    if (consume("("))
    {
        Node *node = expr();
        expect(")");
        return node;
    }
    Token *ident_token = consume_ident();
    if (ident_token)
    {
        if (consume("("))
        {
            return new_func_node(ident_token);
        }
        else
        {
            return new_ident_node(ident_token);
        }
    }

    return new_node_num(expect_number());
}
