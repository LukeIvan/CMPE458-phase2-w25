/* lexer.c */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "../../include/tokens.h"
#include "../../include/lexer.h"

static int current_line = 1;
static char last_token_type = 'x';

// Keywords table
static struct {
    const char* word;
    TokenType type;
} keywords[] = {
    {"if", TOKEN_IF},
    {"int", TOKEN_INT},
    {"print", TOKEN_PRINT},
    {"while", TOKEN_WHILE},
    {"repeat", TOKEN_REPEAT},
    {"until", TOKEN_UNTIL},
    {"do", TOKEN_DO}
};

static int is_keyword(const char* word) {
    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); i++) {
        if (strcmp(word, keywords[i].word) == 0) {
            return keywords[i].type;
        }
    }
    return 0;
}

void print_error(ErrorType error, int line, const char* lexeme) {
    printf("Lexical Error at line %d: ", line);
    switch(error) {
        case ERROR_INVALID_CHAR:
            printf("Invalid character '%s'\n", lexeme);
            break;
        case ERROR_INVALID_NUMBER:
            printf("Invalid number format\n");
            break;
        case ERROR_CONSECUTIVE_OPERATORS:
            printf("Consecutive operators not allowed\n");
            break;
        case ERROR_INVALID_IDENTIFIER:
            printf("Invalid identifier\n");
            break;
        case ERROR_UNEXPECTED_TOKEN:
            printf("Unexpected token '%s'\n", lexeme);
            break;
        default:
            printf("Unknown error\n");
    }
}

void print_token(Token token) {
    if (token.error != ERROR_NONE) {
        print_error(token.error, token.line, token.lexeme);
        return;
    }

    printf("Token: ");
    switch(token.type) {
        case TOKEN_NUMBER:     printf("NUMBER"); break;
        case TOKEN_OPERATOR:   printf("OPERATOR"); break;
        case TOKEN_IDENTIFIER: printf("IDENTIFIER"); break;
        case TOKEN_EQUALS:     printf("EQUALS"); break;
        case TOKEN_SEMICOLON:  printf("SEMICOLON"); break;
        case TOKEN_LPAREN:     printf("LPAREN"); break;
        case TOKEN_RPAREN:     printf("RPAREN"); break;
        case TOKEN_LBRACE:     printf("LBRACE"); break;
        case TOKEN_RBRACE:     printf("RBRACE"); break;
        case TOKEN_IF:         printf("IF"); break;
        case TOKEN_INT:        printf("INT"); break;
        case TOKEN_PRINT:      printf("PRINT"); break;
        case TOKEN_COMPARISON: printf("COMPARISON"); break;
        case TOKEN_DO:      printf("DO"); break;
        case TOKEN_WHILE:      printf("WHILE"); break;
        case TOKEN_REPEAT:      printf("REPEAT"); break;
        case TOKEN_UNTIL:      printf("UNTIL"); break;
        case TOKEN_EOF:        printf("EOF"); break;
        default:              printf("UNKNOWN");
    }
    printf(" | Lexeme: '%s' | Line: %d\n", token.lexeme, token.line);
}

