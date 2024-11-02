#include "9cc.h"

static int count()
{
    static int i = 1;
    return i++;
}

const char argument_reg[][4] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

// we push variable address at top of stack
void gen_lval(Node *node)
{
    if (node->kind != ND_LVAR)
    {
        error("values cannot be assigned to anything other than lval");
    }
    // for now, 26 variables were defined starting with rbp.
    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n");
}

// function call
void gen_args(Node *arg)
{
    int arg_num = arg->arg_num;
    Node *now = arg->args;
    for (int i = 0; i < arg_num; i++)
    {
        printf("    push %s\n", argument_reg[i]);
        gen(now);
        printf("    pop %s\n", argument_reg[i]);
        now = now->next;
    }
}

void recov_arg_reg(int arg_num)
{
    for (int i = arg_num; i > 0; i--)
    {
        printf("    pop %s\n", argument_reg[i - 1]);
    }
}

void gen(Node *node)
{
    switch (node->kind)
    {
    case ND_NUM:
        printf("    push %d\n", node->val);
        return;
    case ND_LVAR:
        // this block generate code when line with only variable.
        // variable assignment is done with ND_ASSIGN
        gen_lval(node);
        // push value from variable
        printf("    pop rax\n");
        printf("    mov rax, [rax]\n");
        printf("    push rax\n");
        return;
    case ND_ASSIGN:
        // variable assignment
        gen_lval(node->lhs);
        gen(node->rhs);
        printf("    pop rdi\n");
        printf("    pop rax\n");
        printf("    mov [rax], rdi\n");
        // push rhs value to stack
        // By doing so, you can perform value assignments like the following:
        // a=b=1
        printf("    push rdi\n");
        return;
    case ND_RETURN:
        gen(node->lhs);
        printf("    pop rax\n");
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
        return;
    case ND_IF:
        gen(node->cond);
        printf("    pop rax\n");
        printf("    cmp rax, 0\n");
        int label_num = count();
        if (node->els)
        {
            printf("    je  .Lelse%d\n", label_num);
            gen(node->then);
            printf("    jmp .Lend%d\n", label_num);
            printf(".Lelse%d:\n", label_num);
            gen(node->els);
        }
        else
        {
            printf("    sete al\n");
            printf("    movzb rax, al\n");
            printf("    push rax\n");
            printf("    je  .Lend%d\n", label_num);
            printf("    pop rax\n");
            gen(node->then);
        }
        printf(".Lend%d:\n", label_num);
        return;
    case ND_FOR:
        int label_num2 = count();
        if (node->init)
        {
            gen(node->init);
        }
        printf(".Lbegin%d:\n", label_num2);
        if (node->cond)
        {
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je  .Lend%d\n", label_num2);
        }
        gen(node->then);
        if (node->inc)
        {
            gen(node->inc);
        }

        printf("    jmp .Lbegin%d\n", label_num2);
        printf(".Lend%d:\n", label_num2);
        return;
    case ND_BLOCK:
        for (Node *s = node->body; s; s = s->next)
        {
            gen(s);
            printf("    pop rax\n");
        }
        printf("    push rax\n");
        return;
    case ND_FUNC:
        // set arguments to register
        gen_args(node);
        printf("    push r12\n");
        // save original rsp to r12
        printf("    mov r12, rsp\n");
        printf("    and rsp, 0xfffffffffffffff0\n");
        printf("    call %s\n", node->func_name);
        // recover original rsp from r12
        printf("    mov rsp, r12\n");
        printf("    pop r12\n");

        recov_arg_reg(node->arg_num);

        // normally the return value is set in rax
        // but now we assume the result is placed on the top of the stack
        printf("    push rax\n");
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind)
    {
    case ND_ADD:
        printf("    add rax, rdi\n");
        break;
    case ND_SUB:
        printf("    sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("    imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("    cqo\n");
        printf("    idiv rdi\n");
        break;
    case ND_EQ:
        printf("    cmp rax, rdi\n");
        printf("    sete al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_NE:
        printf("    cmp rax, rdi\n");
        printf("    setne al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LT:
        printf("    cmp rax, rdi\n");
        printf("    setl al\n");
        printf("    movzb rax, al\n");
        break;
    case ND_LE:
        printf("    cmp rax, rdi\n");
        printf("    setle al\n");
        printf("    movzb rax, al\n");
        break;
    }

    printf("    push rax\n");
}

int update_arg_offset(LVar *var)
{
    int offset = 0;
    for (LVar *now = var; now; now = now->next)
    {
        now->offset = offset + 8;
        offset += 8;
    }
    return offset;
}

// arguments are reference by ident.
// begining of function, arguments are set in register
// so we place them to stack
void save_reg_of_argment_to_stack(Function *fn)
{
    int i = 0;
    for (Node *arg = fn->args; arg; arg = arg->next)
    {

        gen_lval(arg);
        printf("    pop rax\n");
        printf("    mov [rax], %s\n", argument_reg[i]);
        i += 1;
    }
}

void codegen(Function *fn)
{
    for (Function *target_func = fn; target_func; target_func = target_func->next)
    {
        int offset_size = update_arg_offset(target_func->vars);
        printf(".globl %s\n", target_func->func_name);
        printf("%s:\n", target_func->func_name);

        // prologue
        printf("    push rbp\n");
        printf("    mov rbp, rsp\n");
        printf("    sub rsp, %d\n", offset_size);

        save_reg_of_argment_to_stack(target_func);

        gen(target_func->body);

        // one value left on the stack as a result of evaluating the expression,
        // so pop it to prevent the stack from overflowing.
        printf("    pop rax\n");

        // epilogue
        // The result of the last expression remains in RAX, so it becomes the return value
        printf("    mov rsp, rbp\n");
        printf("    pop rbp\n");
        printf("    ret\n");
    }
}
