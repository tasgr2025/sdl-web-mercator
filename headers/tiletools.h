#ifndef TILE_TOOLS_H
#define TILE_TOOLS_H

#include <math.h>
#include <glm/glm.hpp>

using namespace glm;


/// @brief 
/// @param xyz 
/// @param canvas_size Размер в пикселях поверхности для рисования
/// @param zoom 
/// @param pivot 
void set_zoom(vec3& xyz, const vec2& canvas_size, float zoom, const ivec2& pivot);


/// @brief Умножает масштаб вокруг заданной точки
/// @param xyz Текущая позиция
/// @param canvas_size Размер в пикселях поверхности для рисования
/// @param multiplier Множитель масштаба
/// @param pivot Заданная точка
void multiply_zoom(vec3& xyz, const vec2& canvas_size, float multiplier, const ivec2& pivot);


/// @brief Увеличивает масштаб вокруг заданной точки
/// @param xyz Текущая позиция
/// @param canvas_size Размер в пикселях поверхности для рисования
/// @param multiplier Шаг изменения масштаба
/// @param pivot Заданная точка
void step_zoom(vec3& xyz, const vec2& canvas_size, float step, const ivec2& pivot);


/// @brief Возвращает размер плитки
/// @return Размер плитки в пикселях
vec2 get_tile_size();


/// @brief Преобразует долготу и широту в мировые координаты .
/// @param ll Долгота и широта.
/// @return Мировые координаты.
vec2 lonlat_to_world(const vec2 &ll);


/// @brief Преобразует градусы в радианы.
/// @param v Градусы.
/// @return Радианы.
float deg_to_rad(float v);


/// @brief Преобразует xyz-индекс плитки в индекс одномерного массива.
/// @param tx Индекс плитки по оси X.
/// @param ty Индекс плитки по оси Y.
/// @param tz Уровень детализации плитки.
/// @return Индекс одномерного массива.
uint32_t tile_to_index(float tx, float ty, float tz);


/// @brief Преобразует долготу и широту в индекс плитки .
/// @param lon Долгота.
/// @param lat Широта.
/// @param z Уровень детализация (zoom level).
/// @return xy-индекс плитки.
vec2 lonlat_to_tile (float lon, float lat, float z);


/// @brief Преобразует xyz-индекс плитки в долготу и широту.
/// @param tx Индекс плитки по оси X.
/// @param ty Индекс плитки по оси Y.
/// @param tz Уровень детализации плитки.
/// @return Долгота и широта.
vec2 tile_to_lonlat (float tx, float ty, float tz);


/// @brief Преобразует xyz-индекс плитки в мировые координаты.
/// @param tx Индекс плитки по оси X.
/// @param ty Индекс плитки по оси Y.
/// @param tz Уровень детализации плитки.
/// @return Мировые координаты.
vec2 tile_to_world (float tx, float ty, float tz);


/// @brief 
/// @param wx 
/// @param wy 
/// @return 
vec2 world_to_lonlat(float wx, float wy);


/// @brief 
/// @param wx 
/// @param wy 
/// @param z 
/// @return 
vec2 world_to_tile(float wx, float wy, float z);


/// @brief Преобразует мировые координат в координаты экрана.
/// @param xyz
/// @param canvas_size Размер в пикселях поверхности для рисования.
/// @param world_coords Мировые координаты.
/// @return Координаты экрана.
vec2 world_to_screen  (const vec3 &xyz, const vec2 &canvas_size, const vec2 &world_coords);


/// @brief Преобразует долготу и широту в координаты экрана.
/// @param xyz
/// @param canvas_size Размер в пикселях поверхности для рисования.
/// @param lon_lat Долгота и широта.
/// @return Координаты экрана.
vec2 lonlat_to_screen (const vec3 &xyz, const vec2 &canvas_size, const vec2 &ll);


/// @brief Преобразует координаты экрана в долготу и широту.
/// @param xyz 
/// @param canvas_size Размер в пикселях поверхности для рисования.
/// @param screen_coords Координаты экрана.
/// @return Долгота и широта.
vec2 screen_to_lonlat (const vec3 &xyz, const vec2 &canvas_size, const vec2 &screen_coords);


/// @brief Преобразует координаты экрана в мировые координаты
/// @param xyz 
/// @param canvas_size Размер в пикселях поверхности для рисования.
/// @param screen_coords Координаты экрана.
/// @return Мировые координаты.
vec2 screen_to_world  (const vec3 &xyz, const vec2 &canvas_size, const ivec2 &screen_coords);


/// @brief 
/// @param xyz 
/// @param canvas_size 
/// @param screen_coords 
/// @return 
vec2 screen_to_tile   (const vec3 &xyz, const vec2 &canvas_size, const vec2 &screen_coords);
vec2 world_to_screen  (const vec3 &xyz, const vec2 &canvas_size, const vec2 &world_coords);
vec2 tile_to_screen   (const vec3 &xyz, const vec2 &canvas_size, const vec3 &tile_coords);


/// @brief Вычисляет расстояние между двумя точками 1 и 2 на поверхности Земли
/// @param lon1 Долгота точки 1
/// @param lat1 Широта точки 1
/// @param lon2 Долгота точки 2
/// @param lat2 Широта точки 2
/// @return Расстояние в метрах
float haversine_m(float lon1, float lat1, float lon2, float lat2);

#endif // TILE_TOOLS_H
