#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/parser.h"
#include "../../include/lexer.h"
#include "../../include/tokens.h"
#include "../../include/semantic.h"
#include "../../include/symbol.h"


int analyze_semantics(ASTNode* ast, SymbolTable* table){
    (void)ast;
    (void)table;
    return 1;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Must pass exactly one file to parse");
        return 1;
    }

    char *file_buffer;
    size_t file_size;
    FILE *fp;

    fp = fopen(argv[1], "r");
    if (!fp) {
        fprintf(stderr, "Invalid file path: %s", argv[1]);
        return 1;
    } 
    
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);

    file_buffer = (char *)malloc(file_size + 1);
    if (!file_buffer) {
        fprintf(stderr, "Memory allocation error for file %s", argv[1]);
        fclose(fp);
        return 1;
    }
    memset(file_buffer, 0, file_size + 1);
    size_t bytes_read = fread(file_buffer, 1, file_size, fp);

    if (bytes_read != file_size) {
        fprintf(stderr, "Could not read the whole file");
        fclose(fp);
        free(file_buffer);
        return 1;
    }
    fclose(fp);

    printf("Parsing input:\n%s\n", file_buffer);
    parser_init(file_buffer);
    ASTNode *ast = parse_program();

    printf("\nAbstract Syntax Tree:\n");
    print_ast(ast, 0);

    SymbolTable* table = init_symbol_table();
    int res = analyze_semantics(ast, table);

    if (res == 0) { 
        printf("Semantic Analysis Completed Successfully");
    }
    else { 
        printf("Semantic Analysis Failed With Error Code: %d\n", res);
    }

    free_symbol_table(table);
    free_ast(ast);
    free(file_buffer);
    return 0;
}