Token get_next_token(const char* input, int* pos) {
    Token token = {TOKEN_ERROR, "", current_line, ERROR_NONE};
    char c;

    // Skip whitespace and track line numbers
    while ((c = input[*pos]) != '\0' && (c == ' ' || c == '\n' || c == '\t')) {
        if (c == '\n') {
            current_line++;
        }
        (*pos)++;
    }

    if (input[*pos] == '\0') {
        token.type = TOKEN_EOF;
        strcpy(token.lexeme, "EOF");
        return token;
    }

    c = input[*pos];

    if (c == '"') {
        int i = 0;
        (*pos)++; // consume the opening double quote
        c = input[*pos];
        // Read characters until a closing double quote or end-of-file is found
        while (c != '"' && c != '\0') {
            // Optionally handle escape sequences here if needed
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
        }
        if (c == '"') {
            (*pos)++; // consume the closing double quote
            token.lexeme[i] = '\0';
            token.type = TOKEN_STRING;
            return token;
        } else {
            // Unterminated string literal
            token.error = ERROR_INVALID_CHAR; // Or define a specific error like ERROR_INVALID_STRING
            token.lexeme[i] = '\0';
            return token;
        }
    }
    // Handle numbers
    if (isdigit(c)) {
        int i = 0;
        size_t max_lexeme_size = sizeof(token.lexeme) - 1;
        do {
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
        } while (isdigit(c) && (size_t)i < max_lexeme_size);

        token.lexeme[i] = '\0';
        token.type = TOKEN_NUMBER;
        return token;
    }

    // if(c == '='){
    //     token.lexeme[0] = '=';
    //     token.type = TOKEN_EQUALS;
    //     int curr = 1;
    //     (*pos)++;
    //     if(input[*pos] == '='){
    //         (*pos)++;
    //         token.lexeme[curr] = input[*pos];
    //         token.type = TOKEN_COMPARISON;
    //         curr++; 
    //     }
    //     token.lexeme[curr] = '\0';
    //     return token;
    // }

    // if(c == '!'){
    //     token.lexeme[0] = '!';
    //     int curr = 1;
    //     (*pos)++;
    //     if(input[*pos] == '='){
    //         (*pos)++;
    //         token.lexeme[curr] = input[*pos];
    //         token.type = TOKEN_COMPARISON;
    //         token.error = ERROR_NONE;
    //         curr++; 
    //     }
    //     else {
    //         token.type = TOKEN_ERROR;
    //         token.error = ERROR_UNEXPECTED_TOKEN;
    //     }
    //     token.lexeme[curr] = '\0';
    //     return token;
    // }


    if (isalpha(c) || c == '_') {
        int i = 0;
        // Fix: use explicit size comparison with cast to avoid sign comparison warning
        size_t max_lexeme_size = sizeof(token.lexeme) - 1;
        do {
            token.lexeme[i++] = c;
            (*pos)++;
            c = input[*pos];
        } while ((isalnum(c) || c == '_') && (size_t)i < max_lexeme_size);

        token.lexeme[i] = '\0';

        // Check if it's a keyword
        TokenType keyword_type = is_keyword(token.lexeme);
        if (keyword_type) {
            token.type = keyword_type;
        } else {
            token.type = TOKEN_IDENTIFIER;
        }
        return token;
    }

    // Handle operators and delimiters
    (*pos)++;
    token.lexeme[0] = c;
    token.lexeme[1] = '\0';

    switch(c) {
        case '+': case '-': case '*': case '/':
            if (last_token_type == 'o') {
                token.error = ERROR_CONSECUTIVE_OPERATORS;
                return token;
            }
            token.type = TOKEN_OPERATOR;
            last_token_type = 'o';
            break;
        case '=':
            if(input[*pos] == '='){
                (*pos)++;
                token.lexeme[1] = '=';
                token.lexeme[2] = '\0';
                token.type=TOKEN_COMPARISON;
            }
            else{
                token.type = TOKEN_EQUALS;
            }
            break;
        case '!':
            if(input[*pos] == '='){
                (*pos)++;
                token.lexeme[1] = '=';
                token.lexeme[2] = '\0';
                token.type=TOKEN_COMPARISON;
            }
            else{
                token.error = ERROR_INVALID_CHAR;
            }
            break;
        
        case '<': case '>': 
            token.type = TOKEN_COMPARISON;
            break;
        case ';':
            token.type = TOKEN_SEMICOLON;
            break;
        case '(':
            token.type = TOKEN_LPAREN;
            break;
        case ')':
            token.type = TOKEN_RPAREN;
            break;
        case '{':
            token.type = TOKEN_LBRACE;
            break;
        case '}':
            token.type = TOKEN_RBRACE;
            break;
        default:
            token.error = ERROR_INVALID_CHAR;
            break;
    }

    return token;
}

// int main(void) {
//     const char *input = "int x = 123;\n"   // Basic declaration and number
//                        "test_var = 456;\n"  // Identifier and assignment
//                        "print x;\n"         // Keyword and identifier
//                        "if (y > 10) {\n"    // Keywords, identifiers, operators
//                        "    @#$ invalid\n"  // Error case
//                        "    x = ++2;\n"     // Consecutive operator error
//                        "}";

//     printf("Analyzing input:\n%s\n\n", input);
//     int position = 0;
//     Token token;

//     do {
//         token = get_next_token(input, &position);
//         print_token(token);
//     } while (token.type != TOKEN_EOF);

//     return 0;
// }
