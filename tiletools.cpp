#include "tiletools.h"


const float tile_width =  256.0f;
const float tile_height = 256.0f;

using namespace glm;


int sign(float val)
{
    return (0.0f < val) - (val < 0.0f);
}


float deg_to_rad(float v)
{
    return v * (M_PI / 180.0f);
}


vec2 lonlat_to_world(const vec2 &ll)
{
   float x = ll.x / 180.0f;
   float latsin = sinf(deg_to_rad(ll.y) * sign(ll.y));
   float y = (sign(ll.y) * (logf((1.0f + latsin) / (1.0f - latsin)) / 2.0f)) / M_PI;
   return vec2(x, y);
}


// convert lon/lat to screen coords
vec2 lonlat_to_screen(const vec3 &xyz, const vec2 &ll, const vec2 &cs)
{
    vec2 wp = lonlat_to_world(ll);
    return world_to_screen(xyz, wp, cs);
}


float rad_to_deg(float val)
{
    return val * (180.0f / M_PI);
}


// convert world coords to lon/lat
vec2 world_to_lonlat(float wx, float wy)
{
    float lon = wx * 180.0f;
    float lat = rad_to_deg(atan(sinh(wy * M_PI)));
    return vec2(lon, lat);
}


// convert world coords to tile coords
vec2 world_to_tile(float wx, float wy, float z)
{
    float n = pow(2.0, floorf(z));
    float tx = ((wx + 1.0f) / 2.0f) * n;
    float ty = ((-wy + 1.0f) / 2.0f) * n;
    tx = floorf(tx);
    ty = floorf(ty);
    return vec2(tx, ty);
}


// convert lon/lat to tile coords
vec2 lonlat_to_tile(float lon, float lat, float z)
{
    vec2 w = lonlat_to_world(vec2(lon, lat));
    return world_to_tile(w.x, w.y, z);
}


// convert screen coords to lon/lat
vec2 screen_to_lonlat(const vec3 &xyz, const vec2 &canvas_size, const vec2 &screen_coords)
{
    vec2 w = screen_to_world(xyz, canvas_size, screen_coords);
    return world_to_lonlat(w.x, w.y);
}


// convert tile coords to lon/lat
vec2 tile_to_lonlat(float tx, float ty, float tz)
{   
    vec2 w = tile_to_world(tx, ty, tz);
    return world_to_lonlat(w.x, w.y);
}


// convert screen coords to world coords
vec2 screen_to_world(const vec3 &xyz, const vec2 &canvas_size, const vec2 &screen_coords)
{   
    float n = powf(2.0f, xyz.z);
    float span_w = n * tile_width;
    float span_h = n * tile_height;
    float px = screen_coords.x - canvas_size.x / 2.0f + span_w / 2.0f;
    float py = screen_coords.y - canvas_size.y / 2.0f + span_h / 2.0f;
    float xr = px / span_w;
    float yr = py / span_h;
    float x = (xr * 2.0f - 1.0f) + xyz.x;
    float y = ((-yr * 2.0f) + 1.0f) + xyz.y;
    return vec2(x, y);
}


// convert screen coords to tile coords
vec2 screen_to_tile(const vec3 &xyz, const vec2 &canvas_size, const vec2 &screen_coords)
{
    vec2 world = screen_to_world(xyz, canvas_size, screen_coords);
    return world_to_tile(world.x, world.y, xyz.z);
}


// convert tile coords to screen coords
vec2 tile_to_screen(const vec3 &xyz, const vec2 &canvas_size, const vec3 &tile_coords)
{
    vec2 w = tile_to_world(tile_coords.x, tile_coords.y, tile_coords.z);
    return world_to_screen(xyz, canvas_size, w);
}


// convert tile coords to world coords
vec2 tile_to_world(float tx, float ty, float tz)
{
    float n = powf(2.0f, floorf(tz));
    float x = (tx / n) * 2.0f - 1.0f;
    float y = -((ty / n) * 2.0f - 1.0f);
    return vec2(x, y);
}


// convert world coords to screen coords
vec2 world_to_screen(const vec3 &xyz, const vec2 &canvas_size, const vec2 &world_coords)
{   
    float n = powf(2.0f, xyz.z);
    float w = n * tile_width;
    float h = n * tile_height;
    float xr = ((world_coords.x - xyz.x) + 1.0f) / 2.0f;
    float yr = (-(world_coords.y - xyz.y) + 1.0f) / 2.0f;
    float x = w * xr - w / 2.0f + canvas_size.x / 2.0f;
    float y = h * yr - h / 2.0f + canvas_size.y / 2.0f;
    return vec2(x, y);
}


// get the tile index from xyz tile coords
uint32 xyz_to_idx(float x, float y, float z)
{
    float i = (powf(4.0f, z) - 1.0f) / 3.0f;
    float n = powf(2.0f, z);
    return int(i + (y * n + x));
}
