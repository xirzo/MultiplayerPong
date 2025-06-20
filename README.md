# Multiplayer Pong

A game made to learn something about [ECS](https://en.wikipedia.org/wiki/Entity_component_system), only Linux is supported.

## Building

```
git clone https://github.com/xirzo/MultiplayerPong
cd MultiplayerPong
mkdir build
cmake -B build -DFETCH_LIBS=1
cmake --build build
```

## Dependencies

- [Flecs](https://github.com/SanderMertens/flecs)
- [Raylib](https://github.com/raysan5/raylib)
