#include "codegen.h"
#include "error.h"

int main(int argc, char** argv)
{
    if (argc != 2) {
        error("error: require 2 args");
        return 1;
    }

    char* source = argv[1];
    Token* tokens = tokenize(source);
    Token* t = tokens;
    Function* functions = parse(tokens);
    generateFunctions(functions);
    return 0;
}
