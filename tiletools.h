#ifndef TILE_TOOLS_H
#define TILE_TOOLS_H

#include <math.h>
#include <glm/glm.hpp>

using namespace glm;

int sign(float val);

// convert lon/lat to world coords
vec2 lonlat_to_world(vec2 &ll);
float deg_to_rad(float v);
uint32 xyz_to_idx(float x, float y, float z);

// xyz - mercator position,
// ll - lon/lat
// canvas_size - размер поверхности для рисования
vec2 lonlat_to_tile (float lon, float lat, float z);

vec2 tile_to_lonlat (float tx, float ty, float tz);
vec2 tile_to_world  (float tx, float ty, float tz);

vec2 world_to_screen  (const vec3 &xyz, const vec2 &wp, const vec2 &cs);
vec2 lonlat_to_screen (const vec3 &xyz, const vec2 &ll, const vec2 &cs);

vec2 screen_to_lonlat (const vec3 &xyz, const vec2 &canvas_size, const vec2 &screen_coords);
vec2 screen_to_world  (const vec3 &xyz, const vec2 &canvas_size, const vec2 &screen_coords);
vec2 screen_to_tile   (const vec3 &xyz, const vec2 &canvas_size, const vec2 &screen_coords);
vec2 world_to_screen  (const vec3 &xyz, const vec2 &canvas_size, const vec2 &world_coords);
vec2 tile_to_screen   (const vec3 &xyz, const vec2 &canvas_size, const vec3 &tile_coords);

#endif // TILE_TOOLS_H
