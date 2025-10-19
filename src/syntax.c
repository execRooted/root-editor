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
#define FILE_TYPE_JAVASCRIPT 6
#define FILE_TYPE_JAVA    7
#define FILE_TYPE_PHP     8
#define FILE_TYPE_RUBY    9
#define FILE_TYPE_SHELL   10
#define FILE_TYPE_HTML    11
#define FILE_TYPE_CSS     12
#define FILE_TYPE_MAKEFILE 13
#define FILE_TYPE_TYPESCRIPT 14
#define COLOR_DATA_TYPE        7   
#define COLOR_STRING           8   
#define COLOR_COMMENT          9   
#define COLOR_INTEGER_LITERAL 10   
#define COLOR_MODIFIER        11   
#define COLOR_PREPROCESSOR    12   
#define COLOR_DEFAULT         13   
#define COLOR_OPERATOR        14   
#define COLOR_DELIMITER       15   
#define COLOR_FUNCTION        16   
#define COLOR_CONSTANT        17   
#define COLOR_ERROR           18   
#define COLOR_ERROR_BG        19   
#define COLOR_CONTROL_FLOW    20   
#define COLOR_STORAGE_CLASS   21   
#define COLOR_USER_TYPE       22   
#define COLOR_FLOAT_LITERAL   23   
#define COLOR_CHAR_LITERAL    24   
#define COLOR_POINTER_REF     25   
#define COLOR_STDLIB_FUNCTION 26   
#define COLOR_STDLIB_TYPE     27   
#define COLOR_SELECTION       6    



typedef struct {
    char token[64];
    char scope[128];
    int start_pos;
    int end_pos;
    int is_function;
    int is_variable;
    int is_type;
    int is_keyword;
    int is_constant;
} TokenInfo;
const char* data_types[] = {
    "char","int","float","double","void","short","long","signed","unsigned",
    "bool","string","byte","sbyte","ushort","uint","ulong","decimal",
    "i8","i16","i32","i64","u8","u16","u32","u64","f32","f64",
    "isize","usize","str","String"
};
const int data_types_count = sizeof(data_types) / sizeof(data_types[0]);

const char* modifiers[] = {
    "long","short","unsigned","signed","mut","const","static","volatile",
    "extern","register","auto","pub","priv","ref","async","await","unsafe"
};
const int modifiers_count = sizeof(modifiers) / sizeof(modifiers[0]);

const char* control_flow[] = {
    "if","else","for","while","do","switch","case","default","break",
    "continue","return","goto","match","loop","try","catch","finally",
    "throw","raise","pass","yield","with","as","fn","fi"
};
const int control_flow_count = sizeof(control_flow) / sizeof(control_flow[0]);

const char* storage_class[] = {
    "static","const","volatile","extern","let","var","val","def",
    "class","struct","enum","union","typedef","using","import","from",
    "impl","trait","dyn","where","type"
};
const int storage_class_count = sizeof(storage_class) / sizeof(storage_class[0]);

const char* preprocessor[] = {
    "include","define","ifdef","ifndef","endif","pragma","macro",
    "derive","cfg","allow","warn","deny","forbid"
};
const int preprocessor_count = sizeof(preprocessor) / sizeof(preprocessor[0]);

