#include <stdio.h>
#include <stdlib.h>

#include "aabb.h"
#include "collission_sound.h"
#include "components.h"
#include "entities.h"
#include "flecs.h"
#include "game_state.h"
#include "input.h"
#include "movement.h"
#include "network.h"
#include "properties.h"
#include "raylib.h"
#include "server.h"
#include "systems.h"

int main(void) {
  if (properties_load_from_file("configuration.game")) {
    printf("Successfully loaded properties from configuration file\n");
    printf("Server: %s:%d\n", g_Properties.SERVER_IP, g_Properties.SERVER_PORT);
  } else {
    printf("Using default settings\n");
  }

  UDPClient* client = (UDPClient*)malloc(sizeof(UDPClient));
  if (!client) {
    fprintf(stderr, "Error: Failed to allocate memory for client\n");
    return 1;
  }

  if (sr_udp_client_connect(client, g_Properties.SERVER_IP,
                             g_Properties.SERVER_PORT) != 0) {
    fprintf(stderr, "Error: Failed to connect to the server\n");
    fprintf(stderr, "Make sure the server is running on %s:%d\n",
            g_Properties.SERVER_IP, g_Properties.SERVER_PORT);
    free(client);
    return 1;
  }

  SetTraceLogLevel(LOG_ERROR);
  InitWindow(g_Properties.SCREEN_WIDTH, g_Properties.SCREEN_HEIGHT, "game");
  SetTargetFPS(g_Properties.FPS_LOCK);
  InitAudioDevice();

  Wave collision_wave = {
      .data = COLLISSION_SOUND_WAV_DATA,
      .frameCount = COLLISSION_SOUND_WAV_FRAME_COUNT,
      .sampleRate = COLLISSION_SOUND_WAV_SAMPLE_RATE,
      .sampleSize = COLLISSION_SOUND_WAV_SAMPLE_SIZE,
      .channels = COLLISSION_SOUND_WAV_CHANNELS,
  };
  Sound collision_sound = LoadSoundFromWave(collision_wave);

  ecs_world_t* world = ecs_init();
  RegisterAllComponents(world);
  RegisterAllSystems(world);

  Input player_input;
  InitInput(&player_input);
  player_input.bindings[ACTION_MOVE_UP].key = KEY_K;
  player_input.bindings[ACTION_MOVE_DOWN].key = KEY_J;

  GameState game_state = {
      .world = world,
      .client = client,
      .collision_sound = collision_sound,
  };

  game_state.player_entity = CreatePlayerEntity(world, &player_input);
  game_state.enemy_entity = CreateEnemyEntity(world);
  game_state.ball_entity = CreateBallEntity(world, collision_sound);
  game_state.upper_wall_entity = CreateWallEntity(world, 0, 0, kUpperWall);
  game_state.lower_wall_entity = CreateWallEntity(
      world, 0, g_Properties.SCREEN_HEIGHT, kLowerWall);

  while (!WindowShouldClose()) {
    ServerMessage msg;
    while (sr_receive_udp_server_message(client, &msg) == 0) {
      HandleServerMessage(&game_state, &msg);
    }

    UpdateInput(&player_input);

    float delta_time = GetFrameTime();
    ecs_run(world, g_ClampMovementSystem, delta_time, NULL);
    ecs_run(world, g_ProcessInputSystem, delta_time, (void*)&g_Properties);
    ecs_run(world, g_MovePlayerSystem, delta_time, NULL);

    if (client->is_main) {
      ecs_run(world, g_ScoreCountSystem, delta_time, (void*)&g_Properties);
      ecs_run(world, g_BallPaddleCollisions, delta_time, NULL);
      ecs_run(world, g_BallPaddleSoundOnCollission, delta_time, NULL);
      ecs_run(world, g_BallWallSoundOnCollission, delta_time, NULL);
      ecs_run(world, g_BallWallCollisions, delta_time, NULL);
      ecs_run(world, g_MoveBall, delta_time, NULL);

      const Position* ball_pos = ecs_get(world, game_state.ball_entity, Position);
      const ClientMessage ball_msg = {
          .type = CLIENT_MSG_BALL_POSITION,
          .data.position = {ball_pos->x, ball_pos->y},
      };
      sr_send_udp_message_to_server(client, &ball_msg);
    } else {
      const Position* current_player_pos =
          ecs_get(world, game_state.player_entity, Position);
      if (current_player_pos->x > (float)g_Properties.SCREEN_WIDTH / 2) {
        ecs_set(world, game_state.player_entity, Position,
                {.x = g_Properties.PADDLE_SCREEN_SIZE_MARGIN,
                 .y = current_player_pos->y});
      }
    }

    ecs_run(world, g_SendPlayerPositionSystem, delta_time, client);

    BeginDrawing();
    ClearBackground(BLACK);
    ecs_run(world, g_RenderRectangle, delta_time, NULL);

    DrawRectangle(g_Properties.SCREEN_WIDTH / 2, 0,
                  g_Properties.MIDDLE_LINE_WIDTH, g_Properties.SCREEN_HEIGHT,
                  WHITE);

#ifdef DEBUGGING
    const Position* ball_pos = ecs_get(world, game_state.ball_entity, Position);
    const Score* left_score =
        ecs_get(world, game_state.player_entity, Score);
    const Score* right_score =
        ecs_get(world, game_state.enemy_entity, Score);

    DrawText(
        TextFormat("Ball Position: x: %.1f, y: %.1f", ball_pos->x, ball_pos->y),
        10.0f, 15.0f, 20.0f, WHITE);

    if (client->is_main) {
      DrawText(TextFormat("Left score: %d", left_score->value), 10.0f, 35.0f,
               20.0f, WHITE);
      DrawText(TextFormat("Right score: %d", right_score->value), 10.0f, 55.0f,
               20.0f, WHITE);
    }

    DrawFPS(10.0f, g_Properties.SCREEN_HEIGHT - 30.0f);
#endif

    EndDrawing();
  }

  sr_udp_client_close(client);
  free(client);
  UnloadSound(collision_sound);
  CloseAudioDevice();
  CloseWindow();
  ecs_fini(world);

  return 0;
}
