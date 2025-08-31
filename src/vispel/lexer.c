/* VISPEL lexer - Create tokens from plain textle
 *
 * Author: Hugo Coto Florez
 * Repo: github.com/hugocotoflorez/vispel
 *
 * */

#include <assert.h>
#include <ctype.h>
#include <fcntl.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../debug.h"
#include "tokens.h"

/* Location of current char in file buffer */
char *current_ptr = NULL;
/* Location of current token in source */
char *start_line;
char *start_offset;
int line = 1;
/* First token of token list */
vtok *head_token = NULL;

jmp_buf lexer_error;

_Noreturn void
raise_lexer_error()
{
        longjmp(lexer_error, 1);
}

void
print_literal(vtok *tok)
{
        switch (tok->token) {
        case STRING:
                printf(" \"%s\"", tok->str_literal);
                break;
        case IDENTIFIER:
                printf(" `%s`", tok->str_literal);
                break;
        case NUMBER:
                printf(" `%d`", tok->num_literal);
                break;
        case TRUE:
                printf(" `true`");
                break;
        case FALSE:
                printf(" `true`");
                break;
        default:
                break;
        }
}

void
print_token(vtok *tok)
{
        printf("[%2d:%2ld] Token: %s", tok->line, tok->offset, TOKEN_REPR[tok->token]);
        if (tok->token == STRING ||
            tok->token == IDENTIFIER ||
            tok->token == NUMBER ||
            tok->token == TRUE ||
            tok->token == FALSE) {
                print_literal(tok);
        }
        printf("\n");
}

void
print_tokens()
{
        vtok *tok = head_token;
        while (tok) {
                print_token(tok);
                tok = tok->next;
        }
}

void
vspl_free_tokens()
{
        vtok *current = head_token;
        while (current) {
                vtok *next = current->next;
                if (current->token == IDENTIFIER) {
                        free(current->str_literal);
                }
                if (current->token == STRING) {
                        free(current->str_literal);
                }
                free(current);
                current = next;
        }
        head_token = NULL;
}

static vtok *
new_token(vtoktype token)
{
        vtok *tok = malloc(sizeof *tok);
        tok->line = line;
        tok->token = token;
        tok->lexeme = TOKEN_REPR[token];
        tok->offset = start_offset - start_line + 1;
        /* Link token */
        static vtok *prev = NULL;
        tok->next = NULL;
        if (prev) prev->next = tok;
        prev = tok;

        if (head_token == NULL) head_token = tok;
        return tok;
}

void
add_literal_value(vtok *tok, ...)
{
        va_list v;
        va_start(v, tok);
        switch (tok->token) {
        case STRING:
                tok->str_literal = va_arg(v, char *);
                break;
        case IDENTIFIER:
                tok->str_literal = va_arg(v, char *);
                break;
        case NUMBER:
                tok->num_literal = va_arg(v, int);
                break;
        default:
                break;
        }
        va_end(v);
}

