#include "menus.h"
#include "raylib.h"
#include <string.h>

void InitMenuState(MenuState *s, const char* default_ip) {
    s->current_menu = kMainMenu;
    s->connection_failed = false;
    
    strncpy(s->ip_input, default_ip, MENU_MAX_IP_LENGTH);
    s->ip_input[MENU_MAX_IP_LENGTH] = '\0';
    s->ip_letter_count = strlen(s->ip_input);
    s->ip_box_active = false;
}

void UpdateMenus(MenuState *s, UDPClient *client, int server_port) {
    if (s->current_menu == kMainMenu) {
        UpdateMainMenu(s);
    } 
    else if (s->current_menu == kConnectToServer) {
        if (sr_udp_client_connect(client, s->ip_input, server_port) == 0) {
            s->current_menu = kGame;
        } else {
            s->connection_failed = true;
            s->current_menu = kMainMenu;
        }
    }
}

void UpdateMainMenu(MenuState *s) {
    Rectangle ipBox = { 300, 150, 200, 40 };
    Rectangle playBtn = { 300, 220, 200, 50 };
    Rectangle quitBtn = { 300, 290, 200, 50 };

    Vector2 mousePos = GetMousePosition();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        s->ip_box_active = CheckCollisionPointRec(mousePos, ipBox);

        if (CheckCollisionPointRec(mousePos, playBtn)) {
            s->current_menu = kConnectToServer;
            s->connection_failed = false;
        }
        if (CheckCollisionPointRec(mousePos, quitBtn)) {
            s->current_menu = kQuit;
        }
    }

    if (s->ip_box_active) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 48 && key <= 57) || key == 46) {
                if (s->ip_letter_count < MENU_MAX_IP_LENGTH) {
                    s->ip_input[s->ip_letter_count] = (char)key;
                    s->ip_input[s->ip_letter_count + 1] = '\0';
                    s->ip_letter_count++;
                }
            }
            key = GetCharPressed();
        }

        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) {
            if (s->ip_letter_count > 0) {
                s->ip_letter_count--;
                s->ip_input[s->ip_letter_count] = '\0';
            }
        }
    }
}

void DrawMainMenu(MenuState *s, int screen_width, int screen_height) {
    DrawText("Server IP:", 300, 130, 15, LIGHTGRAY);
    DrawRectangle(300, 150, 200, 40, RAYWHITE);
    
    if (s->ip_box_active) {
        DrawRectangleLines(300, 150, 200, 40, RED);
    } else {
        DrawRectangleLines(300, 150, 200, 40, DARKGRAY);
    }
    
    DrawText(s->ip_input, 310, 160, 20, MAROON);

    DrawRectangle(300, 220, 200, 50, DARKGRAY);
    DrawText("Connect & Play", 320, 235, 20, WHITE);

    DrawRectangle(300, 290, 200, 50, DARKGRAY);
    DrawText("Quit", 370, 305, 20, WHITE);

    if (s->connection_failed) {
        DrawText("Failed to connect to server!", 280, 380, 20, RED);
    }
}

void DrawConnectionMenu(MenuState *s, int screen_width, int screen_height) {
    DrawText(TextFormat("Connecting to %s...", s->ip_input), 
             screen_width / 2 - 180, screen_height / 2, 25, LIGHTGRAY);
}

void DrawMenus(MenuState *s, int screen_width, int screen_height) {
    if (s->current_menu == kMainMenu) {
        DrawMainMenu(s, screen_width, screen_height);
    } 
    else if (s->current_menu == kConnectToServer) {
        DrawConnectionMenu(s, screen_width, screen_height);
    }
}
