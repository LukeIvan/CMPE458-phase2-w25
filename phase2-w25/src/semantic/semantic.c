#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/parser.h"
#include "../../include/lexer.h"
#include "../../include/tokens.h"
#include "../../include/semantic.h"
#include "../../include/symbol.h"

VarType get_type_from_token(Token token);
VarType get_type(ASTNode* node, SymbolTable* table);
void semantic_error(SemanticErrorType error, const char* name, int line);
int analyze_semantics(ASTNode* ast, SymbolTable* table);

// Check a variable declaration
int check_declaration(ASTNode* node, SymbolTable* table);

// Check a variable assignment
int check_assignment(ASTNode* node, SymbolTable* table);

// Check an expression for type correctness
int check_expression(ASTNode* node, SymbolTable* table);

// Check a block of statements, handling scope
int check_block(ASTNode* node, SymbolTable* table);

// Check a condition (e.g., in if statements)
int check_condition(ASTNode* node, SymbolTable* table);

void semantic_error(SemanticErrorType error, const char* name, int line) {
    printf("Semantic Error at line %d: ", line);
    switch (error) {
        case SEM_ERROR_REDECLARED_VARIABLE:
            printf("Variable %s already declared within the same scope. \n", name);
            break;
        case SEM_ERROR_UNDECLARED_VARIABLE:
            printf("Attempting to use an undeclared variable '%s'. \n", name);
            break;
        case SEM_ERROR_UNINITIALIZED_VARIABLE:
            printf("Attempting to use an uninitialized variable '%s'. \n", name);
            break;
        case SEM_ERROR_TYPE_MISMATCH:
            printf("Type mismatch for variable '%s'.\n", name);
            break;
        case SEM_ERROR_UNKNOWN_TYPE:
            printf("Unknown type for variable '%s'.\n", name);
            break;
        default:
            printf("Unknown error\n");
    }
}


int check_declaration(ASTNode* node, SymbolTable* table) {
    Symbol* already_declared = lookup_symbol(table, node->left->token.lexeme);
    if (already_declared != NULL && already_declared->scope_level == table->current_scope) {
        semantic_error(SEM_ERROR_REDECLARED_VARIABLE, node->left->token.lexeme, node->token.line);
        return 1;
    }

    VarType type = get_type_from_token(node->token);
    add_symbol(table, node->left->token.lexeme, type, node->token.line);
    return 0;
}

// This checks the syntax for operations (either comparisons or math)
int check_expression(ASTNode* node, SymbolTable* table){
    if(node->right->type == AST_IDENTIFIER){
        Symbol* right = lookup_symbol(table, node->right->token.lexeme);
        if (right == NULL){
            semantic_error(SEM_ERROR_UNDECLARED_VARIABLE, node->token.lexeme, node->token.line);
            return 1;
        }

        if (right->is_initialized == 0){ 
            semantic_error(SEM_ERROR_UNINITIALIZED_VARIABLE, node->right->token.lexeme, node->token.line);
            return 1;
        };
    }

    if(node->left->type == AST_IDENTIFIER){
        Symbol* left = lookup_symbol(table, node->left->token.lexeme);
        if (left == NULL){
            semantic_error(SEM_ERROR_UNDECLARED_VARIABLE, node->token.lexeme, node->token.line);
            return 1;
        }

        if (left->is_initialized == 0){ 
            semantic_error(SEM_ERROR_UNINITIALIZED_VARIABLE, node->left->token.lexeme, node->token.line);
            return 1;
        };
    }

    // Check Type
    VarType left_type = get_type(node->left, table);
    VarType right_type = get_type(node->right, table);

    if (left_type == TYPE_ERROR || right_type == TYPE_ERROR) {
        return 1;
    }

    if (left_type != right_type) {
        semantic_error(SEM_ERROR_TYPE_MISMATCH, node->token.lexeme, node->token.line);
        return 1;
    }

    
    return 0;
    
}

