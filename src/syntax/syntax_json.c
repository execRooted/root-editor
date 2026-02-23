#include "../core/editor.h"
#include <string.h>
#include <stdio.h>

int parse_json_token_colors(const char* json_content, EditorState* state) {
    if (!json_content || !state) {
        return 0;
    }

    state->json_rules_count = 0;
    const char *pos = json_content;
    char buffer[1024];

    while ((pos = strstr(pos, "\"scope\":"))) {
        pos += 8;
        while (*pos && (*pos == ' ' || *pos == '\t' || *pos == '\n' || *pos == '"')) pos++;

        int i = 0;
        while (*pos && *pos != '"' && i < (int)sizeof(buffer)-1) {
            buffer[i++] = *pos++;
        }
        buffer[i] = '\0';

        if (i > 0 && state->json_rules_count < MAX_JSON_RULES) {
            const char *color_pos = pos;
            while ((color_pos = strstr(color_pos, "\"foreground\":"))) {
                color_pos += 13;
                while (*color_pos && (*color_pos == ' ' || *color_pos == '\t' || *color_pos == '\n' || *color_pos == '"')) color_pos++;

                int j = 0;
                char color_buffer[16] = {0};
                while (*color_pos && *color_pos != '"' && j < (int)sizeof(color_buffer)-1) {
                    color_buffer[j++] = *color_pos++;
                }
                color_buffer[j] = '\0';

                if (j > 0) {
                    strncpy(state->json_scopes[state->json_rules_count], buffer, MAX_SCOPE_LENGTH-1);
                    state->json_scopes[state->json_rules_count][MAX_SCOPE_LENGTH-1] = '\0';
                    strncpy(state->json_colors[state->json_rules_count], color_buffer, MAX_COLOR_LENGTH-1);
                    state->json_colors[state->json_rules_count][MAX_COLOR_LENGTH-1] = '\0';
                    state->json_font_styles[state->json_rules_count][0] = '\0';
                    state->json_rules_count++;
                    break;
                }
            }
        }
    }

    state->json_loaded = (state->json_rules_count > 0);
    return state->json_loaded;
}