#include "editor.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define FILE_TYPE_PLAIN   0
#define FILE_TYPE_C       1
#define FILE_TYPE_CPP     2
#define FILE_TYPE_CSHARP  3
#define FILE_TYPE_RUST    4
#define FILE_TYPE_PYTHON  5

// Color definitions - UPDATED to match sintax.json
#define COLOR_DATA_TYPE        7   // yellow
#define COLOR_STRING           8   // green
#define COLOR_COMMENT          9   // white
#define COLOR_INTEGER_LITERAL 10   // yellow
#define COLOR_MODIFIER        11   // magenta
#define COLOR_PREPROCESSOR    12   // magenta
#define COLOR_DEFAULT         13   // white
#define COLOR_OPERATOR        14   // cyan
#define COLOR_DELIMITER       15   // white
#define COLOR_FUNCTION        16   // blue
#define COLOR_CONSTANT        17   // yellow
#define COLOR_ERROR           18   // red
#define COLOR_ERROR_BG        19   // black on red
#define COLOR_CONTROL_FLOW    20   // magenta
#define COLOR_STORAGE_CLASS   21   // magenta
#define COLOR_USER_TYPE       22   // yellow
#define COLOR_FLOAT_LITERAL   23   // yellow
#define COLOR_CHAR_LITERAL    24   // green
#define COLOR_POINTER_REF     25   // magenta
#define COLOR_STDLIB_FUNCTION 26   // blue
#define COLOR_STDLIB_TYPE     27   // yellow

// ----------------- Keyword Tables -----------------
const char* data_types[] = {
    "char","int","float","double","void","short","long","signed","unsigned",
    "bool","string","byte","sbyte","ushort","uint","ulong","decimal",
    "i8","i16","i32","i64","u8","u16","u32","u64","f32","f64",
    "isize","usize","str","String"
};
const int data_types_count = sizeof(data_types) / sizeof(data_types[0]);

const char* modifiers[] = {
    "long","short","unsigned","signed","mut","const","static","volatile",
    "extern","register","auto","pub","priv","ref","async","await"
};
const int modifiers_count = sizeof(modifiers) / sizeof(modifiers[0]);

const char* control_flow[] = {
    "if","else","for","while","do","switch","case","default","break",
    "continue","return","goto","match","loop","try","catch","finally",
    "throw","raise","pass","yield","with","as"
};
const int control_flow_count = sizeof(control_flow) / sizeof(control_flow[0]);

const char* storage_class[] = {
    "static","const","volatile","extern","let","var","val","def",
    "class","struct","enum","union","typedef","using","import","from"
};
const int storage_class_count = sizeof(storage_class) / sizeof(storage_class[0]);

const char* preprocessor[] = {
    "include","define","ifdef","ifndef","endif","pragma","macro",
    "derive","cfg","allow","warn","deny","forbid"
};
const int preprocessor_count = sizeof(preprocessor) / sizeof(preprocessor[0]);

const char* constants[] = {
    "NULL","null","None","true","false","TRUE","FALSE","EXIT_SUCCESS",
    "EXIT_FAILURE","Some","Ok","Err"
};
const int constants_count = sizeof(constants) / sizeof(constants[0]);

const char* stdlib_functions[] = {
    "printf","scanf","strlen","strcpy","strcat","strcmp","malloc","free",
    "calloc","realloc","memcpy","memset","fopen","fclose","fread","fwrite",
    "fprintf","fscanf","sprintf","sscanf","atoi","atol","atof","rand","srand"
};
const int stdlib_functions_count = sizeof(stdlib_functions) / sizeof(stdlib_functions[0]);

const char* stdlib_types[] = {
    "size_t","FILE","time_t","clock_t","ptrdiff_t",
    "int8_t","int16_t","int32_t","int64_t",
    "uint8_t","uint16_t","uint32_t","uint64_t",
    "uintptr_t","intptr_t","va_list"
};
const int stdlib_types_count = sizeof(stdlib_types) / sizeof(stdlib_types[0]);

