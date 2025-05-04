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
void set_zoom(dvec3& xyz, const dvec2& canvas_size, float zoom, const ivec2& pivot);


/// @brief Умножает масштаб вокруг заданной точки
/// @param xyz Текущая позиция
/// @param canvas_size Размер в пикселях поверхности для рисования
/// @param multiplier Множитель масштаба
/// @param pivot Заданная точка
void multiply_zoom(dvec3& xyz, const dvec2& canvas_size, float multiplier, const ivec2& pivot);


/// @brief Увеличивает масштаб вокруг заданной точки
/// @param xyz Текущая позиция
/// @param canvas_size Размер в пикселях поверхности для рисования
/// @param multiplier Шаг изменения масштаба
/// @param pivot Заданная точка
void step_zoom(dvec3& xyz, const dvec2& canvas_size, float step, const ivec2& pivot);


/// @brief Возвращает размер плитки
/// @return Размер плитки в пикселях
dvec2 get_tile_size();


/// @brief Преобразует долготу и широту в мировые координаты .
/// @param ll Долгота и широта.
/// @return Мировые координаты.
dvec2 lonlat_to_world(const dvec2 &ll);


/// @brief Преобразует градусы в радианы.
/// @param v Градусы.
/// @return Радианы.
double deg_to_rad(double v);


/// @brief Преобразует xyz-индекс плитки в индекс одномерного массива.
/// @param tx Индекс плитки по оси X.
/// @param ty Индекс плитки по оси Y.
/// @param tz Уровень детализации плитки.
/// @return Индекс одномерного массива.
int64_t tile_to_index(float tx, float ty, float tz);


/// @brief Преобразует долготу и широту в индекс плитки .
/// @param lon Долгота.
/// @param lat Широта.
/// @param z Уровень детализация (zoom level).
/// @return xy-индекс плитки.
dvec2 lonlat_to_tile (float lon, float lat, float z);


/// @brief Преобразует xyz-индекс плитки в долготу и широту.
/// @param tile_coords
/// @return Долгота и широта.
dvec2 tile_to_lonlat (const dvec3& tile_coords);


/// @brief Преобразует xyz-индекс плитки в мировые координаты.
/// @param tile_coords
/// @return Мировые координаты.
dvec2 tile_to_world (const dvec3& tile_coords);


/// @brief 
/// @param wx 
/// @param wy 
/// @return 
dvec2 world_to_lonlat(float wx, float wy);


/// @brief 
/// @param wx 
/// @param wy 
/// @param z 
/// @return 
dvec2 world_to_tile(float wx, float wy, float z);


/// @brief Преобразует мировые координат в координаты экрана.
/// @param xyz
/// @param canvas_size Размер в пикселях поверхности для рисования.
/// @param world_coords Мировые координаты.
/// @return Координаты экрана.
dvec2 world_to_screen  (const dvec3 &xyz, const dvec2 &canvas_size, const dvec2 &world_coords);


/// @brief Преобразует долготу и широту в координаты экрана.
/// @param xyz
/// @param canvas_size Размер в пикселях поверхности для рисования.
/// @param lon_lat Долгота и широта.
/// @return Координаты экрана.
dvec2 lonlat_to_screen (const dvec3 &xyz, const dvec2 &canvas_size, const dvec2 &ll);


/// @brief Преобразует координаты экрана в долготу и широту.
/// @param xyz 
/// @param canvas_size Размер в пикселях поверхности для рисования.
/// @param screen_coords Координаты экрана.
/// @return Долгота и широта.
dvec2 screen_to_lonlat (const dvec3 &xyz, const dvec2 &canvas_size, const dvec2 &screen_coords);


/// @brief Преобразует координаты экрана в мировые координаты
/// @param xyz 
/// @param canvas_size Размер в пикселях поверхности для рисования.
/// @param screen_coords Координаты экрана.
/// @return Мировые координаты.
dvec2 screen_to_world  (const dvec3 &xyz, const dvec2 &canvas_size, const ivec2 &screen_coords);


/// @brief 
/// @param xyz 
/// @param canvas_size 
/// @param screen_coords 
/// @return 
dvec2 screen_to_tile   (float z, const dvec3 &xyz, const dvec2 &canvas_size, const dvec2 &screen_coords);


/// @brief 
/// @param xyz 
/// @param canvas_size 
/// @param tile_coords 
/// @return 
dvec2 tile_to_screen   (const dvec3 &xyz, const dvec2 &canvas_size, const dvec3 &tile_coords);


/// @brief Вычисляет расстояние между двумя точками 1 и 2 на поверхности Земли
/// @param lon1 Долгота точки 1
/// @param lat1 Широта точки 1
/// @param lon2 Долгота точки 2
/// @param lat2 Широта точки 2
/// @return Расстояние в метрах
float haversine_m(float lon1, float lat1, float lon2, float lat2);

#endif // TILE_TOOLS_H
