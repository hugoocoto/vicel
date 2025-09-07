/*
 * VICEL - Visual Cell editor
 * Copyright (C) 2025  Hugo Coto Florez
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * For questions or support, contact: hugo.coto@member.fsf.org
 */
#include "interpreter.h"
#include "tokens.h"

const char *TOKEN_REPR[] = {
        [LEFT_PARENT] = "LEFT_PARENT",
        [RIGHT_PARENT] = "RIGHT_PARENT",
        [LEFT_BRACE] = "LEFT_BRACE",
        [RIGHT_BRACE] = "RIGHT_BRACE",
        [LEFT_BRACKET] = "LEFT_BRACKET",
        [RIGHT_BRACKET] = "RIGHT_BRACKET",
        [COMMA] = "COMMA",
        [DOT] = "DOT",
        [MINUS] = "MINUS",
        [PLUS] = "PLUS",
        [SEMICOLON] = "SEMICOLON",
        [SLASH] = "SLASH",
        [STAR] = "STAR",
        [BANG] = "BANG",
        [BANG_EQUAL] = "BANG_EQUAL",
        [EQUAL] = "EQUAL",
        [EQUAL_EQUAL] = "EQUAL_EQUAL",
        [GREATER] = "GREATER",
        [GREATER_EQUAL] = "GREATER_EQUAL",
        [LESS] = "LESS",
        [LESS_EQUAL] = "LESS_EQUAL",
        [IDENTIFIER] = "IDENTIFIER",
        [STRING] = "STRING",
        [NUMBER] = "NUMBER",
        [AND] = "AND",
        [CLASS] = "CLASS",
        [ELSE] = "ELSE",
        [FALSE] = "FALSE",
        [FUNCTION] = "FUNCTION",
        [VAR] = "VAR",
        [FOR] = "FOR",
        [IF] = "IF",
        [NIL] = "NIL",
        [OR] = "OR",
        [EXTERN] = "EXTERN",
        [RETURN] = "RETURN",
        [TRUE] = "TRUE",
        [WHILE] = "WHILE",
        [END_OF_FILE] = "END_OF_FILE",
        [BITWISE_AND] = "BITWISE_AND",
        [BITWISE_OR] = "BITWISE_OR",
        [BITWISE_XOR] = "BITWISE_XOR",
        [BITWISE_NOT] = "BITWISE_NOT",
        [PLUS_PLUS] = "PLUS_PLUS",
        [LESS_LESS] = "LESS_LESS",
        [SHIFT_LEFT] = "SHIFT_LEFT",
        [SHIFT_RIGHT] = "SHIFT_RIGHT",
        [FUNC_INPUT] = "FUNC_INPUT",
        [FUNC_OUTPUT] = "FUNC_OUTPUT",
        [ASSERT] = "ASSERT",
        [UNKNOWN] = "UNKNOWN",
};

const char *VALTYPE_REPR[] = {
        [TYPE_NUM] = "NUMBER",
        [TYPE_STR] = "STRING",
        [TYPE_ADDR] = "MEMORY ADDRESS",
        [TYPE_NONE] = "NONE",
        [TYPE_CALLABLE] = "CALLABLE",
        [TYPE_CORE_CALL] = "CORE CALL",
};

const char *STMT_REPR[] = {
        [VARDECLSTMT] = "VARDECLSTMT",
        [BLOCKSTMT] = "BLOCKSTMT",
        [EXPRSTMT] = "EXPRSTMT",
        [ASSERTSTMT] = "ASSERTSTMT",
        [IFSTMT] = "IFSTMT",
        [WHILESTMT] = "WHILESTMT",
        [FUNDECLSTMT] = "FUNDECLSTMT",
        [RETSTMT] = "RETSTMT",
};

const char *EXPR_REPR[] = {
        [ASSIGNEXPR] = "ASSIGNEXPR",
        [BINEXPR] = "BINEXPR",
        [UNEXPR] = "UNEXPR",
        [CALLEXPR] = "CALLEXPR",
        [LITEXPR] = "LITEXPR",
        [VAREXPR] = "VAREXPR",
        [ANDEXPR] = "ANDEXPR",
        [OREXPR] = "OREXPR",
};

