#include "properties.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

Properties g_Properties = { .SCREEN_WIDTH = 900,
                            .SCREEN_HEIGHT = 600,
                            .FPS_LOCK = 60,
                            .PADDLE_SPEED = 450.f,
                            .PADDLE_WIDTH = 20.f,
                            .PADDLE_HEIGHT = 80.f,
                            .PADDLE_SCREEN_SIZE_MARGIN = 50.f,
                            .BALL_SIDE = 10.f,
                            .BALL_MIN_SPEED = 450.f,
                            .BALL_MAX_SPEED = 650.f,
                            .BALL_INITIAL_DIRECTION = { 0.7f, -0.7f },
                            .WALL_THICKNESS = 10.f,
                            .MIDDLE_LINE_WIDTH = 10.f,
                            .SERVER_IP = "127.0.0.1",
                            .SERVER_PORT = 8080 };

static char *trim_whitespace(char *str) {
    if (!str) {
        return NULL;
    }

    char *end;

    while (isspace((unsigned char)*str)) {
        str++;
    }

    if (*str == 0) {
        return str;
    }

    end = str + strlen(str) - 1;

    while (end > str && isspace((unsigned char)*end)) {
        end--;
    }

    end[1] = '\0';

    return str;
}

int properties_load_from_file(const char *filename) {
    FILE *file = fopen(filename, "r");

    if (!file) {
        fprintf(stderr, "error: Configuration file does not exist\n");
        return 0;
    }

    char line[256];
    int  line_num = 0;

    while (fgets(line, sizeof(line), file)) {
        line_num++;

        char *trimmed_line = trim_whitespace(line);

        if (strlen(trimmed_line) == 0 || trimmed_line[0] == '#') {
            continue;
        }

        char *equals_pos = strchr(trimmed_line, '=');

        if (!equals_pos) {
            fprintf(
                stderr,
                "Warning: Invalid format on line %d: %s\n",
                line_num,
                trimmed_line
            );
            continue;
        }

        *equals_pos = '\0';
        char *key = trim_whitespace(trimmed_line);
        char *value = trim_whitespace(equals_pos + 1);

        if (value[0] == '"' && value[strlen(value) - 1] == '"') {
            value[strlen(value) - 1] = '\0';
            value++;
        }

        if (strcmp(key, "SCREEN_WIDTH") == 0) {
            g_Properties.SCREEN_WIDTH = atoi(value);
        } else if (strcmp(key, "SCREEN_HEIGHT") == 0) {
            g_Properties.SCREEN_HEIGHT = atoi(value);
        } else if (strcmp(key, "FPS_LOCK") == 0) {
            g_Properties.FPS_LOCK = atoi(value);
        } else if (strcmp(key, "PADDLE_SPEED") == 0) {
            g_Properties.PADDLE_SPEED = atof(value);
        } else if (strcmp(key, "PADDLE_WIDTH") == 0) {
            g_Properties.PADDLE_WIDTH = atof(value);
        } else if (strcmp(key, "PADDLE_HEIGHT") == 0) {
            g_Properties.PADDLE_HEIGHT = atof(value);
        } else if (strcmp(key, "PADDLE_SCREEN_SIZE_MARGIN") == 0) {
            g_Properties.PADDLE_SCREEN_SIZE_MARGIN = atof(value);
        } else if (strcmp(key, "BALL_SIDE") == 0) {
            g_Properties.BALL_SIDE = atof(value);
        } else if (strcmp(key, "BALL_MIN_SPEED") == 0) {
            g_Properties.BALL_MIN_SPEED = atof(value);
        } else if (strcmp(key, "BALL_MAX_SPEED") == 0) {
            g_Properties.BALL_MAX_SPEED = atof(value);
        } else if (strcmp(key, "BALL_INITIAL_DIRECTION_X") == 0) {
            g_Properties.BALL_INITIAL_DIRECTION.x = atof(value);
        } else if (strcmp(key, "BALL_INITIAL_DIRECTION_Y") == 0) {
            g_Properties.BALL_INITIAL_DIRECTION.y = atof(value);
        } else if (strcmp(key, "WALL_THICKNESS") == 0) {
            g_Properties.WALL_THICKNESS = atof(value);
        } else if (strcmp(key, "MIDDLE_LINE_WIDTH") == 0) {
            g_Properties.MIDDLE_LINE_WIDTH = atof(value);
        } else if (strcmp(key, "SERVER_IP") == 0) {
            strncpy(
                g_Properties.SERVER_IP,
                value,
                sizeof(g_Properties.SERVER_IP) - 1
            );
            g_Properties.SERVER_IP[sizeof(g_Properties.SERVER_IP) - 1] = '\0';
        } else if (strcmp(key, "SERVER_PORT") == 0) {
            g_Properties.SERVER_PORT = (unsigned short)atoi(value);
        } else {
            fprintf(
                stderr,
                "error: Unknown property '%s' on line %d\n",
                key,
                line_num
            );
        }
    }

    fclose(file);
    return 1;
}
