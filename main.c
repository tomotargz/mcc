#include <stdio.h>
#include <stdlib.h>

#include "codegen.h"
#include "error.h"

char* openFile(char* path)
{
    FILE* fp = fopen(path, "r");
    if (!fp) {
        error("failed to open file");
        exit(1);
    }
    fseek(fp, 0, SEEK_END);
    size_t size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char* file = calloc(size + 2, sizeof(char));
    fread(file, size, 1, fp);
    file[size] = '\n';
    file[size + 1] = '\0';
    fclose(fp);
    return file;
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        error("error: require 2 args");
        return 1;
    }

    char* path = argv[1];
    char* source = openFile(path);
    Token* tokens = tokenize(source);
    Program* program = parse(tokens);
    generateCode(program);
    return 0;
}
