#include "9cc.h"

static int count()
{
    static int i = 1;
    return i++;
}

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
            printf("    je  .Lend%d\n", label_num);
            gen(node->then);
        }
        printf(".Lend%d:\n", label_num);
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