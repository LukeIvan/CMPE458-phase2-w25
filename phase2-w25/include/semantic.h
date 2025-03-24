#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "parser.h"
#include "symbol.h"

// Main semantic analysis function

typedef enum {
    SEM_ERROR_NONE,
    SEM_ERROR_UNDECLARED_VARIABLE,
    SEM_ERROR_REDECLARED_VARIABLE,
    SEM_ERROR_TYPE_MISMATCH,
    SEM_ERROR_UNINITIALIZED_VARIABLE,
    SEM_ERROR_INVALID_OPERATION,
    SEM_ERROR_SEMANTIC_ERROR  // Generic semantic error
} SemanticErrorType;

// Report semantic errors
void semantic_error(SemanticErrorType error, const char* name, int line);

int analyze_semantics(ASTNode* ast);

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

#endif