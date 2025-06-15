#ifndef UTILS_H
#define UTILS_H

typedef struct vec3 {
  float x;
  float y;
} vec2;

void vec2_multiply(vec2 *v, float f);
float vec2_magnitude(vec2 *v);
void vec2_normalize(vec2 *v);

#endif // !UTILS_H
