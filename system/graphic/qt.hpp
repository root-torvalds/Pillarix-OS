#pragma once

#ifdef __cplusplus
#include <stdint.h>
extern "C" {
#else
#include <stdint.h>
#endif

EFIAPI void draw_pixel(UINT32 x, UINT32 y, UINT8 r, UINT8 g, UINT8 b, UINT8 alpha);

typedef struct {
    int x;
    int y;
} QPoint;

void drawPoint(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);                              // Отрисовка одной точки по координатам (x, y)
void drawPoints(const QPoint *points, int pointCount, uint8_t r, uint8_t g, uint8_t b, uint8_t a);    // Отрисовка массива отдельных точек
void drawLine(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a);            // Отрисовка прямой линии от (x1, y1) до (x2, y2)
void drawPolyline(const QPoint *points, int pointCount, uint8_t r, uint8_t g, uint8_t b, uint8_t a);  // Отрисовка последовательности соединенных линий

void drawRect(int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b, uint8_t a);        // Отрисовка контура прямоугольника
void fillRect(int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b, uint8_t a);        // Сплошная заливка внутренней области прямоугольника
void eraseRect(int x, int y, int width, int height);                                                  // Полная очистка прямоугольной области (заливка прозрачным черным)

void drawEllipse(int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b, uint8_t a);    // Отрисовка контура эллипса или круга внутри заданной области
void drawArc(int x, int y, int width, int height, int startAngle, int spanAngle, uint8_t r, uint8_t g, uint8_t b, uint8_t a); // Отрисовка незамкнутой дуги эллипса под заданным углом
void drawPie(int x, int y, int width, int height, int startAngle, int spanAngle, uint8_t r, uint8_t g, uint8_t b, uint8_t a); // Отрисовка кругового сектора (дуга с линиями к центру)
void drawChord(int x, int y, int width, int height, int startAngle, int spanAngle, uint8_t r, uint8_t g, uint8_t b, uint8_t a); // Отрисовка хорды (дуга, края которой соединены прямой линией)

void drawPolygon(const QPoint *points, int pointCount, uint8_t r, uint8_t g, uint8_t b, uint8_t a);   // Отрисовка замкнутого многоугольника с заполнением (OddEvenFill)
void drawConvexPolygon(const QPoint *points, int pointCount, uint8_t r, uint8_t g, uint8_t b, uint8_t a); // Отрисовка замкнутого выпуклого многоугольника с заполнением
void drawRoundedRect(int x, int y, int w, int h, int xRadius, int yRadius, uint8_t r, uint8_t g, uint8_t b, uint8_t a); // Отрисовка и заливка прямоугольника со скругленными углами

#ifdef __cplusplus
}
#endif
