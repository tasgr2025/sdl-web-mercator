#include "tiletools.h"

using namespace glm;


static float tile_width =  256.0f;
static float tile_height = 256.0f;
static const float R = 6372795.0f;
static float min_zoom = 2.0f;
static float max_zoom = 19.0;


void set_zoom(vec3& xyz, const vec2& canvas_size, float zoom, const ivec2& pivot) {
    vec2 p1 = screen_to_world(xyz, canvas_size, pivot);
    xyz.z = clamp(zoom, max_zoom, min_zoom);
    vec2 p2 = screen_to_world(xyz, canvas_size, pivot);
    xyz.x -= p2.x - p1.x;
    xyz.y -= p2.y - p1.y;
}


void multiply_zoom(vec3& xyz, const vec2& canvas_size, float multiplier, const ivec2& pivot) {
    vec2 p1 = screen_to_world(xyz, canvas_size, pivot);
    xyz.z = clamp(xyz.z * multiplier, min_zoom, max_zoom);
    vec2 p2 = screen_to_world(xyz, canvas_size, pivot);
    xyz.x -= p2.x - p1.x;
    xyz.y -= p2.y - p1.y;
}


vec2 get_zoom_bounds() {
    return {min_zoom, max_zoom};
}


vec2 get_tile_size() {
    return {tile_width, tile_height};
}


void set_tile_size (const vec2& ts) {
    tile_width = ts.x;
    tile_height = ts.y;
}


float deg_to_rad(float v) {
    return v * (M_PI / 180.0f);
}


vec2 lonlat_to_world(const vec2 &ll) {
   float x = ll.x / 180.0f;
   float latsin = sinf(deg_to_rad(ll.y) * copysignf(1.0f, ll.y));
   float y = (copysignf(1.0f, ll.y) * (logf((1.0f + latsin) / (1.0f - latsin)) / 2.0f)) / M_PI;
   return {x, y};
}


vec2 lonlat_to_screen(const vec3 &xyz, const vec2 &canvas_size, const vec2 &ll) {
    vec2 wp = lonlat_to_world(ll);
    return world_to_screen(xyz, wp, canvas_size);
}


float rad_to_deg(float val) {
    return val * (180.0f / M_PI);
}


vec2 world_to_lonlat(float wx, float wy) {
    float lon = wx * 180.0f;
    float lat = rad_to_deg(atan(sinh(wy * M_PI)));
    return {lon, lat};
}


vec2 world_to_tile(float wx, float wy, float z) {
    float n = pow(2.0, floorf(z));
    float tx = floorf((( wx + 1.0f) / 2.0f) * n);
    float ty = floorf(((-wy + 1.0f) / 2.0f) * n);
    return {tx, ty};
}


vec2 lonlat_to_tile(float lon, float lat, float z) {
    vec2 w = lonlat_to_world(vec2(lon, lat));
    return world_to_tile(w.x, w.y, z);
}


// convert screen coords to lon/lat
vec2 screen_to_lonlat(const vec3 &xyz, const vec2 &canvas_size, const vec2 &screen_coords) {
    vec2 w = screen_to_world(xyz, canvas_size, screen_coords);
    return world_to_lonlat(w.x, w.y);
}


// convert tile coords to lon/lat
vec2 tile_to_lonlat(float tx, float ty, float tz) {   
    vec2 w = tile_to_world(tx, ty, tz);
    return world_to_lonlat(w.x, w.y);
}


// convert screen coords to world coords
vec2 screen_to_world(const vec3 &xyz, const vec2 &canvas_size, const ivec2 &screen_coords) {   
    float n = powf(2.0f, xyz.z);
    float span_w = n * tile_width;
    float span_h = n * tile_height;
    float px = static_cast<float>(screen_coords.x) - canvas_size.x / 2.0f + span_w / 2.0f;
    float py = static_cast<float>(screen_coords.y) - canvas_size.y / 2.0f + span_h / 2.0f;
    float xr = px / span_w;
    float yr = py / span_h;
    float x = (xr * 2.0f - 1.0f) + xyz.x;
    float y = ((-yr * 2.0f) + 1.0f) + xyz.y;
    return {x, y};
}


vec2 screen_to_tile(const vec3 &xyz, const vec2 &canvas_size, const vec2 &screen_coords) {
    vec2 world = screen_to_world(xyz, canvas_size, screen_coords);
    return world_to_tile(world.x, world.y, xyz.z);
}


// convert tile coords to screen coords
vec2 tile_to_screen(const vec3 &xyz, const vec2 &canvas_size, const vec3 &tile_coords) {
    vec2 w = tile_to_world(tile_coords.x, tile_coords.y, tile_coords.z);
    return world_to_screen(xyz, canvas_size, w);
}


// convert tile coords to world coords
vec2 tile_to_world(float tx, float ty, float tz) {
    float n = powf(2.0f, floorf(tz));
    float x = (tx / n) * 2.0f - 1.0f;
    float y = -((ty / n) * 2.0f - 1.0f);
    return {x, y};
}


vec2 world_to_screen(const vec3 &xyz, const vec2 &canvas_size, const vec2 &world_coords) {   
    float n = powf(2.0f, xyz.z);
    float w = n * tile_width;
    float h = n * tile_height;
    float xr = ( (world_coords.x - xyz.x) + 1.0f) / 2.0f;
    float yr = (-(world_coords.y - xyz.y) + 1.0f) / 2.0f;
    float x = w * xr - w / 2.0f + canvas_size.x / 2.0f;
    float y = h * yr - h / 2.0f + canvas_size.y / 2.0f;
    return {x, y};
}


uint32 tile_to_idx(float tx, float ty, float tz) {
    float i = (powf(4.0f, tz) - 1.0f) / 3.0f;
    float n = powf(2.0f, tz);
    return static_cast<uint32>(i + (ty * n + tx));
}


float haversine_m(float lat1, float lon1, float lat2, float lon2) {
    // Преобразование градусов в радианы
    float rlat1 = deg_to_rad(lat1);
    float rlon1 = deg_to_rad(lon1);
    float rlat2 = deg_to_rad(lat2);
    float rlon2 = deg_to_rad(lon2);
    float cl1 = cosf(rlat1);
    float cl2 = cosf(rlat2);
    float sl1 = sinf(rlat1);
    float sl2 = sinf(rlat2);
    float delta = rlon2 - rlon1;
    float cdelta = cosf(delta);
    float sdelta = sinf(delta);
    // Вычисление длины большого круга
    float y = sqrtf(powf(cl2 * sdelta, 2.0f) + powf(cl1 * sl2 - sl1 * cl2 * cdelta, 2.0f));
    float x = sl1 * sl2 + cl1 * cl2 * cdelta;
    float ad = atan2f(y, x);
    float dist = ad * R;
    return dist;
}
