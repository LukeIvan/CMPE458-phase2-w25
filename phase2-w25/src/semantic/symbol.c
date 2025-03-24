#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../include/parser.h"
#include "../../include/lexer.h"
#include "../../include/tokens.h"
#include "../../include/semantic.h"
#include "../../include/symbol.h"

SymbolTable* init_symbol_table() {
    SymbolTable* table = malloc(sizeof(SymbolTable));
    if (table) {
        table->last_symbol = NULL;
        table->current_scope = 0;
    }
    return table;
}

void add_symbol(SymbolTable* table, const char* name, int type, int line) {
    Symbol* new = malloc(sizeof(Symbol));
    if (new) {
        strncpy(new->name, name, sizeof(new->name) - 1);
        new->name[sizeof(new->name) - 1] = '\0';
        new->type = type;
        new->scope_level = table->current_scope;
        new->line_declared = line;
        new->is_initialized = 0; 
        new->next = table->last_symbol;
        table->last_symbol = new;
    }
}

Symbol* lookup_symbol(SymbolTable* table, const char* name) {
    Symbol* curr = table->last_symbol;
    // Since we start iterating through from the last symbol, variable shadowing works as the var in the highest scope (innermost brackets) will
    // be encountered first and returned
    while (curr) {
        if (strcmp(curr->name, name) == 0) {
            return curr;
        }

        curr = curr->next;
    }
    return NULL;
}

void enter_scope(SymbolTable* table) {
    table->current_scope++;
}

void exit_scope(SymbolTable* table) {
    remove_symbols_in_current_scope(table);
    table->current_scope--;
}

void remove_symbols_in_current_scope(SymbolTable* table) {
    // Since we add to the bottom of the table, we only have to remove symbols until we find a symbol with a different scope value
    while (table->last_symbol && table->last_symbol->scope_level == table->current_scope) {
        // Capture current last symbol to free it after incrementing pointer
        Symbol* curr = table->last_symbol;
        table->last_symbol = curr->next;
        free(curr);
    }
}

// Iteratively free all symbols, and then free the table
void free_symbol_table(SymbolTable* table) {
    Symbol* curr = table->last_symbol;
    while (curr) {
        Symbol* next = curr->next;
        free(curr);
        curr = next;
    }
    free(table);
}
