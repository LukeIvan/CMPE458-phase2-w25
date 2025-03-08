/* parser.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/parser.h"
#include "../../include/lexer.h"
#include "../../include/tokens.h"


// TODO 1: Add more parsing function declarations for:
// - if statements: if (condition) { ... }
// - while loops: while (condition) { ... }
// - repeat-until: repeat { ... } until (condition)
// - print statements: print x;
// - blocks: { statement1; statement2; }
// - factorial function: factorial(x)


// Current token being processed
static Token current_token;
static int position = 0;
static const char *source;


static void parse_error(ParseError error, Token token) {
    // TODO 2: Add more error types for:
    // - Missing parentheses
    // - Missing condition
    // - Missing block braces
    // - Invalid operator
    // - Function call errors

    printf("Parse Error at line %d: ", token.line);
    switch (error) {
        case PARSE_ERROR_UNEXPECTED_TOKEN:
            printf("Unexpected token '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_SEMICOLON:
            printf("Missing semicolon after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_IDENTIFIER:
            printf("Expected identifier after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_EQUALS:
            printf("Expected '=' after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_INVALID_EXPRESSION:
            printf("Invalid expression after '%s'\n", token.lexeme);
            break;
        default:
            printf("Unknown error\n");
    }
}

// Get next token
static void advance(void) {
    current_token = get_next_token(source, &position);
}

// Create a new AST node
static ASTNode *create_node(ASTNodeType type) {
    ASTNode *node = malloc(sizeof(ASTNode));
    if (node) {
        node->type = type;
        node->token = current_token;
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

// Match current token with expected type
static int match(TokenType type) {
    return current_token.type == type;
}

// Expect a token type or error
static void expect(TokenType type) {
    if (match(type)) {
        advance();
    } else {
        parse_error(PARSE_ERROR_UNEXPECTED_TOKEN, current_token);
        exit(1); // Or implement error recovery
    }
}

// Forward declarations
static ASTNode *parse_statement(void);
static ASTNode *parse_primary(void);

// TODO 3: Add parsing functions for each new statement type
// static ASTNode* parse_if_statement(void) { ... }
static ASTNode* parse_while(void);
// static ASTNode* parse_repeat_statement(void) { ... }
// static ASTNode* parse_print_statement(void) { ... }
// static ASTNode* parse_block(void) { ... }
// static ASTNode* parse_factorial(void) { ... }

static ASTNode *parse_expression(void);

// Parse variable declaration: int x;
static ASTNode *parse_declaration(void) {
    ASTNode *node = create_node(AST_VARDECL);
    advance(); // consume 'int'

    if (!match(TOKEN_IDENTIFIER)) {
        parse_error(PARSE_ERROR_MISSING_IDENTIFIER, current_token);
        exit(1);
    }

    node->token = current_token;
    advance();

    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, current_token);
        exit(1);
    }
    advance();
    return node;
}

// Parse assignment: x = 5;
static ASTNode *parse_assignment(void) {
    ASTNode *node = create_node(AST_ASSIGN);
    node->left = create_node(AST_IDENTIFIER);
    node->left->token = current_token;
    advance();

    if (!match(TOKEN_EQUALS)) {
        parse_error(PARSE_ERROR_MISSING_EQUALS, current_token);
        exit(1);
    }
    advance();

    node->right = parse_expression();

    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, current_token);
        exit(1);
    }
    advance();
    return node;
}

static ASTNode *parse_block(void){
    if(!match(TOKEN_LBRACE)){
        parse_error(PARSE_ERROR_UNEXPECTED_TOKEN, current_token);
    }

    //ASTNode *node = create_node(AST_BLOCK);
    return NULL;


}

// Parse: if (condition) { ... }
static ASTNode *parse_if(void){
    ASTNode *node = create_node(AST_IF);
    advance();
    
    // Check for '(' after if
    if(!match(TOKEN_LPAREN)){
        parse_error(PARSE_ERROR_UNEXPECTED_TOKEN, current_token);
    }

    // Want to evaluate condition first
    node->left = parse_expression();

    if(!match(TOKEN_RPAREN)){
        parse_error(PARSE_ERROR_UNEXPECTED_TOKEN, current_token);
    }
    advance();

    
    node->right = parse_block();

    return node;
}

static ASTNode *parse_while(void){
    ASTNode *node = create_node(AST_WHILE);
    advance();
    
    // Check for '(' after while
    if(!match(TOKEN_LPAREN)){
        parse_error(PARSE_ERROR_UNEXPECTED_TOKEN, current_token);
    }

    // Want to evaluate condition first
    node->left = parse_expression();

    if(!match(TOKEN_RPAREN)){
        parse_error(PARSE_ERROR_MISSING_BRACKET, current_token);
    }
    advance();

    
    node->right = parse_block();

    return node;
}

// Parse statement
static ASTNode *parse_statement(void) {
    if (match(TOKEN_INT)) {
        return parse_declaration();
    } else if (match(TOKEN_IDENTIFIER)) {
        return parse_assignment();
    } else if (match(TOKEN_IF)) {
        return parse_if();
    } else if (match(TOKEN_WHILE)) {
        return parse_while();
    }
    // TODO 4: Add cases for new statement types
     // else if (match(TOKEN_REPEAT)) return parse_repeat_statement();
    // else if (match(TOKEN_PRINT)) return parse_print_statement();
    // ...

    printf("Syntax Error: Unexpected token\n");
    exit(1);
}

// Parse expression (currently only handles numbers and identifiers)

// TODO 5: Implement expression parsing
// Current expression parsing is basic. Need to implement:
// - Binary operations (+-*/)
// - Comparison operators (<, >, ==, etc.)
// - Operator precedence
// - Parentheses grouping
// - Function calls

