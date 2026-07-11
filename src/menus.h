#ifndef MENUS_H_H
#define MENUS_H_H

#include <stdbool.h>
#include "network.h"

#define MENU_MAX_IP_LENGTH 15 

typedef enum GameMenu {
    kMainMenu = 0,
    kConnectToServer,
    kGame,
    kQuit
} GameMenu;

typedef struct MenuState {
    GameMenu current_menu;
    bool connection_failed;
    
    char ip_input[MENU_MAX_IP_LENGTH + 1];
    int ip_letter_count;
    bool ip_box_active;
} MenuState;

void InitMenuState(MenuState *s, const char* default_ip);

void UpdateMenus(MenuState *s, UDPClient *client, int server_port);
void DrawMenus(MenuState *s, int screen_width, int screen_height);

void UpdateMainMenu(MenuState *s);
void DrawMainMenu(MenuState *s, int screen_width, int screen_height);
void DrawConnectionMenu(MenuState *s, int screen_width, int screen_height);

#endif // !MENUS_H_H
