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

## Controls

| Key | Action |
|-----|--------|
| `J` | Move up |
| `K` | Move down |

## Dependencies

- [Flecs](https://github.com/SanderMertens/flecs)
- [Raylib](https://github.com/raysan5/raylib)