/*
    From understanding, operator precedence here is just multi -> addition -> comparison 
    as per cpp reference . com. Should include full table in documentation.
    Parse expression should wrap this and check for right then left parentheses. ? ? ? DANIEL EHLP
*/

static ASTNode *parse_multiplication(void) {
    ASTNode *node = parse_primary();

    while (match(TOKEN_OPERATOR) &&
          (strcmp(current_token.lexeme, "*") == 0 || strcmp(current_token.lexeme, "/") == 0)) {

        Token operator_token = current_token;
        advance();

        ASTNode *new_node = create_node(AST_OPERATOR);
        new_node->token = operator_token;
        new_node->left = node;
        new_node->right = parse_primary();
        node = new_node;
    }
    return node;
}

static ASTNode *parse_addition(void) {
    ASTNode *node = parse_multiplication();

    while (match(TOKEN_OPERATOR) &&
          (strcmp(current_token.lexeme, "+") == 0 || strcmp(current_token.lexeme, "-") == 0)) {

        Token operator_token = current_token;
        advance();

        ASTNode *new_node = create_node(AST_OPERATOR);
        new_node->token = operator_token;
        new_node->left = node;
        new_node->right = parse_multiplication();
        node = new_node;
    }
    return node;
}

static ASTNode *parse_comparison(void) {
    ASTNode *node = parse_addition();

    while (match(TOKEN_OPERATOR) &&
          (strcmp(current_token.lexeme, "<") == 0 ||
           strcmp(current_token.lexeme, ">") == 0 ||
           strcmp(current_token.lexeme, "==") == 0 ||
           strcmp(current_token.lexeme, "!=") == 0)) {

        Token operator_token = current_token;
        advance();

        ASTNode *new_node = create_node(AST_OPERATOR);
        new_node->token = operator_token;
        new_node->left = node;
        new_node->right = parse_addition();
        node = new_node;
    }
    return node;
}

static ASTNode *parse_expression(void)
{
    return parse_comparison();
}

static ASTNode *parse_primary(void) {
    ASTNode *node;

    if (match(TOKEN_NUMBER)) {
        node = create_node(AST_NUMBER);
        advance();
    } else if (match(TOKEN_IDENTIFIER)) {
        node = create_node(AST_IDENTIFIER);
        advance();
    } else if (match(TOKEN_LPAREN)) {
        node = parse_comparison(); // Evaluate Internal Expression
        expect(TOKEN_RPAREN);
        advance();
    } else {
        parse_error(PARSE_ERROR_INVALID_EXPRESSION, current_token);
        exit(1);
    }
    return node;
}

// Parse program (multiple statements)
static ASTNode *parse_program(void) {
    ASTNode *program = create_node(AST_PROGRAM);
    ASTNode *current = program;

    while (!match(TOKEN_EOF)) {
        current->left = parse_statement();
        if (!match(TOKEN_EOF)) {
            current->right = create_node(AST_PROGRAM);
            current = current->right;
        }
    }

    return program;
}

// Initialize parser
void parser_init(const char *input) {
    source = input;
    position = 0;
    advance(); // Get first token
}

// Print AST (for debugging)
void print_ast(ASTNode *node, int level) {
    if (!node) return;

    // Indent based on level
    for (int i = 0; i < level; i++) printf("  ");

    // Print node info
    switch (node->type) {
        case AST_PROGRAM:
            printf("Program\n");
            break;
        case AST_VARDECL:
            printf("VarDecl: %s\n", node->token.lexeme);
            break;
        case AST_ASSIGN:
            printf("Assign\n");
            break;
        case AST_NUMBER:
            printf("Number: %s\n", node->token.lexeme);
            break;
        case AST_IDENTIFIER:
            printf("Identifier: %s\n", node->token.lexeme);
            break;
        case AST_OPERATOR:
            printf("Operator: %s\n", node->token.lexeme);
            break;
       case AST_WHILE:
            printf("While\n"); 
            break;
        // TODO 6: Add cases for new node types
        // case AST_IF: printf("If\n"); break;
        // case AST_REPEAT: printf("Repeat-Until\n"); break;
        // case AST_BLOCK: printf("Block\n"); break;
        // case AST_BINOP: printf("BinaryOp: %s\n", node->token.lexeme); break;
        default:
            printf("Unknown node type\n");
    }

    // Print children
    print_ast(node->left, level + 1);
    print_ast(node->right, level + 1);
}

// Free AST memory
void free_ast(ASTNode *node) {
    if (!node) return;
    free_ast(node->left);
    free_ast(node->right);
    free(node);
}

// Main function for testing
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
        fprintf(stderr, "file AT %s is null", argv[1]);
        free(file_buffer);
        fclose(fp);
        return 1;
    }
    memset(file_buffer, 0, file_size + 1);  // initialize file_buffer to null
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

    free_ast(ast);
    return 0;
}
