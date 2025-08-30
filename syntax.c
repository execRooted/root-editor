#include "editor.h"

#define FILE_TYPE_PLAIN 0
#define FILE_TYPE_C 1
#define FILE_TYPE_CPP 2
#define FILE_TYPE_CSHARP 3
#define FILE_TYPE_RUST 4
#define FILE_TYPE_PYTHON 5

void load_keywords(EditorState* state);

#define COLOR_DATA_TYPE 7
#define COLOR_STRING 8
#define COLOR_COMMENT 9
#define COLOR_INTEGER_LITERAL 10
#define COLOR_MODIFIER 11
#define COLOR_PREPROCESSOR 12
#define COLOR_DEFAULT 13
#define COLOR_OPERATOR 14
#define COLOR_DELIMITER 15
#define COLOR_FUNCTION 16
#define COLOR_CONSTANT 17
#define COLOR_ERROR 18
#define COLOR_ERROR_BG 19
#define COLOR_CONTROL_FLOW 20
#define COLOR_STORAGE_CLASS 21
#define COLOR_USER_TYPE 22
#define COLOR_FLOAT_LITERAL 23
#define COLOR_CHAR_LITERAL 24
#define COLOR_POINTER_REF 25
#define COLOR_STDLIB_FUNCTION 26
#define COLOR_STDLIB_TYPE 27

const char* data_types[] = {
    "char", "int", "float", "double", "void", "short", "long", "signed", "unsigned",
    "bool", "string", "byte", "sbyte", "ushort", "uint", "ulong", "decimal",
    "i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f32", "f64",
    "isize", "usize", "str", "String"
};
const int data_types_count = sizeof(data_types) / sizeof(data_types[0]);

const char* modifiers[] = {
    "long", "short", "unsigned", "signed", "mut", "const", "static", "volatile",
    "extern", "register", "auto", "pub", "priv", "ref", "async", "await"
};
const int modifiers_count = sizeof(modifiers) / sizeof(modifiers[0]);

const char* control_flow[] = {
    "if", "else", "for", "while", "do", "switch", "case", "default", "break",
    "continue", "return", "goto", "match", "loop", "try", "catch", "finally",
    "throw", "raise", "pass", "yield", "with", "as"
};
const int control_flow_count = sizeof(control_flow) / sizeof(control_flow[0]);

const char* storage_class[] = {
    "static", "const", "volatile", "extern", "let", "var", "val", "def",
    "class", "struct", "enum", "union", "typedef", "using", "import", "from"
};
const int storage_class_count = sizeof(storage_class) / sizeof(storage_class[0]);

const char* preprocessor[] = {
    "include", "define", "ifdef", "ifndef", "endif", "pragma", "macro",
    "derive", "cfg", "allow", "warn", "deny", "forbid"
};
const int preprocessor_count = sizeof(preprocessor) / sizeof(preprocessor[0]);

const char* constants[] = {
    "NULL", "null", "None", "true", "false", "TRUE", "FALSE", "EXIT_SUCCESS",
    "EXIT_FAILURE", "Some", "Ok", "Err"
};
const int constants_count = sizeof(constants) / sizeof(constants[0]);

const char* stdlib_functions[] = {
    "printf", "scanf", "strlen", "strcpy", "strcat", "strcmp", "malloc", "free",
    "calloc", "realloc", "memcpy", "memset", "fopen", "fclose", "fread", "fwrite",
    "fprintf", "fscanf", "sprintf", "sscanf", "atoi", "atol", "atof", "rand", "srand"
};
const int stdlib_functions_count = sizeof(stdlib_functions) / sizeof(stdlib_functions[0]);

const char* stdlib_types[] = {
    "size_t", "FILE", "time_t", "clock_t", "ptrdiff_t", "int8_t", "int16_t",
    "int32_t", "int64_t", "uint8_t", "uint16_t", "uint32_t", "uint64_t",
    "uintptr_t", "intptr_t", "va_list"
};
const int stdlib_types_count = sizeof(stdlib_types) / sizeof(stdlib_types[0]);

const char* c_operators[] = {
    "+", "-", "*", "/", "%", "=", "==", "!=", "<", ">", "<=", ">=", "&&", "||", "!",
    "&", "|", "^", "~", "<<", ">>", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=",
    "<<=", ">>=", "++", "--", "->", ".", "?", ":"
};

const int c_operator_count = sizeof(c_operators) / sizeof(c_operators[0]);