const char* c_operators[] = {
    "+","-","*","/","%","=","==","!=","<",">","<=",">=","&&","||","!","&",
    "|","^","~","<<",">>","+=","-=","*=","/=","%=","&=","|=","^=","<<=",">>=",
    "++","--","->",".","?",":"
};
const int c_operator_count = sizeof(c_operators) / sizeof(c_operators[0]);

// ----------------- Function Prototypes -----------------
void load_keywords(EditorState* state);

// ----------------- Utility Functions -----------------
int is_in_list(const char* word, const char** list, int count) {
    if (!word) return 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(word, list[i]) == 0) return 1;
    }
    return 0;
}

int is_data_type(const char* word)        { return is_in_list(word, data_types, data_types_count); }
int is_modifier(const char* word)         { return is_in_list(word, modifiers, modifiers_count); }
int is_control_flow(const char* word)     { return is_in_list(word, control_flow, control_flow_count); }
int is_storage_class(const char* word)    { return is_in_list(word, storage_class, storage_class_count); }
int is_preprocessor(const char* word)     { return is_in_list(word, preprocessor, preprocessor_count); }
int is_constant(const char* word)         { return is_in_list(word, constants, constants_count); }
int is_stdlib_function(const char* word)  { return is_in_list(word, stdlib_functions, stdlib_functions_count); }
int is_stdlib_type(const char* word)      { return is_in_list(word, stdlib_types, stdlib_types_count); }

int is_operator(const char* op) {
    if (!op) return 0;
    for (int i = 0; i < c_operator_count; i++) {
        if (strcmp(op, c_operators[i]) == 0) return 1;
    }
    return 0;
}

int is_bracket(char ch) {
    return ch=='(' || ch==')' || ch=='[' || ch==']' || ch=='{' || ch=='}';
}

int is_number(const char* token) {
    if (!token || !*token) return 0;
    int i = 0, has_dot = 0;
    if (token[0]=='-' || token[0]=='+') i++;
    for (; token[i]; i++) {
        if (token[i]=='.') { if (has_dot) return 0; has_dot=1; }
        else if (!isdigit(token[i])) return 0;
    }
    return 1;
}

// ----------------- Syntax Handling -----------------
void init_syntax_highlighting(EditorState* state) {
    if (!state->syntax_enabled) return;
    detect_file_type(state);
    load_keywords(state);
}

void detect_file_type(EditorState* state) {
    if (!*state->filename) { state->file_type = FILE_TYPE_PLAIN; return; }
    const char* ext = strrchr(state->filename, '.');
    if (!ext) { state->file_type = FILE_TYPE_PLAIN; return; }
    if (strcmp(ext, ".py") == 0) state->file_type = FILE_TYPE_PYTHON;
    else if (strcmp(ext, ".c") == 0) state->file_type = FILE_TYPE_C;
    else state->file_type = FILE_TYPE_PLAIN;
}

void load_keywords(EditorState* state) { (void)state; }

void free_syntax_data(EditorState* state) {
    if (!state->keywords) return;
    for (int i = 0; i < state->keyword_count; i++) free(state->keywords[i]);
    free(state->keywords);
    state->keywords = NULL;
    state->keyword_count = 0;
}

