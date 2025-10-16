# Multiplayer Pong

<img width="1806" height="629" alt="pong" src="https://github.com/user-attachments/assets/6863ce08-90c0-42b5-8031-f7f8561f6a13" />

A game made to learn [ECS](https://en.wikipedia.org/wiki/Entity_component_system), only Linux is supported. Uses TCP for networking, seems to work just fine for 2 players. When I used the same networking for syncing movements of 3 players - latency increased drastically.

## Building & Running

> [!NOTE] 
> In order to use `FETCH_LIBS=OFF` you must have libraries installed of your machine.

```sh
cmake -DFETCH_LIBS=ON -B build/
cmake --build build/
```

Now you may run the binaries: **pong_server** and connect with 2 **pong_client**.

## Configuration

The game supports runtime configuration through a properties file. Create a file named `configuration.game` in the same directory as the executable to override default settings. The file uses a simple key-value format:

```
SCREEN_WIDTH = 1024
SCREEN_HEIGHT = 768
FPS_LOCK = 120

PADDLE_SPEED = 500.0
PADDLE_WIDTH = 25.0
PADDLE_HEIGHT = 100.0
PADDLE_SCREEN_SIZE_MARGIN = 60.0

BALL_SIDE = 15.0
BALL_MIN_SPEED = 500.0
BALL_MAX_SPEED = 700.0
BALL_INITIAL_DIRECTION_X = 0.8
BALL_INITIAL_DIRECTION_Y = -0.6

WALL_THICKNESS = 15.0
MIDDLE_LINE_WIDTH = 8.0

SERVER_IP = "192.168.1.100"
SERVER_PORT = 9090
```

### Available Settings

| Category | Setting | Type | Default | Description |
|----------|---------|------|---------|-------------|
| Display | `SCREEN_WIDTH` | integer | 900 | Game window width |
| Display | `SCREEN_HEIGHT` | integer | 600 | Game window height |
| Display | `FPS_LOCK` | integer | 60 | Maximum frames per second |
| Paddle | `PADDLE_SPEED` | float | 450.0 | Paddle movement speed |
| Paddle | `PADDLE_WIDTH` | float | 20.0 | Paddle width |
| Paddle | `PADDLE_HEIGHT` | float | 80.0 | Paddle height |
| Paddle | `PADDLE_SCREEN_SIZE_MARGIN` | float | 50.0 | Distance from screen edge |
| Ball | `BALL_SIDE` | float | 10.0 | Ball size |
| Ball | `BALL_MIN_SPEED` | float | 450.0 | Minimum ball speed |
| Ball | `BALL_MAX_SPEED` | float | 650.0 | Maximum ball speed |
| Ball | `BALL_INITIAL_DIRECTION_X` | float | 0.7 | Initial horizontal direction |
| Ball | `BALL_INITIAL_DIRECTION_Y` | float | -0.7 | Initial vertical direction |
| Game | `WALL_THICKNESS` | float | 10.0 | Thickness of walls |
| Game | `MIDDLE_LINE_WIDTH` | float | 10.0 | Width of center line |
| Network | `SERVER_IP` | string | "127.0.0.1" | Server IP address |
| Network | `SERVER_PORT` | integer | 8080 | Server port number |

## Controls

| Key | Action |
|-----|--------|
| `J` | Move up |
| `K` | Move down |

## Dependencies

- [Flecs](https://github.com/SanderMertens/flecs)
- [Raylib](https://github.com/raysan5/raylib)
