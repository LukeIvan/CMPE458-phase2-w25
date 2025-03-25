/* parser.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/parser.h"
#include "../../include/lexer.h"
#include "../../include/tokens.h"

// Current token being processed
static Token current_token;
static int position = 0;
static const char *source;
static void advance(void);

static void synchronize(void) {
    while (current_token.type != TOKEN_EOF &&
           current_token.type != TOKEN_SEMICOLON &&
           current_token.type != TOKEN_RBRACE) {
        advance();
    }
    if (current_token.type == TOKEN_SEMICOLON || current_token.type == TOKEN_RBRACE) {
        advance();
    }
}

static void parse_error(ParseError error, Token token) {
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
        case PARSE_ERROR_MISSING_RPAREN:
            printf("Expected right parentheses after '%s'\n", token.lexeme);
            break;
        case PARSE_ERROR_MISSING_UNTIL:
            printf("Expected 'Until', found '%s' instead.\n", token.lexeme);
            break;
        // Additional error types (e.g. missing block bracket) can be added here.
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

// Expect a token type or recover
static void expect(TokenType type, ParseError error) {
    if (match(type)) {
        advance();
    } else {
        parse_error(error, current_token);
        synchronize();
    }
}

static void expect_default(TokenType type) {
    expect(type, PARSE_ERROR_UNEXPECTED_TOKEN);
}

// Forward declarations
static ASTNode *parse_statement(void);
static ASTNode *parse_primary(void);
static ASTNode *parse_expression(void);
static ASTNode *parse_block(void);
static ASTNode *parse_while(void);
static ASTNode *parse_if(void);
static ASTNode *parse_print(void);
static ASTNode *parse_repeat(void);
static ASTNode *parse_factorial(void);

// Parse variable declaration: int x;
static ASTNode *parse_declaration(void) {
    ASTNode *node = create_node(AST_VARDECL);
    node->token = current_token;
    advance(); // consume variable name

    if (!match(TOKEN_IDENTIFIER)) {
        parse_error(PARSE_ERROR_MISSING_IDENTIFIER, current_token);
        synchronize();
        return node;
    }

    // Create a new node for the identifier
    ASTNode *identifier_node = create_node(AST_IDENTIFIER);
    identifier_node->token = current_token;
    node->left = identifier_node;
    advance();

    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, current_token);
        synchronize();
        return node;
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
        synchronize();
        return node;
    }
    advance();

    node->right = parse_expression();

    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, current_token);
        synchronize();
        return node;
    }
    advance();
    return node;
}

static ASTNode *parse_block(void) {
    // Begin block parsing, expecting '{'
    ASTNode *node = create_node(AST_BLOCK);
    if (!match(TOKEN_LBRACE)) {
        parse_error(PARSE_ERROR_UNEXPECTED_TOKEN, current_token);
        synchronize();
        return node;
    }
    advance();

    ASTNode *curr = NULL;
    ASTNode *prev = NULL;

    while (!match(TOKEN_RBRACE) && !match(TOKEN_EOF)) {
        curr = parse_statement();
        if (!node->left) {
            node->left = curr;
        }
        if (prev && !node->right) {
            prev->right = curr;
        }
        prev = curr;
    }
    if (!match(TOKEN_RBRACE)) {
        parse_error(PARSE_ERROR_MISSING_BRACKET, current_token);
        synchronize();
    } else {
        advance();
    }

    return node;
}

// Parse: if (condition) { ... }
static ASTNode *parse_if(void) {
    ASTNode *node = create_node(AST_IF);
    advance();
    
    if (!match(TOKEN_LPAREN)) {
        parse_error(PARSE_ERROR_UNEXPECTED_TOKEN, current_token);
        synchronize();
    } else {
        advance(); // consume '('
    }
    
    node->left = parse_expression();
    
    if (!match(TOKEN_RPAREN)) {
        parse_error(PARSE_ERROR_MISSING_RPAREN, current_token);
        synchronize();
    } else {
        advance(); // consume ')'
    }
    
    node->right = parse_block();
    return node;
}

static ASTNode *parse_while(void) {
    ASTNode *node = create_node(AST_WHILE);
    advance();
    
    if (!match(TOKEN_LPAREN)) {
        parse_error(PARSE_ERROR_UNEXPECTED_TOKEN, current_token);
        synchronize();
    } else {
        advance(); // consume '('
    }
    
    node->left = parse_expression();
    
    if (!match(TOKEN_RPAREN)) {
        parse_error(PARSE_ERROR_MISSING_RPAREN, current_token);
        synchronize();
    } else {
        advance(); // consume ')'
    }
    
    node->right = parse_block();
    return node;
}

static ASTNode *parse_print(void) {
    ASTNode *node = create_node(AST_PRINT);
    advance(); // consume 'print'
    
    node->left = parse_expression();
    
    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, current_token);
        synchronize();
        return node;
    }
    advance(); // consume ';'
    return node;
}

static ASTNode *parse_repeat(void) {
    ASTNode *node = create_node(AST_REPEAT);
    advance();
    
    node->left = parse_block();
    
    if (!match(TOKEN_UNTIL)) {
        parse_error(PARSE_ERROR_MISSING_UNTIL, current_token);
        synchronize();
        return node;
    }
    advance();
    
    node->right = parse_expression();
    
    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, current_token);
        synchronize();
        return node;
    }
    advance();
    return node;
}

static ASTNode *parse_factorial(void) {
    ASTNode *node = create_node(AST_FACTORIAL);
    advance();
    node->right = parse_expression();
    if (!match(TOKEN_SEMICOLON)) {
        parse_error(PARSE_ERROR_MISSING_SEMICOLON, current_token);
        synchronize();
        return node;
    }
    advance();
    return node;
}

// Parse statement
static ASTNode *parse_statement(void) {
    if (match(TOKEN_INT) 
    || match(TOKEN_FLOAT) 
    || match(TOKEN_BOOL)
    || match(TOKEN_CHAR)
    || match(TOKEN_STRING)) {
        return parse_declaration();
    } else if (match(TOKEN_IDENTIFIER)) {
        return parse_assignment();
    } else if (match(TOKEN_IF)) {
        return parse_if();
    } else if (match(TOKEN_WHILE)) {
        return parse_while();
    } else if (match(TOKEN_PRINT)) {
        return parse_print();
    } else if (match(TOKEN_REPEAT)) {
        return parse_repeat();
    } else if (match(TOKEN_FACTORIAL)) {
        return parse_factorial();
    }
    parse_error(PARSE_ERROR_UNEXPECTED_TOKEN, current_token);
    synchronize();
    return create_node(AST_ERROR);
}

// Parse expression (basic support for numbers, identifiers, and simple operations)
static ASTNode *parse_multiplication(void) {
    ASTNode *node = parse_primary();

    while (match(TOKEN_OPERATOR) &&
          (strcmp(current_token.lexeme, "*") == 0 || strcmp(current_token.lexeme, "/") == 0)) {

        Token operator_token = current_token;
        advance();

        ASTNode *new_node = create_node(AST_BINOP);
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

        ASTNode *new_node = create_node(AST_BINOP);
        new_node->token = operator_token;
        new_node->left = node;
        new_node->right = parse_multiplication();
        node = new_node;
    }
    return node;
}

static ASTNode *parse_comparison(void) {
    ASTNode *node = parse_addition();
 
    while (match(TOKEN_COMPARISON) &&
          (strcmp(current_token.lexeme, "<") == 0 ||
           strcmp(current_token.lexeme, ">") == 0 ||
           strcmp(current_token.lexeme, "==") == 0 ||
           strcmp(current_token.lexeme, "!=") == 0)) {

        Token operator_token = current_token;
        advance();

        ASTNode *new_node = create_node(AST_COMPOP);
        new_node->token = operator_token;
        new_node->left = node;
        new_node->right = parse_addition();
        node = new_node;
    }
    return node;
}

static ASTNode *parse_expression(void) {
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
        advance();
        node = parse_comparison();
        expect(TOKEN_RPAREN, PARSE_ERROR_MISSING_RPAREN);
    } else if (match(TOKEN_STRING)) {  // Handle string literals
        node = create_node(AST_STRING);
        advance();
    } else {
        parse_error(PARSE_ERROR_INVALID_EXPRESSION, current_token);
        synchronize();
        return create_node(AST_ERROR);
    }
    return node;
}

// Parse program (multiple statements)
ASTNode *parse_program(void) {
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


void print_ast_node(ASTNode* node) {
    if (!node) {
        printf("NULL node\n");
        return;
    }

    printf("ASTNode Type: ");
    switch (node->type) {
        case AST_PROGRAM:    printf("AST_PROGRAM\n"); break;
        case AST_VARDECL:    printf("AST_VARDECL\n"); break;
        case AST_ASSIGN:     printf("AST_ASSIGN\n"); break;
        case AST_PRINT:      printf("AST_PRINT\n"); break;
        case AST_NUMBER:     printf("AST_NUMBER\n"); break;
        case AST_IDENTIFIER: printf("AST_IDENTIFIER\n"); break;
        case AST_BINOP:      printf("AST_BINOP\n"); break;
        case AST_COMPOP:     printf("AST_COMPOP\n"); break;
        case AST_IF:         printf("AST_IF\n"); break;
        case AST_WHILE:      printf("AST_WHILE\n"); break;
        case AST_BLOCK:      printf("AST_BLOCK\n"); break;
        case AST_STRING:     printf("AST_STRING\n"); break;
        case AST_REPEAT:     printf("AST_REPEAT\n"); break;
        case AST_FACTORIAL:  printf("AST_FACTORIAL\n"); break;
        case AST_ERROR:      printf("AST_ERROR\n"); break;
        default:             printf("UNKNOWN\n");
    }

    printf("Token:\n");
    printf("  Type: ");
    switch (node->token.type) {
        case TOKEN_EOF:         printf("TOKEN_EOF\n"); break;
        case TOKEN_NUMBER:      printf("TOKEN_NUMBER\n"); break;
        case TOKEN_OPERATOR:    printf("TOKEN_OPERATOR\n"); break;
        case TOKEN_IDENTIFIER:  printf("TOKEN_IDENTIFIER\n"); break;
        case TOKEN_EQUALS:      printf("TOKEN_EQUALS\n"); break;
        case TOKEN_SEMICOLON:   printf("TOKEN_SEMICOLON\n"); break;
        case TOKEN_LPAREN:      printf("TOKEN_LPAREN\n"); break;
        case TOKEN_RPAREN:      printf("TOKEN_RPAREN\n"); break;
        case TOKEN_LBRACE:      printf("TOKEN_LBRACE\n"); break;
        case TOKEN_RBRACE:      printf("TOKEN_RBRACE\n"); break;
        case TOKEN_IF:          printf("TOKEN_IF\n"); break;
        case TOKEN_WHILE:       printf("TOKEN_WHILE\n"); break;
        case TOKEN_INT:         printf("TOKEN_INT\n"); break;
        case TOKEN_PRINT:       printf("TOKEN_PRINT\n"); break;
        case TOKEN_COMPARISON:  printf("TOKEN_COMPARISON\n"); break;
        case TOKEN_REPEAT:      printf("TOKEN_REPEAT\n"); break;
        case TOKEN_DO:          printf("TOKEN_DO\n"); break;
        case TOKEN_UNTIL:       printf("TOKEN_UNTIL\n"); break;
        case TOKEN_ERROR:       printf("TOKEN_ERROR\n"); break;
        case TOKEN_FACTORIAL:   printf("TOKEN_FACTORIAL\n"); break;
        case TOKEN_STRING:      printf("TOKEN_STRING\n"); break;
        default:                printf("UNKNOWN\n");
    }
    printf("  Lexeme: %s\n", node->token.lexeme);
    printf("  Line: %d\n", node->token.line);
    printf("  Error: ");
    switch (node->token.error) {
        case ERROR_NONE:                   printf("ERROR_NONE\n"); break;
        case ERROR_INVALID_CHAR:           printf("ERROR_INVALID_CHAR\n"); break;
        case ERROR_INVALID_NUMBER:         printf("ERROR_INVALID_NUMBER\n"); break;
        case ERROR_CONSECUTIVE_OPERATORS:  printf("ERROR_CONSECUTIVE_OPERATORS\n"); break;
        case ERROR_CONSECUTIVE_COMPARISON: printf("ERROR_CONSECUTIVE_COMPARISON\n"); break;
        case ERROR_INVALID_IDENTIFIER:     printf("ERROR_INVALID_IDENTIFIER\n"); break;
        case ERROR_UNEXPECTED_TOKEN:       printf("ERROR_UNEXPECTED_TOKEN\n"); break;
        default:                           printf("UNKNOWN\n");
    }
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
        case AST_BINOP:
            printf("Binary Operator: %s\n", node->token.lexeme);
            break;
        case AST_COMPOP:
            printf("Comparison Operator: %s\n", node->token.lexeme);
            break;
        case AST_IF:
            printf("If: %s\n", node->token.lexeme);
            break;
        case AST_BLOCK:
            printf("Block: %s\n", node->token.lexeme);
            break;
        case AST_WHILE:
            printf("While: %s\n", node->token.lexeme); 
            break;
        case AST_REPEAT:
            printf("Repeat-Until: %s\n", node->token.lexeme);
            break;
        case AST_FACTORIAL:
            printf("Factorial: %s\n", node->token.lexeme);
            break;
        case AST_STRING:
            printf("String: %s\n", node->token.lexeme);
            break;
        case AST_PRINT:
            printf("Print\n");
            break;
        case AST_ERROR:
            printf("Error Node\n");
            break;
        default:
            printf("Unknown node type\n");
    }

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

// // Main function for testing
// int main(int argc, char* argv[]) {
//     if (argc != 2) {
//         fprintf(stderr, "Must pass exactly one file to parse");
//         return 1;
//     }

//     char *file_buffer;
//     size_t file_size;
//     FILE *fp;

//     fp = fopen(argv[1], "r");
//     if (!fp) {
//         fprintf(stderr, "Invalid file path: %s", argv[1]);
//         return 1;
//     } 
    
//     fseek(fp, 0L, SEEK_END);
//     file_size = ftell(fp);
//     rewind(fp);

//     file_buffer = (char *)malloc(file_size + 1);
//     if (!file_buffer) {
//         fprintf(stderr, "Memory allocation error for file %s", argv[1]);
//         fclose(fp);
//         return 1;
//     }
//     memset(file_buffer, 0, file_size + 1);
//     size_t bytes_read = fread(file_buffer, 1, file_size, fp);

//     if (bytes_read != file_size) {
//         fprintf(stderr, "Could not read the whole file");
//         fclose(fp);
//         free(file_buffer);
//         return 1;
//     }
//     fclose(fp);

//     printf("Parsing input:\n%s\n", file_buffer);
//     parser_init(file_buffer);
//     ASTNode *ast = parse_program();

//     printf("\nAbstract Syntax Tree:\n");
//     print_ast(ast, 0);

//     free_ast(ast);
//     free(file_buffer);
//     return 0;
// }
