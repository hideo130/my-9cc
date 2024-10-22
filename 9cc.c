#include "9cc.h"

Node *code[100];
LVar *locals;

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("引数の個数が正しくありません\n");
        return 1;
    }
    locals = calloc(1, sizeof(LVar));
    Token *token = tokenize(argv[1]);
    program(token, code);

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // prologue
    // allocate space for 26 variables
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n");
    for (int i = 0; code[i]; i++)
    {
        gen(code[i]);
        // one value left on the stack as a result of evaluating the expression, 
        // so pop it to prevent the stack from overflowing.
        printf("    pop rax\n");
    }

    // epilogue    
    // The result of the last expression remains in RAX, so it becomes the return value
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");


    return 0;
}