const char* c_constants[] = {
    "true", "false", "TRUE", "FALSE"
};

const int c_constant_count = sizeof(c_constants) / sizeof(c_constants[0]);

void init_syntax_highlighting(EditorState* state) {
    if (!state->syntax_enabled) return;

    detect_file_type(state);
    load_keywords(state);
}

void detect_file_type(EditorState* state) {
    if (strlen(state->filename) == 0) {
        state->file_type = FILE_TYPE_PLAIN;
        return;
    }

    const char* ext = strrchr(state->filename, '.');
    if (!ext) {
        state->file_type = FILE_TYPE_PLAIN;
        return;
    }

    if (strcmp(ext, ".c") == 0) {
        state->file_type = FILE_TYPE_C;
    } else if (strcmp(ext, ".cpp") == 0 || strcmp(ext, ".cc") == 0 || strcmp(ext, ".cxx") == 0) {
        state->file_type = FILE_TYPE_CPP;
    } else if (strcmp(ext, ".cs") == 0) {
        state->file_type = FILE_TYPE_CSHARP;
    } else if (strcmp(ext, ".rs") == 0) {
        state->file_type = FILE_TYPE_RUST;
    } else if (strcmp(ext, ".py") == 0) {
        state->file_type = FILE_TYPE_PYTHON;
    } else {
        state->file_type = FILE_TYPE_PLAIN;
    }
}

void load_keywords(EditorState* state) {
}

