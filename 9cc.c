#include "9cc.h"

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        error("引数の個数が正しくありません\n");
        return 1;
    }
    locals = calloc(1, sizeof(LVar));
    Token *token = tokenize(argv[1]);
    Function *fn = program(token);

    printf(".intel_syntax noprefix\n");

    codegen(fn);

    return 0;
}