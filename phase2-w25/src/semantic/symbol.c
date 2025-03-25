#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"
#include "symbol.h"

const char* get_type_name(VarType type);

SymbolTable* init_symbol_table(void) {
    SymbolTable* table = malloc(sizeof(SymbolTable));
    if (table) {
        table->last_symbol = NULL;
        table->current_scope = 0;
    }
    return table;
}

void add_symbol(SymbolTable* table, const char* name, VarType type, int line) {
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
    while (table->last_symbol && table->last_symbol->scope_level == table->current_scope) {
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

void print_table(SymbolTable* table) {
    printf("\n== SYMBOL TABLE DUMP ==\n");
    if (!table || !table->last_symbol) {
        printf("Symbol table is empty.\n");
        return;
    }

    Symbol* current = table->last_symbol;
    int count = 0;
    while (current) {
        printf("Symbol[%d]:\n", count);
        printf(" Name: %s\n", current->name);
        printf(" Type: %s\n", get_type_name(current->type));
        printf(" Scope Level: %d\n", current->scope_level);
        printf(" Line Declared: %d\n", current->line_declared);
        printf(" Initialized: %s\n", current->is_initialized ? "Yes" : "No");
        printf("\n");
        current = current->next;
        count++;
    }

    printf("Total symbols: %d\n", count);
    printf("Current scope level: %d\n", table->current_scope);
    printf("===================\n");
}

