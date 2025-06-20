# Multiplayer Pong

A game made to learn something about [ECS](https://en.wikipedia.org/wiki/Entity_component_system), only Linux is supported. Uses TCP for networking, seems to work just fine for 2 players. When I used same networking for syncing movements of 3 players latency increased drastically.

![image](https://github.com/user-attachments/assets/6d69f8f2-e079-4a09-8b78-c303faa134ec)

## Movement

- Move Up - J
- Move Down - K

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