#define add_token(token, ...)                          \
        do {                                           \
                vtok *tok = new_token(token);          \
                add_literal_value(tok, ##__VA_ARGS__); \
        } while (0)

static bool
match_word(const char *restrict word)
{
        if (strncmp(word, current_ptr - 1, strlen(word))) return false;
        current_ptr += strlen(word) - 1;
        return true;
}

static bool
match(char expected)
{
        if (*current_ptr != expected) return false;
        ++current_ptr;
        return true;
}

static inline char
get_consume_lex()
{
        return *current_ptr++;
}

static char *
get_string()
{
        char *ret = current_ptr;
        char tmp;

        do {
                tmp = get_consume_lex();
                if (tmp == 0 || tmp == EOF) {
                        report("String delimiter `\"` was never closed!");
                        raise_lexer_error();
                }
        } while (tmp != '"');

        current_ptr[-1] = 0;
        ret = strdup(ret);
        current_ptr[-1] = tmp;
        return ret;
}

static char *
get_identifier()
{
        char *ret = current_ptr - 1;
        char tmp;

        do {
                tmp = get_consume_lex();
        } while (isalnum(tmp) || tmp == '_');

        current_ptr[-1] = 0;
        ret = strdup(ret);
        current_ptr[-1] = tmp;
        --current_ptr;
        return ret;
}

static int
get_number()
{
        int n = current_ptr[-1] - '0';
        while (*current_ptr >= '0' && *current_ptr <= '9') {
                n *= 10;
                n += *current_ptr - '0';
                ++current_ptr;
        }
        return n;
}

static void
get_comment()
{
        for (;;) {
                if (*current_ptr == '\n' ||
                    *current_ptr == EOF ||
                    *current_ptr == 0) break;
                ++current_ptr;
        }
}

int
lex_analize(char *source)
{
        char current;
        head_token = NULL;
        current_ptr = source;
        start_line = current_ptr;

        if (setjmp(lexer_error)) {
                return 1;
        }

        for (;;) {
                start_offset = current_ptr;
                switch (current = get_consume_lex()) {
                case '(':
                        add_token(LEFT_PARENT);
                        break;
                case ')':
                        add_token(RIGHT_PARENT);
                        break;
                case '{':
                        add_token(LEFT_BRACE);
                        break;
                case '}':
                        add_token(RIGHT_BRACE);
                        break;
                case '[':
                        add_token(LEFT_BRACKET);
                        break;
                case ']':
                        add_token(RIGHT_BRACKET);
                        break;
                case ',':
                        add_token(COMMA);
                        break;
                case '.':
                        add_token(DOT);
                        break;
                case '^':
                        add_token(BITWISE_XOR);
                        break;
                case '~':
                        add_token(BITWISE_NOT);
                        break;
                case ';':
                        add_token(SEMICOLON);
                        break;
                case '*':
                        add_token(STAR);
                        break;

                case '/':
                        if (match('/'))
                                get_comment();
                        else
                                add_token(SLASH);
                        break;
                case '&':
                        if (match('&'))
                                add_token(AND);
                        else
                                add_token(BITWISE_AND);
                        break;
                case '+':
                        if (match('+'))
                                add_token(PLUS_PLUS);
                        else
                                add_token(PLUS);
                        break;
                case '-':
                        if (match('-'))
                                add_token(LESS_LESS);
                        else
                                add_token(MINUS);
                        break;
                case '=':
                        if (match('='))
                                add_token(EQUAL_EQUAL);
                        else
                                add_token(EQUAL);
                        break;
                case '!':
                        if (match('='))
                                add_token(BANG_EQUAL);
                        else
                                add_token(BANG);
                        break;
                case '|':
                        if (match('|'))
                                add_token(OR);
                        else
                                add_token(BITWISE_OR);
                        break;


                case '>':
                        if (match('>'))
                                add_token(SHIFT_LEFT);
                        else if (match('='))
                                add_token(GREATER_EQUAL);
                        else
                                add_token(GREATER);
                        break;
                case '<':
                        if (match('<'))
                                add_token(SHIFT_RIGHT);
                        else if (match('='))
                                add_token(LESS_EQUAL);
                        else
                                add_token(LESS);
                        break;

                case '"':
                        add_token(STRING, get_string());
                        break;

                case '0' ... '9':
                        add_token(NUMBER, get_number());
                        break;

                case '\n':
                        start_line = current_ptr;
                        ++line;
                        break;
                case 0:
                case EOF:
                        add_token(END_OF_FILE);
                        break;

                default:
                        if (isspace(current)) break;
                        if (!isalpha(current) && current != '_') {
                                report("[line %d] Invalid lexeme: `%c`",
                                       line, current);
                                add_token(UNKNOWN);
                                break;
                        }

                        else if (match_word("else"))
                                add_token(ELSE);
                        else if (match_word("false"))
                                add_token(FALSE);
                        else if (match_word("func"))
                                add_token(FUNCTION);
                        else if (match_word("var"))
                                add_token(VAR);
                        else if (match_word("for"))
                                add_token(FOR);
                        else if (match_word("if"))
                                add_token(IF);
                        else if (match_word("nil"))
                                add_token(NIL);
                        else if (match_word("extern"))
                                add_token(EXTERN);
                        else if (match_word("return"))
                                add_token(RETURN);
                        else if (match_word("true"))
                                add_token(TRUE);
                        else if (match_word("while"))
                                add_token(WHILE);
                        else if (match_word("assert"))
                                add_token(ASSERT);
                        else
                                add_token(IDENTIFIER, get_identifier());

                        break;
                }

                if (current == EOF || current == 0) break;
        }
        return 0;
}