const char* constants[] = {
    "NULL","null","None","true","false","TRUE","FALSE","EXIT_SUCCESS",
    "EXIT_FAILURE","Some","Ok","Err","self","Self","crate","super"
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
void load_keywords(EditorState* state);
int is_in_list(const char* word, const char** list, int count)
{
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

int is_operator(const char* op)
{
    if (!op) return 0;
    for (int i = 0; i < c_operator_count; i++) {
        if (strcmp(op, c_operators[i]) == 0) return 1;
    }
    return 0;
}

int is_bracket(char ch)
{
    return ch=='(' || ch==')' || ch=='[' || ch==']' || ch=='{' || ch=='}';
}

int is_number(const char* token)
{
    if (!token || !*token) return 0;
    int i = 0, has_dot = 0;
    if (token[0]=='-' || token[0]=='+') i++;
    for (; token[i]; i++) {
        if (token[i]=='.') { if (has_dot) return 0; has_dot=1; }
        else if (!isdigit(token[i])) return 0;
    }
    return 1;
}
void init_syntax_highlighting(EditorState* state)
{
    if (!state->syntax_enabled) return;
    detect_file_type(state);
    load_keywords(state);
}

void detect_file_type(EditorState* state)
{
    if (!*state->filename) {
        state->file_type = FILE_TYPE_PLAIN;
        state->syntax_enabled = 0;
        return;
    }
    if (strcmp(state->filename, "Makefile") == 0 || strcmp(state->filename, "makefile") == 0) {
        state->file_type = FILE_TYPE_MAKEFILE;
        state->syntax_enabled = 1;
        return;
    }
    const char* ext = strrchr(state->filename, '.');
    if (!ext) {
        state->file_type = FILE_TYPE_PLAIN;
        state->syntax_enabled = 0;
        return;
    }

    
    if (strcmp(ext, ".c") == 0 || strcmp(ext, ".cpp") == 0 || strcmp(ext, ".cc") == 0 ||
        strcmp(ext, ".cxx") == 0 || strcmp(ext, ".h") == 0 || strcmp(ext, ".hpp") == 0 ||
        strcmp(ext, ".py") == 0 || strcmp(ext, ".java") == 0 || strcmp(ext, ".js") == 0 ||
        strcmp(ext, ".ts") == 0 || strcmp(ext, ".php") == 0 || strcmp(ext, ".rb") == 0 ||
        strcmp(ext, ".rs") == 0 || strcmp(ext, ".go") == 0 || strcmp(ext, ".swift") == 0 ||
        strcmp(ext, ".kt") == 0 || strcmp(ext, ".scala") == 0 || strcmp(ext, ".cs") == 0 ||
        strcmp(ext, ".fs") == 0 || strcmp(ext, ".ml") == 0 || strcmp(ext, ".hs") == 0 ||
        strcmp(ext, ".lua") == 0 || strcmp(ext, ".pl") == 0 || strcmp(ext, ".pm") == 0 ||
        strcmp(ext, ".tcl") == 0 || strcmp(ext, ".sh") == 0 || strcmp(ext, ".bash") == 0 ||
        strcmp(ext, ".zsh") == 0 || strcmp(ext, ".fish") == 0 || strcmp(ext, ".ps1") == 0 ||
        strcmp(ext, ".sql") == 0 || strcmp(ext, ".html") == 0 || strcmp(ext, ".xml") == 0 ||
        strcmp(ext, ".css") == 0 || strcmp(ext, ".scss") == 0 || strcmp(ext, ".sass") == 0 ||
        strcmp(ext, ".less") == 0 || strcmp(ext, ".json") == 0 || strcmp(ext, ".yaml") == 0 ||
        strcmp(ext, ".yml") == 0 || strcmp(ext, ".toml") == 0 || strcmp(ext, ".ini") == 0 ||
        strcmp(ext, ".cfg") == 0 || strcmp(ext, ".conf") == 0 || strcmp(ext, ".md") == 0 ||
        strcmp(ext, ".tex") == 0 || strcmp(ext, ".r") == 0 || strcmp(ext, ".R") == 0 ||
        strcmp(ext, ".m") == 0 || strcmp(ext, ".jl") == 0 || strcmp(ext, ".dart") == 0 ||
        strcmp(ext, ".dart") == 0) {

        
        if (strcmp(ext, ".py") == 0) state->file_type = FILE_TYPE_PYTHON;
        else if (strcmp(ext, ".c") == 0 || strcmp(ext, ".cpp") == 0 || strcmp(ext, ".cc") == 0 ||
                  strcmp(ext, ".cxx") == 0 || strcmp(ext, ".h") == 0 || strcmp(ext, ".hpp") == 0) {
            state->file_type = FILE_TYPE_C;
        } else if (strcmp(ext, ".rs") == 0) {
            state->file_type = FILE_TYPE_RUST;
        } else if (strcmp(ext, ".js") == 0) {
            state->file_type = FILE_TYPE_JAVASCRIPT;
        } else if (strcmp(ext, ".ts") == 0) {
            state->file_type = FILE_TYPE_TYPESCRIPT;
        } else if (strcmp(ext, ".java") == 0) {
            state->file_type = FILE_TYPE_JAVA;
        } else if (strcmp(ext, ".php") == 0) {
            state->file_type = FILE_TYPE_PHP;
        } else if (strcmp(ext, ".rb") == 0) {
            state->file_type = FILE_TYPE_RUBY;
        } else if (strcmp(ext, ".sh") == 0 || strcmp(ext, ".bash") == 0 || strcmp(ext, ".bat") == 0 || strcmp(ext, ".cmd") == 0) {
            state->file_type = FILE_TYPE_SHELL;
        } else if (strcmp(ext, ".html") == 0) {
            state->file_type = FILE_TYPE_HTML;
        } else if (strcmp(ext, ".css") == 0) {
            state->file_type = FILE_TYPE_CSS;
        } else if (strcmp(ext, ".cs") == 0) {
            state->file_type = FILE_TYPE_CSHARP;
        } else {
            state->file_type = FILE_TYPE_PLAIN;
        }

        
        state->syntax_enabled = 1;
    } else {
        state->file_type = FILE_TYPE_PLAIN;
        state->syntax_enabled = 0; 
    }
}

void load_keywords(EditorState* state) { (void)state; }

void free_syntax_data(EditorState* state)
{
    if (!state->keywords) return;
    for (int i = 0; i < state->keyword_count; i++) free(state->keywords[i]);
    free(state->keywords);
    state->keywords = NULL;
    state->keyword_count = 0;
}

void update_syntax_highlighting(EditorState* state)
{
    
    (void)state;
}
void analyze_token_context(EditorState* state, int line_num, int token_start, int token_end, TokenInfo* info)
{
    if (!state->lines[line_num]) return;

    char* line = state->lines[line_num];
    char token[64] = {0};
    int len = token_end - token_start;
    if (len >= sizeof(token)) len = sizeof(token) - 1;
    strncpy(token, &line[token_start], len);
    token[len] = '\0';
    strcpy(info->token, token);
    info->start_pos = token_start;
    info->end_pos = token_end;

    
    info->is_function = 0;
    info->is_variable = 0;
    info->is_type = 0;
    info->is_keyword = 0;
    info->is_constant = 0;

    
    int after_pos = token_end;
    while (after_pos < strlen(line) && isspace(line[after_pos])) after_pos++;

    
    if (after_pos < strlen(line) && line[after_pos] == '(') {
        info->is_function = 1;
        strcpy(info->scope, "entity.name.function");
        return;
    }

    
    int before_pos = token_start - 1;
    while (before_pos >= 0 && isspace(line[before_pos])) before_pos--;

    
    if (state->file_type == FILE_TYPE_C || state->file_type == FILE_TYPE_CPP) {
        if (strcmp(token, "int") == 0 || strcmp(token, "char") == 0 || strcmp(token, "float") == 0 ||
            strcmp(token, "double") == 0 || strcmp(token, "void") == 0 || strcmp(token, "long") == 0 ||
            strcmp(token, "short") == 0 || strcmp(token, "unsigned") == 0 || strcmp(token, "signed") == 0) {
            info->is_type = 1;
            strcpy(info->scope, "storage.type");
            return;
        }
    }

    
    if (strcmp(token, "if") == 0 || strcmp(token, "else") == 0 || strcmp(token, "for") == 0 ||
        strcmp(token, "while") == 0 || strcmp(token, "do") == 0 || strcmp(token, "switch") == 0 ||
        strcmp(token, "case") == 0 || strcmp(token, "default") == 0 || strcmp(token, "break") == 0 ||
        strcmp(token, "continue") == 0 || strcmp(token, "return") == 0 || strcmp(token, "goto") == 0) {
        info->is_keyword = 1;
        strcpy(info->scope, "keyword.control");
        return;
    }

    
    if (strcmp(token, "NULL") == 0 || strcmp(token, "null") == 0 || strcmp(token, "true") == 0 ||
        strcmp(token, "false") == 0 || strcmp(token, "TRUE") == 0 || strcmp(token, "FALSE") == 0) {
        info->is_constant = 1;
        strcpy(info->scope, "constant.language");
        return;
    }

    
    info->is_variable = 1;
    strcpy(info->scope, "variable");
}
int get_vscode_scope_color(EditorState* state, const char* token, int pos_in_line, char* line, int line_num)
{
    
    TokenInfo info;
    analyze_token_context(state, line_num, pos_in_line, pos_in_line + strlen(token), &info);

    
    if (strlen(info.scope) > 0) {
        int color = get_dynamic_color(state, info.scope);
        if (color != COLOR_DEFAULT) return color;
    }

    
    switch (state->file_type) {
        case FILE_TYPE_C:
        case FILE_TYPE_CPP:
            
            if (strcmp(token, "int") == 0 || strcmp(token, "char") == 0 || strcmp(token, "float") == 0 ||
                strcmp(token, "double") == 0 || strcmp(token, "void") == 0 || strcmp(token, "long") == 0 ||
                strcmp(token, "short") == 0 || strcmp(token, "unsigned") == 0 || strcmp(token, "signed") == 0 ||
                strcmp(token, "bool") == 0 || strcmp(token, "size_t") == 0) {
                return get_dynamic_color(state, "storage.type");
            }
            
            if (strcmp(token, "if") == 0 || strcmp(token, "else") == 0 || strcmp(token, "for") == 0 ||
                strcmp(token, "while") == 0 || strcmp(token, "do") == 0 || strcmp(token, "switch") == 0 ||
                strcmp(token, "case") == 0 || strcmp(token, "default") == 0 || strcmp(token, "break") == 0 ||
                strcmp(token, "continue") == 0 || strcmp(token, "return") == 0 || strcmp(token, "goto") == 0) {
                return get_dynamic_color(state, "keyword.control");
            }
            break;
    }

    
    if (info.is_function) {
        return get_dynamic_color(state, "entity.name.function");
    }
    if (info.is_type) {
        return get_dynamic_color(state, "storage.type");
    }
    if (info.is_keyword) {
        return get_dynamic_color(state, "keyword.control");
    }
    if (info.is_constant) {
        return get_dynamic_color(state, "constant.language");
    }

    if (state->file_type == FILE_TYPE_SHELL) {
        if (is_control_flow(token)) return COLOR_MODIFIER;
        if (strcmp(token, "echo") == 0 || strcmp(token, "clear") == 0 || strcmp(token, "cd") == 0 || strcmp(token, "sudo") == 0 || strcmp(token, "rm") == 0 || strcmp(token, "cp") == 0 || strcmp(token, "mkdir") == 0) return COLOR_INTEGER_LITERAL;
    }

    return get_dynamic_color(state, "variable");
}
static int get_hierarchical_color(EditorState* state, const char* scope)
{
    if (!scope) return COLOR_DEFAULT;
    int color = get_dynamic_color(state, scope);
    if (color != COLOR_DEFAULT) return color;

    
    char buf[256];
    strncpy(buf, scope, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    while (1) {
        char* last_dot = strrchr(buf, '.');
        if (!last_dot) break;
        *last_dot = '\0';
        color = get_dynamic_color(state, buf);
        if (color != COLOR_DEFAULT) return color;
    }

    
    if (strstr(scope, "punctuation")) {
        color = get_dynamic_color(state, "punctuation");
        if (color != COLOR_DEFAULT) return color;
        color = get_dynamic_color(state, "keyword.operator");
        if (color != COLOR_DEFAULT) return color;
    }
    if (strstr(scope, "operator")) {
        color = get_dynamic_color(state, "keyword.operator");
        if (color != COLOR_DEFAULT) return color;
    }
    if (strstr(scope, "number") || strstr(scope, "numeric")) {
        color = get_dynamic_color(state, "constant.numeric");
        if (color != COLOR_DEFAULT) return color;
    }
    if (strstr(scope, "string")) {
        color = get_dynamic_color(state, "string");
        if (color != COLOR_DEFAULT) return color;
    }
    if (strstr(scope, "comment")) {
        
        return COLOR_OPERATOR;
    }
    if (strstr(scope, "function")) {
        color = get_dynamic_color(state, "entity.name.function");
        if (color != COLOR_DEFAULT) return color;
    }
    if (strstr(scope, "variable")) {
        color = get_dynamic_color(state, "variable");
        if (color != COLOR_DEFAULT) return color;
    }
    if (strstr(scope, "type") || strstr(scope, "storage")) {
        color = get_dynamic_color(state, "storage.type");
        if (color != COLOR_DEFAULT) return color;
    }
    if (strstr(scope, "keyword")) {
        color = get_dynamic_color(state, "keyword");
        if (color != COLOR_DEFAULT) return color;
        color = get_dynamic_color(state, "keyword.control");
        if (color != COLOR_DEFAULT) return color;
    }
    return COLOR_DEFAULT;
}
int load_syntax_json(EditorState* state);
int parse_json_token_colors(const char* json_content, EditorState* state);
int hex_to_color_pair(const char* hex_color);
int match_scope(const char* token_scope, const char* rule_scope);
int get_dynamic_color(EditorState* state, const char* scope);
const char* get_dynamic_font_style(EditorState* state, const char* scope);
int hex_to_color_pair(const char* hex_color)
{
    if (!hex_color || hex_color[0] != '#') return COLOR_DEFAULT;

    if (strcmp(hex_color, "#d20f39") == 0) return COLOR_DATA_TYPE;
    if (strcmp(hex_color, "#40a02b") == 0) return COLOR_STRING;
    if (strcmp(hex_color, "#9ca0b0") == 0) return COLOR_COMMENT;
    if (strcmp(hex_color, "#fe640b") == 0) return COLOR_INTEGER_LITERAL;
    if (strcmp(hex_color, "#8839ef") == 0) return COLOR_MODIFIER;
    if (strcmp(hex_color, "#179299") == 0) return COLOR_OPERATOR;
    if (strcmp(hex_color, "#1e66f5") == 0) return COLOR_FUNCTION;
    if (strcmp(hex_color, "#df8e1d") == 0) return COLOR_CONSTANT;
    if (strcmp(hex_color, "#4c4f69") == 0) return COLOR_DEFAULT;
    return COLOR_DEFAULT;
}

int match_scope(const char* token_scope, const char* rule_scope)
{
    if (!token_scope || !rule_scope) return 0;
    return strcmp(token_scope, rule_scope) == 0;
}

int get_dynamic_color(EditorState* state, const char* scope)
{
    if (!state->json_loaded || !scope) return COLOR_DEFAULT;
    for (int i = 0; i < state->json_rules_count; i++) {
        if (match_scope(scope, state->json_scopes[i])) {
            return hex_to_color_pair(state->json_colors[i]);
        }
    }
    return COLOR_DEFAULT;
}

const char* get_dynamic_font_style(EditorState* state, const char* scope)
{
    if (!state->json_loaded || !scope) return "";
    for (int i = 0; i < state->json_rules_count; i++) {
        if (match_scope(scope, state->json_scopes[i])) {
            return state->json_font_styles[i];
        }
    }
    return "";
}
static int toggle_block_comment_state_in_line(const char* line, int upto_col, int in_comment)
{
    if (!line) return in_comment;
    int len = (int)strlen(line);
    if (len < 0) len = 0; 
    if (upto_col < 0) upto_col = 0;
    if (upto_col > len) upto_col = len;

    for (int i = 0; i < upto_col; i++) {
        if (!in_comment) {
            if (line[i] == '/' && i + 1 < upto_col && line[i + 1] == '*') {
                in_comment = 1;
                i++; 
            } else if (line[i] == '/' && i + 1 < upto_col && line[i + 1] == '/') {
                
                break;
            }
        } else {
            if (line[i] == '*' && i + 1 < upto_col && line[i + 1] == '/') {
                in_comment = 0;
                i++; 
            }
        }
    }
    return in_comment;
}

static int in_block_comment_before(EditorState* state, int line_num, int upto_col)
{
    if (!state || line_num < 0 || line_num >= state->line_count) {
        return 0;
    }

    int in_comment = 0;
    
    int max_lines_to_check = 137; 
    int lines_checked = 0;

    for (int l = 0; l < line_num && lines_checked < max_lines_to_check; l++) {
        if (!state->lines[l]) continue; 
        const char* ln = state->lines[l];
        int len = (int)strlen(ln);
        if (len < 0) len = 0; 
        in_comment = toggle_block_comment_state_in_line(ln, len, in_comment);
        lines_checked++;
    }

    
    if (lines_checked >= max_lines_to_check) {
        return 0;
    }

    const char* cur = state->lines[line_num];
    if (cur) {
        int len = (int)strlen(cur);
        if (upto_col < 0) upto_col = 0;
        if (upto_col > len) upto_col = len;
        in_comment = toggle_block_comment_state_in_line(cur, upto_col, in_comment);
    }
    return in_comment;
}

static int starts_with_preprocessor(const char* line)
{
    if (!line) return 0;
    int i = 0;
    while (line[i] == ' ' || line[i] == '\t') i++;
    return line[i] == '#';
}
void highlight_line(EditorState* state, int line_num, int screen_row, int line_num_width, int horizontal_scroll_offset)
{
    if (!state->syntax_enabled || !state->syntax_display_enabled || state->file_type == FILE_TYPE_PLAIN) {
        char* line = state->lines[line_num];
        attron(COLOR_PAIR(COLOR_DEFAULT));
        mvprintw(screen_row, line_num_width, "%.*s", COLS - line_num_width - 1, line);
        attroff(COLOR_PAIR(COLOR_DEFAULT));
        return;
    }

    char* line = state->lines[line_num];
    int len = strlen(line);

    
    int start_col = horizontal_scroll_offset;
    if (start_col < 0) start_col = 0;
    if (start_col > len) start_col = len;

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);
    int avail_width = max_x - line_num_width - 1;
    int end_col = start_col + avail_width;
    if (end_col > len) end_col = len;

    
    int is_comment_line = 0;

    
    if (len >= 2) {
        int i = start_col;
        while (i < end_col && isspace(line[i])) i++; 
        if (i + 1 < end_col && line[i] == '/' && line[i + 1] == '/') {
            is_comment_line = 1;
        } else if (line[i] == '#') {
            is_comment_line = 1;
        }
    } else if (len >= 1) {
        int i = start_col;
        while (i < end_col && isspace(line[i])) i++; 
        if (line[i] == '#') {
            is_comment_line = 1;
        }
    }

    
    int in_block_comment = in_block_comment_before(state, line_num, start_col);

    if (is_comment_line) {
        attron(COLOR_PAIR(COLOR_COMMENT));
        mvprintw(screen_row, line_num_width, "%.*s", end_col - start_col, line + start_col);
        attroff(COLOR_PAIR(COLOR_COMMENT));
        return;
    }

    int col = line_num_width;

    for (int i = start_col; i < end_col && col < max_x - 1; ) {
        if (in_block_comment) {
            int close_pos = -1;
            for (int j = i; j < len - 1; j++) {
                if (line[j] == '*' && line[j + 1] == '/') { close_pos = j; break; }
            }
            int seg_len;
            if (close_pos >= 0) {
                seg_len = (close_pos + 2) - i;
                in_block_comment = 0;
            } else {
                seg_len = len - i;
            }
            attron(COLOR_PAIR(COLOR_COMMENT));
            mvprintw(screen_row, col, "%.*s", seg_len, &line[i]);
            col += seg_len;
            attroff(COLOR_PAIR(COLOR_COMMENT));
            if (close_pos >= 0) {
                i = close_pos + 2;
                continue;
            } else {
                break;
            }
        }
        char ch = line[i];

        
        if (isspace(ch)) { mvaddch(screen_row, col++, ch); i++; continue; }

        if (is_bracket(ch)) {
            attron(COLOR_PAIR(COLOR_DELIMITER));
            mvaddch(screen_row, col++, ch);
            attroff(COLOR_PAIR(COLOR_DELIMITER));
            i++;
            continue;
        }

        if ((state->file_type==FILE_TYPE_PYTHON && ch=='#') ||
            (ch=='/' && i+1<len && (line[i+1]=='/' || line[i+1]=='*'))) {
            int comment_end = len;
            if (ch=='/' && i+1 < len && line[i+1]=='*') {
                
                for (int j = i + 2; j < len - 1; j++) {
                    if (line[j] == '*' && line[j+1] == '/') {
                        comment_end = j + 2;
                        break;
                    }
                }
            }
            attron(COLOR_PAIR(COLOR_COMMENT));
            mvprintw(screen_row, col, "%.*s", comment_end - i, &line[i]);
            col += comment_end - i;
            attroff(COLOR_PAIR(COLOR_COMMENT));
            if (comment_end < len) {
                i = comment_end;
                continue;
            } else {
                break;
            }
        }

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

        

        
        if (isalpha(ch) || ch=='_') {
            int start = i;
            while (i < len && (isalnum(line[i]) || line[i]=='_')) i++;
            int word_len = i-start;
            char word[64] = {0};
            strncpy(word, &line[start], word_len);

            int color = COLOR_DEFAULT;

            
            if (is_data_type(word)) color = COLOR_DATA_TYPE;
            
            else if (is_control_flow(word)) color = COLOR_CONTROL_FLOW;
            
            else if (i < len && line[i]=='(') color = COLOR_FUNCTION;
            
            else if (is_constant(word)) color = COLOR_CONSTANT;
            else if (is_stdlib_type(word)) color = COLOR_STDLIB_TYPE;
            else if (is_modifier(word)) color = COLOR_MODIFIER;
            else if (is_storage_class(word)) color = COLOR_STORAGE_CLASS;
            else if (is_preprocessor(word)) color = COLOR_PREPROCESSOR;
            else if (is_stdlib_function(word)) color = COLOR_STDLIB_FUNCTION;

            if (state->file_type == FILE_TYPE_SHELL) {
                if (is_control_flow(word)) color = COLOR_MODIFIER;
                else if (strcmp(word, "echo") == 0 || strcmp(word, "clear") == 0 || strcmp(word, "cd") == 0 || strcmp(word, "sudo") == 0 || strcmp(word, "rm") == 0 || strcmp(word, "cp") == 0 || strcmp(word, "mkdir") == 0) color = COLOR_INTEGER_LITERAL;
            }

            attron(COLOR_PAIR(color));
            mvprintw(screen_row, col, "%.*s", word_len, &line[start]);
            col += word_len;
            attroff(COLOR_PAIR(color));
            continue;
        }

        
        mvaddch(screen_row, col++, ch);
        i++;
    }
}
void highlight_line_segment(EditorState* state, int line_num, int screen_row, int line_num_width, int start_col, int max_len)
{
    if (!state || line_num < 0 || line_num >= state->line_count) return;
    char* line = state->lines[line_num];
    if (!line) return;

    int len = (int)strlen(line);
    if (start_col < 0) start_col = 0;
    if (start_col > len) start_col = len;

    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    const int text_margin = 0;
    const int text_start_col = line_num_width + text_margin;
    int avail_screen_w = max_x - text_start_col - 1;
    if (avail_screen_w < 1) avail_screen_w = 1;

    int usable = max_len;
    if (usable > avail_screen_w) usable = avail_screen_w;

    int end_col = start_col + usable;
    if (end_col > len) end_col = len;

    
    int has_selection = state->select_mode && line_num >= state->select_start_y && line_num <= state->select_end_y;
    int sel_start_x = has_selection ? (line_num == state->select_start_y ? state->select_start_x : 0) : -1;
    int sel_end_x   = has_selection ? (line_num == state->select_end_y   ? state->select_end_x   : len) : -1;

    
    int is_preproc = starts_with_preprocessor(line);
    if (is_preproc) {
        int print_len = end_col - start_col;
        if (print_len > 0) {
            int pre_color = get_hierarchical_color(state, "keyword.control");
            if (has_selection) {
                int vis_sel_start = sel_start_x - start_col;
                int vis_sel_end = sel_end_x - start_col;
                if (vis_sel_start < 0) vis_sel_start = 0;
                if (vis_sel_end > print_len) vis_sel_end = print_len;
                if (vis_sel_end < 0) vis_sel_end = 0;

                if (vis_sel_start > 0) {
                    attron(COLOR_PAIR(pre_color));
                    mvprintw(screen_row, text_start_col, "%.*s", vis_sel_start, line + start_col);
                    attroff(COLOR_PAIR(pre_color));
                }
                if (vis_sel_end > vis_sel_start) {
                    attron(COLOR_PAIR(COLOR_SELECTION));
                    mvprintw(screen_row, text_start_col + vis_sel_start, "%.*s",
                             vis_sel_end - vis_sel_start, line + start_col + vis_sel_start);
                    attroff(COLOR_PAIR(COLOR_SELECTION));
                }
                if (vis_sel_end < print_len) {
                    attron(COLOR_PAIR(pre_color));
                    mvprintw(screen_row, text_start_col + vis_sel_end, "%.*s",
                             print_len - vis_sel_end, line + start_col + vis_sel_end);
                    attroff(COLOR_PAIR(pre_color));
                }
            } else {
                attron(COLOR_PAIR(pre_color));
                mvprintw(screen_row, text_start_col, "%.*s", print_len, line + start_col);
                attroff(COLOR_PAIR(pre_color));
            }
        }
        return;
    }

    
    int in_block_comment = in_block_comment_before(state, line_num, start_col);

    int col = text_start_col;

    
    int comment_start = -1;
    for (int check_i = start_col; check_i < end_col - 1; check_i++) {
        if (line[check_i] == '/' && line[check_i + 1] == '/') {
            comment_start = check_i;
            break;
        }
    }

    
    if (comment_start >= start_col) {
        
        int normal_end = comment_start;
        for (int i = start_col; i < normal_end && col < max_x - 1; ) {
            char ch = line[i];
            int is_selected = has_selection && i >= sel_start_x && i < sel_end_x;

            
            if (in_block_comment) {
                int close_pos = -1;
                for (int j = i; j < normal_end - 1; j++) {
                    if (line[j] == '*' && line[j + 1] == '/') { close_pos = j; break; }
                }
                int seg_len;
                if (close_pos >= 0) {
                    seg_len = (close_pos + 2) - i;
                    in_block_comment = 0;
                } else {
                    seg_len = normal_end - i;
                }
                int cmt_color = get_hierarchical_color(state, "comment");
                attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : cmt_color));
                mvprintw(screen_row, col, "%.*s", seg_len, &line[i]);
                col += seg_len;
                i += seg_len;
                continue;
            }

            
            if (isspace((unsigned char)ch)) {
                attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : COLOR_DEFAULT));
                mvaddch(screen_row, col++, ch);
                attroff(COLOR_PAIR(is_selected ? COLOR_SELECTION : COLOR_DEFAULT));
                i++;
                continue;
            }

            
            if (ch == '(' || ch == ')' || ch == '[' || ch == ']' || ch == '{' || ch == '}' ||
                ch == '.' || ch == ',' || ch == ';' || ch == '<' || ch == '>') {
                const char* scope = "punctuation";
                if (ch == '(' || ch == ')') scope = "punctuation.parenthesis";
                else if (ch == '[' || ch == ']') scope = "punctuation.bracket";
                else if (ch == '{' || ch == '}') scope = "punctuation.curly";
                else if (ch == '<' || ch == '>') scope = "punctuation.angle";
                else if (ch == '.') scope = "punctuation.dot";
                else if (ch == ',') scope = "punctuation.comma";
                else if (ch == ';') scope = "punctuation.semicolon";

                int pcolor = get_hierarchical_color(state, scope);
                if (pcolor == COLOR_DEFAULT) pcolor = get_hierarchical_color(state, "punctuation");
                if (pcolor == COLOR_DEFAULT) pcolor = get_hierarchical_color(state, "keyword.operator");

                attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : pcolor));
                mvaddch(screen_row, col++, ch);
                attroff(COLOR_PAIR(is_selected ? COLOR_SELECTION : pcolor));
                i++;
                continue;
            }

            
            int op_len = 0;
            if (ch == '=' || ch == '!' || ch == '<' || ch == '>' || ch == '+' || ch == '-' ||
                ch == '*' || ch == '/' || ch == '%' || ch == '&' || ch == '|' || ch == '^' ||
                ch == '~' || ch == '?' || ch == ':') {
                if (i + 1 < normal_end) {
                    char next = line[i + 1];
                    if ((ch == '=' && (next == '=' || next == '+' || next == '-' || next == '*' || next == '/' || next == '%' || next == '&' || next == '|' || next == '^' || next == '<' || next == '>')) ||
                        (ch == '+' && next == '+') ||
                        (ch == '-' && next == '-') ||
                        (ch == '&' && next == '&') ||
                        (ch == '|' && next == '|') ||
                        (ch == '<' && (next == '<' || next == '=')) ||
                        (ch == '>' && (next == '>' || next == '='))) {
                        op_len = 2;
                    } else if (i + 2 < normal_end && ch == '>' && line[i+1] == '>' && line[i+2] == '=') {
                        op_len = 3;
                    } else if (i + 2 < normal_end && ch == '<' && line[i+1] == '<' && line[i+2] == '=') {
                        op_len = 3;
                    }
                }
                if (op_len == 0) op_len = 1;

                int op_color = get_hierarchical_color(state, "keyword.operator");
                if (op_color == COLOR_DEFAULT) op_color = get_hierarchical_color(state, "punctuation");
                attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : op_color));
                for (int k = 0; k < op_len; k++) {
                    mvaddch(screen_row, col++, line[i + k]);
                }
                attroff(COLOR_PAIR(is_selected ? COLOR_SELECTION : op_color));
                i += op_len;
                continue;
            }

            
            if (isdigit((unsigned char)ch) || (ch == '-' && i + 1 < len && isdigit((unsigned char)line[i + 1]))) {
                int num_start = i;
                if (ch == '0' && i + 1 < len && (line[i + 1] == 'x' || line[i + 1] == 'X')) {
                    i += 2;
                    while (i < normal_end && isxdigit((unsigned char)line[i])) i++;
                } else if (ch == '0' && i + 1 < len && (line[i + 1] == 'b' || line[i + 1] == 'B')) {
                    i += 2;
                    while (i < normal_end && (line[i] == '0' || line[i] == '1')) i++;
                } else if (ch == '0' && i + 1 < len && isdigit((unsigned char)line[i + 1])) {
                    i++;
                    while (i < normal_end && line[i] >= '0' && line[i] <= '7') i++;
                } else {
                    int has_dot = 0, has_e = 0;
                    while (i < normal_end && (isdigit((unsigned char)line[i]) ||
                            (line[i] == '.' && !has_dot) ||
                            ((line[i] == 'e' || line[i] == 'E') && !has_e))) {
                        if (line[i] == '.') has_dot = 1;
                        if (line[i] == 'e' || line[i] == 'E') {
                            has_e = 1;
                            if (i + 1 < normal_end && (line[i + 1] == '+' || line[i + 1] == '-')) i++;
                        }
                        i++;
                    }
                }
                int seg_len = i - num_start;
                int num_color = get_hierarchical_color(state, "constant.numeric");
                attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : num_color));
                mvprintw(screen_row, col, "%.*s", seg_len, &line[num_start]);
                col += seg_len;
                continue;
            }

            
            if (ch == '"' || ch == '\'') {
                char quote = ch;
                int start = i++;
                int escaped = 0;
                while (i < normal_end) {
                    if (line[i] == '\\' && !escaped) { escaped = 1; i++; }
                    else if (line[i] == quote && !escaped) { i++; break; }
                    else { escaped = 0; i++; }
                }
                int str_color = get_hierarchical_color(state, "string");
                attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : str_color));
                mvprintw(screen_row, col, "%.*s", i - start, &line[start]);
                col += (i - start);
                continue;
            }

            
            if (isalpha((unsigned char)ch) || ch == '_' || ch == '$') {
                int start = i;
                while (i < normal_end && (isalnum((unsigned char)line[i]) || line[i] == '_' || line[i] == '$')) i++;
                int word_len = i - start;
                char word[64] = {0};
                if (word_len < (int)sizeof(word)) {
                    strncpy(word, &line[start], word_len);
                }

                int color = get_vscode_scope_color(state, word, start, line, line_num);
                if (color == COLOR_DEFAULT) {
                    color = get_hierarchical_color(state, "variable");
                }

                attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : color));
                mvprintw(screen_row, col, "%.*s", word_len, &line[start]);
                col += word_len;
                continue;
            }

            
            attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : COLOR_DEFAULT));
            mvaddch(screen_row, col++, ch);
            attroff(COLOR_PAIR(is_selected ? COLOR_SELECTION : COLOR_DEFAULT));
            i++;
        }

        
        int comment_len = end_col - comment_start;
        if (comment_len > 0) {
            int is_selected_comment = has_selection && comment_start >= sel_start_x && comment_start < sel_end_x;
            attron(COLOR_PAIR(is_selected_comment ? COLOR_SELECTION : COLOR_OPERATOR));
            mvprintw(screen_row, col, "%.*s", comment_len, &line[comment_start]);
            col += comment_len;
            attroff(COLOR_PAIR(is_selected_comment ? COLOR_SELECTION : COLOR_OPERATOR));
        }
        return;
    }

    for (int i = start_col; i < end_col && col < max_x - 1; ) {
        char ch = line[i];

        
        int is_selected = has_selection && i >= sel_start_x && i < sel_end_x;

        
        if (in_block_comment) {
            int close_pos = -1;
            for (int j = i; j < end_col - 1; j++) {
                if (line[j] == '*' && line[j + 1] == '/') { close_pos = j; break; }
            }
            int seg_len;
            if (close_pos >= 0) {
                seg_len = (close_pos + 2) - i;
                in_block_comment = 0; 
            } else {
                seg_len = end_col - i;
            }
            int cmt_color = get_hierarchical_color(state, "comment");
            const char* font_style = get_dynamic_font_style(state, "comment");

            
            if (strcmp(font_style, "italic") == 0) {
                attron(A_DIM);  
            }

            attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : cmt_color));
            mvprintw(screen_row, col, "%.*s", seg_len, &line[i]);
            col += seg_len;
            i += seg_len;
            attroff(COLOR_PAIR(is_selected ? COLOR_SELECTION : cmt_color));

            
            if (strcmp(font_style, "italic") == 0) {
                attroff(A_DIM);
            }
            continue;
        }

        
        if (isspace((unsigned char)ch)) {
            attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : COLOR_DEFAULT));
            mvaddch(screen_row, col++, ch);
            attroff(COLOR_PAIR(is_selected ? COLOR_SELECTION : COLOR_DEFAULT));
            i++;
            continue;
        }

        
        if (ch == '(' || ch == ')' || ch == '[' || ch == ']' || ch == '{' || ch == '}' ||
            ch == '.' || ch == ',' || ch == ';' || ch == '<' || ch == '>') {
            const char* scope = "punctuation";
            if (ch == '(' || ch == ')') scope = "punctuation.parenthesis";
            else if (ch == '[' || ch == ']') scope = "punctuation.bracket";
            else if (ch == '{' || ch == '}') scope = "punctuation.curly";
            else if (ch == '<' || ch == '>') scope = "punctuation.angle";
            else if (ch == '.') scope = "punctuation.dot";
            else if (ch == ',') scope = "punctuation.comma";
            else if (ch == ';') scope = "punctuation.semicolon";

            int pcolor = get_hierarchical_color(state, scope);
            if (pcolor == COLOR_DEFAULT) pcolor = get_hierarchical_color(state, "punctuation");
            if (pcolor == COLOR_DEFAULT) pcolor = get_hierarchical_color(state, "keyword.operator");

            attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : pcolor));
            mvaddch(screen_row, col++, ch);
            attroff(COLOR_PAIR(is_selected ? COLOR_SELECTION : pcolor));
            i++;
            continue;
        }

        
        int op_len = 0;
        if (ch == '=' || ch == '!' || ch == '<' || ch == '>' || ch == '+' || ch == '-' ||
            ch == '*' || ch == '/' || ch == '%' || ch == '&' || ch == '|' || ch == '^' ||
            ch == '~' || ch == '?' || ch == ':') {
            if (i + 1 < end_col) {
                char next = line[i + 1];
                if ((ch == '=' && (next == '=' || next == '+' || next == '-' || next == '*' || next == '/' || next == '%' || next == '&' || next == '|' || next == '^' || next == '<' || next == '>')) ||
                    (ch == '+' && next == '+') ||
                    (ch == '-' && next == '-') ||
                    (ch == '&' && next == '&') ||
                    (ch == '|' && next == '|') ||
                    (ch == '<' && (next == '<' || next == '=')) ||
                    (ch == '>' && (next == '>' || next == '='))) {
                    op_len = 2;
                } else if (i + 2 < end_col && ch == '>' && line[i+1] == '>' && line[i+2] == '=') {
                    op_len = 3;
                } else if (i + 2 < end_col && ch == '<' && line[i+1] == '<' && line[i+2] == '=') {
                    op_len = 3;
                }
            }
            if (op_len == 0) op_len = 1;

            
            if (ch == '/' && i + 1 < end_col && line[i + 1] == '/') {
                int cmt_color = get_hierarchical_color(state, "comment");
                const char* font_style = get_dynamic_font_style(state, "comment");

                
                if (strcmp(font_style, "italic") == 0) {
                    attron(A_DIM);  
                }

                attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : cmt_color));
                mvprintw(screen_row, col, "%.*s", end_col - i, &line[i]);
                col += (end_col - i);
                i = end_col;
                attroff(COLOR_PAIR(is_selected ? COLOR_SELECTION : cmt_color));

                
                if (strcmp(font_style, "italic") == 0) {
                    attroff(A_DIM);
                }
                break;
            }
            if (ch == '/' && i + 1 < end_col && line[i + 1] == '*') {
                int j = i + 2;
                int closed = -1;
                while (j < end_col - 1) {
                    if (line[j] == '*' && line[j + 1] == '/') { closed = j; break; }
                    j++;
                }
                int cmt_color = get_hierarchical_color(state, "comment");
                if (closed >= 0) {
                    int cmt_len = (closed + 2) - i;
                    attron(A_DIM);
                    attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : cmt_color));
                    mvprintw(screen_row, col, "%.*s", cmt_len, &line[i]);
                    col += cmt_len;
                    i += cmt_len;
                    attroff(COLOR_PAIR(is_selected ? COLOR_SELECTION : cmt_color));
                    attroff(A_DIM);
                } else {
                    
                    int cmt_len = end_col - i;
                    attron(A_DIM);
                    attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : cmt_color));
                    mvprintw(screen_row, col, "%.*s", cmt_len, &line[i]);
                    col += cmt_len;
                    i = end_col;
                    attroff(COLOR_PAIR(is_selected ? COLOR_SELECTION : cmt_color));
                    attroff(A_DIM);
                    
                }
                continue;
            }

            int op_color = get_hierarchical_color(state, "keyword.operator");
            if (op_color == COLOR_DEFAULT) op_color = get_hierarchical_color(state, "punctuation");
            attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : op_color));
            for (int k = 0; k < op_len; k++) {
                mvaddch(screen_row, col++, line[i + k]);
            }
            attroff(COLOR_PAIR(is_selected ? COLOR_SELECTION : op_color));
            i += op_len;
            continue;
        }

        
        if (isdigit((unsigned char)ch) || (ch == '-' && i + 1 < len && isdigit((unsigned char)line[i + 1]))) {
            int num_start = i;
            if (ch == '0' && i + 1 < len && (line[i + 1] == 'x' || line[i + 1] == 'X')) {
                i += 2;
                while (i < end_col && isxdigit((unsigned char)line[i])) i++;
            } else if (ch == '0' && i + 1 < len && (line[i + 1] == 'b' || line[i + 1] == 'B')) {
                i += 2;
                while (i < end_col && (line[i] == '0' || line[i] == '1')) i++;
            } else if (ch == '0' && i + 1 < len && isdigit((unsigned char)line[i + 1])) {
                i++;
                while (i < end_col && line[i] >= '0' && line[i] <= '7') i++;
            } else {
                int has_dot = 0, has_e = 0;
                while (i < end_col && (isdigit((unsigned char)line[i]) ||
                        (line[i] == '.' && !has_dot) ||
                        ((line[i] == 'e' || line[i] == 'E') && !has_e))) {
                    if (line[i] == '.') has_dot = 1;
                    if (line[i] == 'e' || line[i] == 'E') {
                        has_e = 1;
                        if (i + 1 < end_col && (line[i + 1] == '+' || line[i + 1] == '-')) i++;
                    }
                    i++;
                }
            }
            int seg_len = i - num_start;
            int num_color = get_hierarchical_color(state, "constant.numeric");
            attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : num_color));
            mvprintw(screen_row, col, "%.*s", seg_len, &line[num_start]);
            col += seg_len;
            attroff(COLOR_PAIR(is_selected ? COLOR_SELECTION : num_color));
            continue;
        }

        
        if (ch == '"' || ch == '\'') {
            char quote = ch;
            int start = i++;
            int escaped = 0;
            while (i < end_col) {
                if (line[i] == '\\' && !escaped) { escaped = 1; i++; }
                else if (line[i] == quote && !escaped) { i++; break; }
                else { escaped = 0; i++; }
            }
            int str_color = get_hierarchical_color(state, "string");
            attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : str_color));
            mvprintw(screen_row, col, "%.*s", i - start, &line[start]);
            col += (i - start);
            attroff(COLOR_PAIR(is_selected ? COLOR_SELECTION : str_color));
            continue;
        }

        
        if ((state->file_type == FILE_TYPE_PYTHON ||
              state->file_type == FILE_TYPE_RUBY ||
              state->file_type == FILE_TYPE_SHELL ||
              state->file_type == FILE_TYPE_MAKEFILE) && ch == '#') {
            int cmt_color = get_hierarchical_color(state, "comment");
            const char* font_style = get_dynamic_font_style(state, "comment");

            
            if (strcmp(font_style, "italic") == 0) {
                attron(A_DIM);  
            }

            attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : cmt_color));
            mvprintw(screen_row, col, "%.*s", end_col - i, &line[i]);
            col += (end_col - i);
            i = end_col;
            attroff(COLOR_PAIR(is_selected ? COLOR_SELECTION : cmt_color));

            
            if (strcmp(font_style, "italic") == 0) {
                attroff(A_DIM);
            }
            break;
        }

        
        if (state->file_type == FILE_TYPE_HTML && ch == '<' && i + 3 < len && strncmp(&line[i], "<!--", 4) == 0) {
            int j = i + 4;
            int endpos = -1;
            while (j < end_col - 2) {
                if (line[j] == '-' && line[j + 1] == '-' && line[j + 2] == '>') { endpos = j + 3; break; }
                j++;
            }
            int cmt_color = get_hierarchical_color(state, "comment");
            const char* font_style = get_dynamic_font_style(state, "comment");
            int seg_len = (endpos >= 0 ? endpos : end_col) - i;

            
            if (strcmp(font_style, "italic") == 0) {
                attron(A_DIM);  
            }

            attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : cmt_color));
            mvprintw(screen_row, col, "%.*s", seg_len, &line[i]);
            col += seg_len;
            i += seg_len;
            attroff(COLOR_PAIR(is_selected ? COLOR_SELECTION : cmt_color));

            
            if (strcmp(font_style, "italic") == 0) {
                attroff(A_DIM);
            }
            continue;
        }

        
        if (isalpha((unsigned char)ch) || ch == '_' || ch == '$') {
            int start = i;
            while (i < end_col && (isalnum((unsigned char)line[i]) || line[i] == '_' || line[i] == '$')) i++;
            int word_len = i - start;
            char word[64] = {0};
            if (word_len < (int)sizeof(word)) {
                strncpy(word, &line[start], word_len);
            }

            int color = get_vscode_scope_color(state, word, start, line, line_num);
            if (color == COLOR_DEFAULT) {
                color = get_hierarchical_color(state, "variable");
            }

            
            const char* font_style = "";
            if (color != COLOR_DEFAULT) {
                TokenInfo info;
                analyze_token_context(state, line_num, start, i, &info);
                if (strlen(info.scope) > 0) {
                    font_style = get_dynamic_font_style(state, info.scope);
                }
            }

            
            if (strcmp(font_style, "underline") == 0) {
                attron(A_UNDERLINE);
            }

            attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : color));
            mvprintw(screen_row, col, "%.*s", word_len, &line[start]);
            col += word_len;
            attroff(COLOR_PAIR(is_selected ? COLOR_SELECTION : color));

            
            if (strcmp(font_style, "underline") == 0) {
                attroff(A_UNDERLINE);
            }
            continue;
        }

        
        attron(COLOR_PAIR(is_selected ? COLOR_SELECTION : COLOR_DEFAULT));
        mvaddch(screen_row, col++, ch);
        attroff(COLOR_PAIR(is_selected ? COLOR_SELECTION : COLOR_DEFAULT));
        i++;
    }
    
    
}