#include "9cc.h"

Token *token;
LVar *locals;
bool is_initialize_variable;

Node *new_node(NodeKind kind);

bool equal(char *op)
{
    return token->len == strlen(op) &&
           memcmp(token->str, op, token->len) == 0;
}

bool is_type()
{
    return token->kind == TK_TYPE;
}

char *copy_token_str(Token *target)
{
    char *s = calloc(target->len + 1, sizeof(char));
    strncpy(s, target->str, target->len);
    return s;
}

// 現在のtokenがopならば現在のtokenを更新する。
bool consume(char *op)
{
    if (!equal(op))
        return false;
    token = token->next;
    return true;
}

void skip(char *op)
{
    if (equal(op))
        token = token->next;
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
        error_at(token->str, "'%s'ではありません", op);
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

Node *append_lvar(Token *target)
{
    LVar *lvar = calloc(1, sizeof(LVar));
    Node *node = new_node(ND_LVAR);

    lvar->next = locals;
    lvar->name = target->str;
    lvar->len = target->len;
    if (locals)
    {
        lvar->offset = locals->offset + 8;
    }
    else
    {
        lvar->offset = 8;
    }
    node->offset = lvar->offset;
    locals = lvar;
    return node;
}

Function *function();
Node *compound_stmt();
Node *stmt();
Node *declaration();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();

void parse_arg_definition(Function *fn)
{
    int is_first = 0;
    Node head = {};
    Node *cur = &head;
    expect("(");

    while (!consume(")"))
    {
        if (is_first != 0)
            expect(",");
        is_first = 1;

        if (find_lvar(token))
            error_at(token->str, "same arg names are not allowed.");
        Node *node = append_lvar(token);
        token = token->next;
        cur = cur->next = node;
    }
    fn->args = head.next;
}

// function-definition = ident "(" ")"
//  | ident "(" ident, (",", ident)* ")"
Function *function()
{
    Function *fn = calloc(1, sizeof(Function));
    Token *func_token = consume_ident();
    // copy function name
    fn->func_name = calloc(func_token->len + 1, sizeof(char));
    strncpy(fn->func_name, func_token->str, func_token->len);

    locals = NULL;
    parse_arg_definition(fn);
    // function must have compound statement
    if (!equal("{"))
        error_at(token->str, "'{'ではありません");
    fn->body = stmt();
    fn->vars = locals;

    return fn;
}

// program = function-definition*
Function *program(Token *tok)
{
    token = tok;
    Function head = {};
    Function *cur = &head;
    while (!at_eof())
    {
        cur = cur->next = function();
    }
    return head.next;
}

Node *new_node(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

Node *new_lhs_node(NodeKind kind, Node *lhs)
{
    Node *node = new_node(kind);
    node->lhs = lhs;
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

    LVar *lvar = find_lvar(ident_token);
    if (lvar)
    {
        Node *node = new_node(ND_LVAR);
        node->offset = lvar->offset;
        return node;
    }
    else if (is_initialize_variable)
    {
        // In defining variable we can defin new variable.
        // this path is to support int a=b=1;
        Node *node = append_lvar(ident_token);
        return node;
    }
    else
    {
        error_at(ident_token->str, "undefined token");
        return NULL;
    }
}

// func-call = ident "(" ")" |
//            ident "(" assign ( "," , assign )* )
Node *new_func_node(Token *func_token)
{
    Node *node = new_node(ND_FUNC);
    node->func_name = calloc(func_token->len + 1, sizeof(char));
    strncpy(node->func_name, func_token->str, func_token->len);

    int arg_num = 0;
    Node head = {};
    Node *cur = &head;
    while (!consume(")"))
    {
        arg_num++;
        if (cur != &head)
        {
            expect(",");
        }
        cur->next = assign();
        cur = cur->next;
    }
    node->args = head.next;
    node->arg_num = arg_num;
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

// compound_stmt = stmt | declaration
Node *compound_stmt()
{
    Node head = {};
    Node *cur = &head;
    while (!consume("}"))
    {
        if (is_type())
        {
            Node *node = declaration();
            if (node)
            {
                cur = cur->next = node;
            }
        }
        else
        {
            cur = cur->next = stmt();
        }
    }
    return head.next;
}

// stmt = expr ";"
//      | "return" expr ";"
//      | "{" stmt* "}"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
Node *stmt()
{
    Node *node;
    if (skip_token(TK_RETURN))
    {
        node = new_lhs_node(ND_RETURN, expr());
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

// declspec = "int"?
void declspec()
{
    skip("int");
}

// declarator = ident
Token *declarator()
{
    // In specification "int" keyword only statement is ok but, this compiler does not accept.
    if (token->kind != TK_IDENT)
        error_at(token->str, "expect a variable name, but got %s", copy_token_str(token));
    Token *tmp = token;
    token = token->next;
    return tmp;
}

// declaration = declspec (declarator ("=" expr)? ("," declarator ("=" expr)?)*)? ";"
Node *declaration()
{
    declspec();
    Node head = {};
    Node *cur = &head;
    if (token->kind != TK_IDENT)
    {
        return cur;
    }

    int is_first = 0;
    is_initialize_variable = 1;
    while (token->kind == TK_IDENT)
    {
        if (is_first != 0)
        {
            is_first = 1;
            expect(",");
        }
        Token *name = declarator();
        if (find_lvar(name))
        {
            error_at(token->str, "redefinition of ‘%s’", copy_token_str(token));
        }
        Node *tmp = append_lvar(name);
        if (!consume("="))
        {
            cur = cur->next = tmp;
            continue;
        }
        else
        {
            cur = cur->next = new_binary(ND_ASSIGN, tmp, assign());
        }
    }
    is_initialize_variable = 0;

    expect(";");
    if (cur == &head)
    {
        // we dont have variable definition with initial condition.
        return cur;
    }
    Node *ret = new_node(ND_BLOCK);
    ret->body = head.next;
    return ret;
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

// unary = ("+" | "-" | "*" | "&")? primary
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
    else if (consume("&"))
    {
        return new_lhs_node(ND_ADDR, primary());
    }
    else if (consume("*"))
    {
        return new_lhs_node(ND_DEREF, primary());
    }
    else
    {
        return primary();
    }
}

// primary = num
//         | ident
//         | func-call
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
