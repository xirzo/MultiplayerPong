#include <string.h>
#include "menus.h"
#include "raylib.h"

void InitMenuState(MenuState* s, const char* default_ip) {
  s->current_menu = kMainMenu;
  s->connection_failed = false;

  strncpy(s->ip_input, default_ip, MENU_MAX_IP_LENGTH);
  s->ip_input[MENU_MAX_IP_LENGTH] = '\0';
  s->ip_letter_count = strlen(s->ip_input);
  s->ip_box_active = false;
}

void UpdateMenus(MenuState* s, UDPClient* client, int server_port) {
  if (s->current_menu == kMainMenu) {
    UpdateMainMenu(s);
  } else if (s->current_menu == kConnectToServer) {
    if (sr_udp_client_connect(client, s->ip_input, server_port) == 0) {
      s->current_menu = kGame;
    } else {
      s->connection_failed = true;
      s->current_menu = kMainMenu;
    }
  }
}

void UpdateMainMenu(MenuState* s) {
  int sw = GetScreenWidth();
  int sh = GetScreenHeight();

  int box_w = (int)(sw * 0.4f);
  int box_h = (int)(sh * 0.08f);
  if (box_w < 200)
    box_w = 200;
  if (box_h < 40)
    box_h = 40;

  int start_x = (sw - box_w) / 2;
  int start_y = (int)(sh * 0.4f);
  int spacing = box_h + 20;

  Rectangle ipBox = {start_x, start_y, box_w, box_h};
  Rectangle playBtn = {start_x, start_y + spacing, box_w, box_h};
  Rectangle quitBtn = {start_x, start_y + spacing * 2, box_w, box_h};

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

void DrawMainMenu(MenuState* s, int screen_width, int screen_height) {
  int box_w = (int)(screen_width * 0.4f);
  int box_h = (int)(screen_height * 0.08f);
  if (box_w < 200)
    box_w = 200;
  if (box_h < 40)
    box_h = 40;

  int start_x = (screen_width - box_w) / 2;
  int start_y = (int)(screen_height * 0.4f);
  int spacing = box_h + 20;

  int font_title = (int)(screen_height * 0.06f);
  int font_label = (int)(screen_height * 0.03f);
  int font_btn = (int)(screen_height * 0.04f);
  if (font_title < 30)
    font_title = 30;
  if (font_label < 15)
    font_label = 15;
  if (font_btn < 20)
    font_btn = 20;

  const char* title = "Pong";
  int title_w = MeasureText(title, font_title);
  DrawText(title, (screen_width - title_w) / 2, start_y - (int)(box_h * 1.5f),
           font_title, WHITE);

  DrawText("Server IP:", start_x, start_y - font_label - 5, font_label,
           LIGHTGRAY);
  DrawRectangle(start_x, start_y, box_w, box_h, RAYWHITE);

  if (s->ip_box_active) {
    DrawRectangleLines(start_x, start_y, box_w, box_h, RED);
  } else {
    DrawRectangleLines(start_x, start_y, box_w, box_h, DARKGRAY);
  }

  DrawText(s->ip_input, start_x + 10, start_y + (box_h - font_btn) / 2,
           font_btn, MAROON);

  DrawRectangle(start_x, start_y + spacing, box_w, box_h, DARKGRAY);
  const char* play_txt = "Connect & Play";
  int play_w = MeasureText(play_txt, font_btn);
  DrawText(play_txt, start_x + (box_w - play_w) / 2,
           start_y + spacing + (box_h - font_btn) / 2, font_btn, WHITE);

  DrawRectangle(start_x, start_y + spacing * 2, box_w, box_h, DARKGRAY);
  const char* quit_txt = "Quit";
  int quit_w = MeasureText(quit_txt, font_btn);
  DrawText(quit_txt, start_x + (box_w - quit_w) / 2,
           start_y + spacing * 2 + (box_h - font_btn) / 2, font_btn, WHITE);

  if (s->connection_failed) {
    const char* err_txt = "Failed to connect to server!";
    int err_w = MeasureText(err_txt, font_label);
    DrawText(err_txt, (screen_width - err_w) / 2, start_y + spacing * 3 + 10,
             font_label, RED);
  }
}

void DrawConnectionMenu(MenuState* s, int screen_width, int screen_height) {
  const char* text = TextFormat("Connecting to %s...", s->ip_input);

  int font_size = (int)(screen_height * 0.05f);
  if (font_size < 20)
    font_size = 20;

  int text_w = MeasureText(text, font_size);
  DrawText(text, (screen_width - text_w) / 2, (screen_height - font_size) / 2,
           font_size, LIGHTGRAY);
}

void DrawMenus(MenuState* s, int screen_width, int screen_height) {
  if (s->current_menu == kMainMenu) {
    DrawMainMenu(s, screen_width, screen_height);
  } else if (s->current_menu == kConnectToServer) {
    DrawConnectionMenu(s, screen_width, screen_height);
  }
}
