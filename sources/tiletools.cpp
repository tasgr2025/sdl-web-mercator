#include "tiletools.h"

using namespace glm;

static const double R = 6372795.0;
static double tile_width =  256.0;
static double tile_height = 256.0;
static double min_zoom = 0.0;   // TODO: убрать этот параметр
static double max_zoom = 20.0;  // TODO: убрать этот параметр


void set_zoom(dvec3& xyz, const dvec2& canvas_size, double zoom, const ivec2& pivot) {
    dvec2 p1 = screen_to_world(xyz, canvas_size, pivot);
    xyz.z = clamp(zoom, max_zoom, min_zoom);
    dvec2 p2 = screen_to_world(xyz, canvas_size, pivot);
    xyz.x -= p2.x - p1.x;
    xyz.y -= p2.y - p1.y;
}


void multiply_zoom(dvec3& xyz, const dvec2& canvas_size, double multiplier, const ivec2& pivot) {
    dvec2 p1 = screen_to_world(xyz, canvas_size, pivot);
    xyz.z = clamp(xyz.z * multiplier, double(min_zoom), double(max_zoom));
    dvec2 p2 = screen_to_world(xyz, canvas_size, pivot);
    xyz.x -= p2.x - p1.x;
    xyz.y -= p2.y - p1.y;
}


void step_zoom(dvec3& xyz, const dvec2& canvas_size, double step, const ivec2& pivot) {
    dvec2 p1 = screen_to_world(xyz, canvas_size, pivot);
    xyz.z = clamp(xyz.z + step, double(min_zoom), double(max_zoom));
    dvec2 p2 = screen_to_world(xyz, canvas_size, pivot);
    xyz.x -= p2.x - p1.x;
    xyz.y -= p2.y - p1.y;
}


dvec2 screen_to_lonlat(const dvec3& xyz, const dvec2& canvas_size, const dvec2& screen_coords) {
    dvec2 w = screen_to_world(xyz, canvas_size, screen_coords);
    return world_to_lonlat(w.x, w.y);
}


dvec2 screen_to_world(const dvec3& xyz, const dvec2& canvas_size, const ivec2 &screen_coords) {
    double n = pow(2.0, xyz.z);
    double span_w = n * tile_width;
    double span_h = n * tile_height;
    double px = double(screen_coords.x) - canvas_size.x / 2.0 + span_w / 2.0;
    double py = double(screen_coords.y) - canvas_size.y / 2.0 + span_h / 2.0;
    double xr = px / span_w;
    double yr = py / span_h;
    double x =  ( xr * 2.0  - 1.0) + xyz.x;
    double y = ((-yr * 2.0) + 1.0) + xyz.y;
    return {x, y};
}


dvec2 screen_to_tile(double z, const dvec3& xyz, const dvec2& canvas_size, const dvec2& screen_coords) {
    dvec2 world = screen_to_world(xyz, canvas_size, screen_coords);
    return world_to_tile(world.x, world.y, z);
}


dvec2 tile_to_screen(const dvec3& xyz, const dvec2& canvas_size, const dvec3& tile_coords) {
    dvec2 w = tile_to_world(tile_coords);
    return world_to_screen(xyz, canvas_size, w);
}


dvec2 world_to_screen(const dvec3& xyz, const dvec2& canvas_size, const dvec2& world_coords) {   
    double n = pow(2.0, xyz.z);
    double w = n * tile_width;
    double h = n * tile_height;
    double xr = ( (world_coords.x - xyz.x) + 1.0) / 2.0;
    double yr = (-(world_coords.y - xyz.y) + 1.0) / 2.0;
    double x = w * xr - w / 2.0 + canvas_size.x / 2.0;
    double y = h * yr - h / 2.0 + canvas_size.y / 2.0;
    return {ceil(x), ceil(y)};
}


dvec2 lonlat_to_screen(const dvec3& xyz, const dvec2& canvas_size, const dvec2& ll) {
     dvec2 wp = lonlat_to_world(ll);
     return world_to_screen(xyz, wp, canvas_size);
}


dvec2 get_zoom_bounds() {
    return {min_zoom, max_zoom};
}


dvec2 get_tile_size() {
    return {tile_width, tile_height};
}


void set_tile_size (const dvec2& ts) {
    tile_width = ts.x;
    tile_height = ts.y;
}


double deg_to_rad(double v) {
    return v * (M_PI / 180.0);
}


dvec2 lonlat_to_world(const dvec2& ll) {
    double x = ll.x / 180.0;
    double latsin = sin(deg_to_rad(ll.y) * copysign(1.0, ll.y));
    double y = (copysign(1.0, ll.y) * (log((1.0 + latsin) / (1.0 - latsin)) / 2.0)) / M_PI;
    return {x, y};
 }
 
 
 double rad_to_deg(double val) {
    return val * (180.0 / M_PI);
}
 
 
dvec2 world_to_lonlat(double wx, double wy) {
    double lon = wx * 180.0f;
    double lat = rad_to_deg(atan(sinh(wy * M_PI)));
    return {lon, lat};
}


dvec2 world_to_tile(double wx, double wy, double z) {
    double n = pow(2.0, floor(z));
    double tx = (( wx + 1.0) / 2.0) * n;
    double ty = ((-wy + 1.0) / 2.0) * n;
    return {floor(tx), floor(ty)};
}


dvec2 lonlat_to_tile(double lon, double lat, double z) {
    dvec2 w = lonlat_to_world(dvec2(lon, lat));
    return world_to_tile(w.x, w.y, z);
}


dvec2 tile_to_lonlat(const dvec3& tile_coords) {   
    dvec2 w = tile_to_world(tile_coords);
    return world_to_lonlat(w.x, w.y);
}


dvec2 tile_to_world(const dvec3& t) {
    double n = pow(2.0, floor(t.z));
    double x =   (t.x / n) * 2.0 - 1.0;
    double y = -((t.y / n) * 2.0 - 1.0);
    return {x, y};
}


uint64_t tile_to_index(double tx, double ty, double tz) {
    double i = (pow(4.0, tz) - 1.0) / 3.0;
    double n =  pow(2.0, tz);
    return uint64_t(round(i + (ty * n + tx)));
}


double haversine_m(double lat1, double lon1, double lat2, double lon2) {
    double rlat1 = deg_to_rad(lat1);
    double rlon1 = deg_to_rad(lon1);
    double rlat2 = deg_to_rad(lat2);
    double rlon2 = deg_to_rad(lon2);
    double cl1 = cos(rlat1);
    double cl2 = cos(rlat2);
    double sl1 = sin(rlat1);
    double sl2 = sin(rlat2);
    double delta = rlon2 - rlon1;
    double cdelta = cos(delta);
    double sdelta = sin(delta);
    double y = sqrt(pow(cl2 * sdelta, 2.0) + pow(cl1 * sl2 - sl1 * cl2 * cdelta, 2.0));
    double x = sl1 * sl2 + cl1 * cl2 * cdelta;
    double ad = atan2(y, x);
    double dist = ad * R;
    return dist;
}