int is_data_type(const char* word) {
    if (!word) return 0;

    for (int i = 0; i < data_types_count; i++) {
        if (strcmp(word, data_types[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_modifier(const char* word) {
    if (!word) return 0;

    for (int i = 0; i < modifiers_count; i++) {
        if (strcmp(word, modifiers[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_control_flow(const char* word) {
    if (!word) return 0;

    for (int i = 0; i < control_flow_count; i++) {
        if (strcmp(word, control_flow[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_storage_class(const char* word) {
    if (!word) return 0;

    for (int i = 0; i < storage_class_count; i++) {
        if (strcmp(word, storage_class[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_preprocessor(const char* word) {
    if (!word) return 0;

    for (int i = 0; i < preprocessor_count; i++) {
        if (strcmp(word, preprocessor[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_constant(const char* word) {
    if (!word) return 0;

    for (int i = 0; i < constants_count; i++) {
        if (strcmp(word, constants[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_stdlib_function(const char* word) {
    if (!word) return 0;

    for (int i = 0; i < stdlib_functions_count; i++) {
        if (strcmp(word, stdlib_functions[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_stdlib_type(const char* word) {
    if (!word) return 0;

    for (int i = 0; i < stdlib_types_count; i++) {
        if (strcmp(word, stdlib_types[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_operator(const char* op) {
    if (!op) return 0;

    for (int i = 0; i < c_operator_count; i++) {
        if (strcmp(op, c_operators[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

int is_bracket(char ch) {
    return ch == '(' || ch == ')' || ch == '[' || ch == ']' || ch == '{' || ch == '}';
}

int is_number(const char* token) {
    if (!token || strlen(token) == 0) return 0;

    int i = 0;
    if (token[0] == '-' || token[0] == '+') i++;

    int has_dot = 0;
    for (; token[i] != '\0'; i++) {
        if (token[i] == '.') {
            if (has_dot) return 0;
            has_dot = 1;
        } else if (!isdigit(token[i])) {
            return 0;
        }
    }
    return 1;
}

void highlight_line(EditorState* state, int line_num, int screen_row, int line_num_width) {
    if (!state->syntax_enabled || state->file_type == FILE_TYPE_PLAIN) {
        char* line = state->lines[line_num];
        mvprintw(screen_row, line_num_width, "%.*s", COLS - line_num_width - 1, line);
        return;
    }

    char* line = state->lines[line_num];
    int line_len = strlen(line);
    int col = line_num_width;
    int max_col = COLS - 1;

    if (line_len == 0) return;

    int i = 0;
    while (i < line_len && col < max_col) {
        char ch = line[i];

        if (ch == ' ' || ch == '\t') {
            mvaddch(screen_row, col++, ch);
            i++;
            continue;
        }

        if (is_bracket(ch)) {
            attron(COLOR_PAIR(COLOR_DELIMITER));
            mvaddch(screen_row, col++, ch);
            attroff(COLOR_PAIR(COLOR_DELIMITER));
            i++;
            continue;
        }

        int found_operator = 0;
        if (i + 1 < line_len) {
            char op2[3] = {ch, line[i+1], '\0'};
            if (is_operator(op2)) {
                if ((ch == '-' && line[i+1] == '>') || ch == '*' || ch == '&') {
                    attron(COLOR_PAIR(COLOR_POINTER_REF));
                    mvaddch(screen_row, col++, ch);
                    mvaddch(screen_row, col++, line[i+1]);
                    attroff(COLOR_PAIR(COLOR_POINTER_REF));
                } else {
                    attron(COLOR_PAIR(COLOR_OPERATOR));
                    mvaddch(screen_row, col++, ch);
                    mvaddch(screen_row, col++, line[i+1]);
                    attroff(COLOR_PAIR(COLOR_OPERATOR));
                }
                i += 2;
                found_operator = 1;
            }
        }
        if (!found_operator && i + 2 < line_len) {
            char op3[4] = {ch, line[i+1], line[i+2], '\0'};
            if (is_operator(op3)) {
                attron(COLOR_PAIR(COLOR_OPERATOR));
                mvaddch(screen_row, col++, ch);
                mvaddch(screen_row, col++, line[i+1]);
                mvaddch(screen_row, col++, line[i+2]);
                attroff(COLOR_PAIR(COLOR_OPERATOR));
                i += 3;
                found_operator = 1;
            }
        }
        if (!found_operator) {
            char op1[2] = {ch, '\0'};
            if (is_operator(op1)) {
                if (ch == '*' || ch == '&') {
                    attron(COLOR_PAIR(COLOR_POINTER_REF));
                    mvaddch(screen_row, col++, ch);
                    attroff(COLOR_PAIR(COLOR_POINTER_REF));
                } else {
                    attron(COLOR_PAIR(COLOR_OPERATOR));
                    mvaddch(screen_row, col++, ch);
                    attroff(COLOR_PAIR(COLOR_OPERATOR));
                }
                i++;
                continue;
            }
        }

        if (ch == '#') {
            attron(COLOR_PAIR(COLOR_PREPROCESSOR));
            int start = i;
            while (i < line_len && !isspace(line[i]) && line[i] != '\0') {
                i++;
            }
            mvprintw(screen_row, col, "%.*s", i - start, &line[start]);
            col += i - start;
            attroff(COLOR_PAIR(COLOR_PREPROCESSOR));
            continue;
        }

        if (ch == '/' && i + 1 < line_len) {
            char next = line[i + 1];
            if (next == '/') {
                attron(COLOR_PAIR(COLOR_COMMENT));
                mvprintw(screen_row, col, "%.*s", line_len - i, &line[i]);
                col += line_len - i;
                attroff(COLOR_PAIR(COLOR_COMMENT));
                break;
            } else if (next == '*') {
                attron(COLOR_PAIR(COLOR_COMMENT));
                mvaddch(screen_row, col++, ch);
                mvaddch(screen_row, col++, next);
                attroff(COLOR_PAIR(COLOR_COMMENT));
                i += 2;
                continue;
            }
        }

        if (ch == '"' || ch == '\'') {
            char quote = ch;
            attron(COLOR_PAIR(COLOR_STRING));
            mvaddch(screen_row, col++, ch);
            i++;
            int start = i;
            while (i < line_len && line[i] != quote) {
                if (line[i] == '\\' && i + 1 < line_len) {
                    i++;
                }
                i++;
            }
            if (start < i) {
                mvprintw(screen_row, col, "%.*s", i - start, &line[start]);
                col += i - start;
            }
            if (i < line_len) {
                mvaddch(screen_row, col++, line[i++]);
            }
            attroff(COLOR_PAIR(COLOR_STRING));
            continue;
        }

        if ((ch >= '0' && ch <= '9') || (ch == '-' && i + 1 < line_len && line[i + 1] >= '0' && line[i + 1] <= '9')) {
            int start = i;
            i++;
            int has_dot = 0;
            while (i < line_len) {
                char c = line[i];
                if (c >= '0' && c <= '9') {
                    i++;
                } else if (c == '.' && !has_dot) {
                    has_dot = 1;
                    i++;
                } else {
                    break;
                }
            }
            if (has_dot) {
                attron(COLOR_PAIR(COLOR_FLOAT_LITERAL));
            } else {
                attron(COLOR_PAIR(COLOR_INTEGER_LITERAL));
            }
            mvprintw(screen_row, col, "%.*s", i - start, &line[start]);
            col += i - start;
            if (has_dot) {
                attroff(COLOR_PAIR(COLOR_FLOAT_LITERAL));
            } else {
                attroff(COLOR_PAIR(COLOR_INTEGER_LITERAL));
            }
            continue;
        }

        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_') {
            int start = i;
            i++;
            while (i < line_len) {
                char c = line[i];
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_') {
                    i++;
                } else {
                    break;
                }
            }

            int len = i - start;
            if (len <= 32) {
                char word[33];
                memcpy(word, &line[start], len);
                word[len] = '\0';

                static char last_keyword[33] = "";
                int is_user_type = 0;

                if (strcmp(word, "struct") == 0 || strcmp(word, "enum") == 0 || strcmp(word, "typedef") == 0) {
                    strcpy(last_keyword, word);
                } else if (strlen(last_keyword) > 0) {
                    is_user_type = 1;
                    last_keyword[0] = '\0';
                }

                if (is_constant(word)) {
                    attron(COLOR_PAIR(COLOR_CONSTANT));
                    mvprintw(screen_row, col, "%s", word);
                    col += len;
                    attroff(COLOR_PAIR(COLOR_CONSTANT));
                } else if (is_data_type(word)) {
                    attron(COLOR_PAIR(COLOR_DATA_TYPE));
                    mvprintw(screen_row, col, "%s", word);
                    col += len;
                    attroff(COLOR_PAIR(COLOR_DATA_TYPE));
                } else if (is_stdlib_type(word)) {
                    attron(COLOR_PAIR(COLOR_STDLIB_TYPE));
                    mvprintw(screen_row, col, "%s", word);
                    col += len;
                    attroff(COLOR_PAIR(COLOR_STDLIB_TYPE));
                } else if (is_user_type) {
                    attron(COLOR_PAIR(COLOR_USER_TYPE));
                    mvprintw(screen_row, col, "%s", word);
                    col += len;
                    attroff(COLOR_PAIR(COLOR_USER_TYPE));
                } else if (is_modifier(word)) {
                    attron(COLOR_PAIR(COLOR_MODIFIER));
                    mvprintw(screen_row, col, "%s", word);
                    col += len;
                    attroff(COLOR_PAIR(COLOR_MODIFIER));
                } else if (is_control_flow(word)) {
                    attron(COLOR_PAIR(COLOR_CONTROL_FLOW));
                    mvprintw(screen_row, col, "%s", word);
                    col += len;
                    attroff(COLOR_PAIR(COLOR_CONTROL_FLOW));
                } else if (is_storage_class(word)) {
                    attron(COLOR_PAIR(COLOR_STORAGE_CLASS));
                    mvprintw(screen_row, col, "%s", word);
                    col += len;
                    attroff(COLOR_PAIR(COLOR_STORAGE_CLASS));
                } else if (is_preprocessor(word)) {
                    attron(COLOR_PAIR(COLOR_PREPROCESSOR));
                    mvprintw(screen_row, col, "%s", word);
                    col += len;
                    attroff(COLOR_PAIR(COLOR_PREPROCESSOR));
                } else {
                    int is_function = 0;
                    if (i < line_len && line[i] == '(') {
                        is_function = 1;
                    }

                    if (is_function) {
                        if (is_stdlib_function(word)) {
                            attron(COLOR_PAIR(COLOR_STDLIB_FUNCTION));
                            mvprintw(screen_row, col, "%s", word);
                            col += len;
                            attroff(COLOR_PAIR(COLOR_STDLIB_FUNCTION));
                        } else {
                            attron(COLOR_PAIR(COLOR_FUNCTION));
                            mvprintw(screen_row, col, "%s", word);
                            col += len;
                            attroff(COLOR_PAIR(COLOR_FUNCTION));
                        }
                    } else {
                        mvprintw(screen_row, col, "%s", word);
                        col += len;
                    }
                }
            } else {
                mvprintw(screen_row, col, "%.*s", len, &line[start]);
                col += len;
            }
            continue;
        }

        mvaddch(screen_row, col++, ch);
        i++;
    }
}

void free_syntax_data(EditorState* state) {
    if (state->keywords) {
        for (int i = 0; i < state->keyword_count; i++) {
            free(state->keywords[i]);
        }
        free(state->keywords);
        state->keywords = NULL;
        state->keyword_count = 0;
    }


    
}