// Check a variable assignment
int check_assignment(ASTNode* node, SymbolTable* table) {
    Symbol* left = lookup_symbol(table, node->left->token.lexeme);
    if (left == NULL) {
        semantic_error(SEM_ERROR_UNDECLARED_VARIABLE, node->left->token.lexeme, node->token.line);
        return 1;
    }

    VarType right_type = get_type(node->right, table);
    if (right_type == TYPE_ERROR) {
        return 1;
    }
    short int_to_float = ((left->type == TYPE_INT && right_type == TYPE_FLOAT) ||
    (left->type == TYPE_FLOAT && right_type == TYPE_INT));

    if (left->type != right_type && !int_to_float) {
        semantic_error(SEM_ERROR_TYPE_MISMATCH, node->left->token.lexeme, node->token.line);
        return 1;
    }

    switch (left->type) {
        case TYPE_CHAR:
            if (node->right->type == AST_STRING && strlen(node->right->token.lexeme) != 3) {
                // Character literals should be of the form 'c'
                semantic_error(SEM_ERROR_TYPE_MISMATCH, node->left->token.lexeme, node->token.line);
                return 1;
            }
            break;
        case TYPE_STRING:
            if (node->right->type != AST_STRING) {
                semantic_error(SEM_ERROR_TYPE_MISMATCH, node->left->token.lexeme, node->token.line);
                return 1;
            }
            break;
        default:
            break;
    }    
    left->is_initialized = 1;
    return 0;
}

// Check a block of statements, handling scope
int check_block(ASTNode* node, SymbolTable* table);

// Check a condition (e.g., in if statements)
int check_condition(ASTNode* node, SymbolTable* table);


int process_node(ASTNode* node, SymbolTable* table) { 
    // print_ast_node(node);
    int error = 0;

    switch(node->type) { 
        case AST_VARDECL:
            error = check_declaration(node, table);
            break;
        
        case AST_ASSIGN:
            error = check_assignment(node, table);
            break;

        case AST_BINOP:
        case AST_COMPOP:
            error = check_expression(node, table);
            break;
        
        default:
            break;
    }
    if (node->left) error += process_node(node->left, table);
    if (node->right) error += process_node(node->right, table);
    return error;   
}

int analyze_semantics(ASTNode* ast, SymbolTable* table){
    return process_node(ast, table);
}

// Recursively analyzes nodes
VarType get_type(ASTNode* node, SymbolTable* table) {
    if (!node) return TYPE_ERROR; // Null Check
    VarType left;
    VarType right;
    Symbol* symbol;
    switch (node->type) {
        case AST_NUMBER:
            // If constant contains a ., define as a float
            return (strchr(node->token.lexeme, '.') == NULL) ? TYPE_INT : TYPE_FLOAT;
        case AST_STRING:
            return TYPE_STRING;
        case AST_CHAR:
            return TYPE_CHAR;
        case AST_IDENTIFIER:
            symbol = lookup_symbol(table, node->token.lexeme);
            if (symbol == NULL) {
                semantic_error(SEM_ERROR_UNDECLARED_VARIABLE, node->token.lexeme, node->token.line);
                return TYPE_ERROR;
            }
            if (!symbol->is_initialized) {
                semantic_error(SEM_ERROR_UNINITIALIZED_VARIABLE, node->token.lexeme, node->token.line);
                return TYPE_ERROR;
            }
            return symbol->type;
        case AST_BINOP:
            left = get_type(node->left, table);
            right = get_type(node->right, table);
            if (left == right && left != TYPE_STRING) {
                return left;
            }
            //semantic_error(SEM_ERROR_TYPE_MISMATCH, node->token.lexeme, node->token.line);
            return TYPE_ERROR;
        case AST_COMPOP: // Comparisons can be done between any var
            return TYPE_BOOL;
        default:
            return TYPE_ERROR;
    }
}

VarType get_type_from_token(Token token) {
    TokenType token_type = token.type;
    switch (token_type) {
        case TOKEN_INT:
            return TYPE_INT;
        case TOKEN_BOOL:
            return TYPE_BOOL;
        case TOKEN_FLOAT:
            return TYPE_FLOAT;
        case TOKEN_CHAR:
            return TYPE_CHAR;
        case TOKEN_STRING:
            return TYPE_STRING;
        default:
            return TYPE_ERROR;
    }
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
        printf("\nSemantic Analysis Completed Successfully\n");
    }
    else { 
        printf("\nSemantic Analysis Failed With Errors\n");
    }

    print_table(table);
    free_symbol_table(table);
    free_ast(ast);
    free(file_buffer);
    return 0;
}