// ----------------- Highlight Line -----------------
void highlight_line(EditorState* state, int line_num, int screen_row, int line_num_width) {
    if (!state->syntax_enabled || state->file_type == FILE_TYPE_PLAIN) {
        char* line = state->lines[line_num];
        attron(COLOR_PAIR(COLOR_DEFAULT));
        mvprintw(screen_row, line_num_width, "%.*s", COLS - line_num_width - 1, line);
        attroff(COLOR_PAIR(COLOR_DEFAULT));
        return;
    }

    char* line = state->lines[line_num];
    int len = strlen(line);
    int col = line_num_width;

    for (int i = 0; i < len && col < COLS - 1; ) {
        char ch = line[i];

        // Whitespace
        if (isspace(ch)) { mvaddch(screen_row, col++, ch); i++; continue; }

        // Brackets ()[]{} → yellow (COLOR_DELIMITER)
        if (is_bracket(ch)) {
            attron(COLOR_PAIR(COLOR_DELIMITER));
            mvaddch(screen_row, col++, ch);
            attroff(COLOR_PAIR(COLOR_DELIMITER));
            i++;
            continue;
        }

        // Operators
        int op_len = 0;
        for (int l = 3; l > 0; l--) {
            if (i + l <= len) {
                char temp[4] = {0};
                strncpy(temp, &line[i], l);
                if (is_operator(temp)) { op_len = l; break; }
            }
        }
        if (op_len > 0) {
            attron(COLOR_PAIR(COLOR_OPERATOR));
            for (int k = 0; k < op_len; k++) mvaddch(screen_row, col++, line[i+k]);
            attroff(COLOR_PAIR(COLOR_OPERATOR));
            i += op_len;
            continue;
        }

        // Numbers
        if (isdigit(ch) || (ch=='-' && i+1<len && isdigit(line[i+1]))) {
            int start = i;
            int has_dot = 0;
            while (i < len && (isdigit(line[i]) || (line[i]=='.' && !has_dot))) {
                if (line[i]=='.') has_dot=1;
                i++;
            }
            attron(COLOR_PAIR(has_dot ? COLOR_FLOAT_LITERAL : COLOR_INTEGER_LITERAL));
            mvprintw(screen_row, col, "%.*s", i-start, &line[start]);
            col += i-start;
            attroff(COLOR_PAIR(has_dot ? COLOR_FLOAT_LITERAL : COLOR_INTEGER_LITERAL));
            continue;
        }

        // Strings
        if (ch=='"' || ch=='\'') {
            char quote = ch;
            int start = i++;
            while (i < len && line[i] != quote) {
                if (line[i]=='\\' && i+1<len) i++;
                i++;
            }
            if (i < len) i++;
            attron(COLOR_PAIR(COLOR_STRING));
            mvprintw(screen_row, col, "%.*s", i-start, &line[start]);
            col += i-start;
            attroff(COLOR_PAIR(COLOR_STRING));
            continue;
        }

        // Comments
        if ((state->file_type==FILE_TYPE_PYTHON && ch=='#') ||
            (ch=='/' && i+1<len && (line[i+1]=='/' || line[i+1]=='*'))) {
            attron(COLOR_PAIR(COLOR_COMMENT));
            mvprintw(screen_row, col, "%.*s", len-i, &line[i]);
            col += len-i;
            attroff(COLOR_PAIR(COLOR_COMMENT));
            break;
        }

        // Words
        if (isalpha(ch) || ch=='_') {
            int start = i;
            while (i < len && (isalnum(line[i]) || line[i]=='_')) i++;
            int word_len = i-start;
            char word[64] = {0};
            strncpy(word, &line[start], word_len);

            int color = COLOR_DEFAULT;

            // Data types → blue (COLOR_DATA_TYPE)
            if (is_data_type(word)) color = COLOR_DATA_TYPE;
            // Control flow → purple (COLOR_CONTROL_FLOW)
            else if (is_control_flow(word)) color = COLOR_CONTROL_FLOW;
            // Function names → yellow (COLOR_FUNCTION)
            else if (i < len && line[i]=='(') color = COLOR_FUNCTION;
            // Other syntax elements
            else if (is_constant(word)) color = COLOR_CONSTANT;
            else if (is_stdlib_type(word)) color = COLOR_STDLIB_TYPE;
            else if (is_modifier(word)) color = COLOR_MODIFIER;
            else if (is_storage_class(word)) color = COLOR_STORAGE_CLASS;
            else if (is_preprocessor(word)) color = COLOR_PREPROCESSOR;
            else if (is_stdlib_function(word)) color = COLOR_STDLIB_FUNCTION;

            attron(COLOR_PAIR(color));
            mvprintw(screen_row, col, "%.*s", word_len, &line[start]);
            col += word_len;
            attroff(COLOR_PAIR(color));
            continue;
        }

        // Default: just print
        mvaddch(screen_row, col++, ch);
        i++;
    }
}