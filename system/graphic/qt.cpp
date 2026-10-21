#include "../kernel/kernel/kernel.h"

static inline int int_abs(int n) {
    return (n < 0) ? -n : n;
}

static inline void safe_draw_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    // Исключаем вылет за верхнюю/левую границу (отрицательные координаты)
    if (x >= 0 && y >= 0) {
        draw_pixel((UINT32)x, (UINT32)y, (UINT8)r, (UINT8)g, (UINT8)b, (UINT8)a);
    }
}

static inline float local_atan2(float dy, float dx) { (void)dy; (void)dx; return 0.0f; }
static inline float local_cos(float rad) { (void)rad; return 1.0f; }
static inline float local_sin(float rad) { (void)rad; return 0.0f; }

void drawPoint(int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    safe_draw_pixel(x, y, r, g, b, a);
}

void drawPoints(const QPoint *points, int pointCount, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    for (int i = 0; i < pointCount; ++i) {
        safe_draw_pixel(points[i].x, points[i].y, r, g, b, a);
    }
}

void drawLine(int x1, int y1, int x2, int y2, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    int dx = int_abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = -int_abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = dx + dy, e2;

    while (1) {
        safe_draw_pixel(x1, y1, r, g, b, a);
        if (x1 == x2 && y1 == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) { err += dy; x1 += sx; }
        if (e2 <= dx) { err += dx; y1 += sy; }
    }
}

void drawPolyline(const QPoint *points, int pointCount, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (pointCount < 2) return;
    for (int i = 0; i < pointCount - 1; ++i) {
        drawLine(points[i].x, points[i].y, points[i+1].x, points[i+1].y, r, g, b, a);
    }
}

void drawRect(int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (width <= 0 || height <= 0) return;
    int x2 = x + width - 1;
    int y2 = y + height - 1;
    drawLine(x, y, x2, y, r, g, b, a);
    drawLine(x, y2, x2, y2, r, g, b, a);
    drawLine(x, y, x, y2, r, g, b, a);
    drawLine(x2, y, x2, y2, r, g, b, a);
}

void fillRect(int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    for (int py = y; py < y + height; ++py) {
        for (int px = x; px < x + width; ++px) {
            safe_draw_pixel(px, py, r, g, b, a);
        }
    }
}

void eraseRect(int x, int y, int width, int height) {
    fillRect(x, y, width, height, 0, 0, 0, 0);
}

void drawEllipse(int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (width <= 0 || height <= 0) return;

    int rx = width / 2;
    int ry = height / 2;
    int xc = x + rx;
    int yc = y + ry;

    for (int py = y; py < y + height; ++py) {
        for (int px = x; px < x + width; ++px) {
            float dx = (float)(px - xc);
            float dy = (float)(py - yc);

            float norm_x = dx / (float)rx;
            float norm_y = dy / (float)ry;
            float distance = norm_x * norm_x + norm_y * norm_y;

            if (distance <= 1.0f) {
                if (distance > 0.88f) {
                    float alpha_factor = (1.0f - distance) / (1.0f - 0.88f);
                    uint8_t final_a = (uint8_t)((float)a * alpha_factor);
                    if (final_a > 0) safe_draw_pixel(px, py, r, g, b, final_a);
                } else {
                    safe_draw_pixel(px, py, r, g, b, a);
                }
            }
        }
    }
}

static inline int checkArcAngle(int cx, int cy, int px, int py, int startAngle16, int spanAngle16) {
    float dx = (float)(px - cx);
    float dy = (float)(-(py - cy));
    float angle = local_atan2(dy, dx) * 57.29577f;
    if (angle < 0.0f) angle += 360.0f;

    float start = startAngle16 / 16.0f;
    float span = spanAngle16 / 16.0f;
    float end = start + span;

    if (span < 0) { float t = start; start = end; end = t; }
    if (end > 360.0f && angle < start) angle += 360.0f;

    return (angle >= start && angle <= end);
}

void drawArc(int x, int y, int width, int height, int startAngle, int spanAngle, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (width <= 0 || height <= 0) return;

    int rx = width / 2;
    int ry = height / 2;
    int xc = x + rx;
    int yc = y + ry;

    for (int py = y; py < y + height; ++py) {
        for (int px = x; px < x + width; ++px) {
            if (!checkArcAngle(xc, yc, px, py, startAngle, spanAngle)) continue;

            float dx = (float)(px - xc);
            float dy = (float)(py - yc);
            float norm_x = dx / (float)rx;
            float norm_y = dy / (float)ry;
            float distance = norm_x * norm_x + norm_y * norm_y;

            if (distance >= 0.85f && distance <= 1.05f) {
                float diff = 1.0f - (float)int_abs((int)((distance - 0.95f) * 20.0f));
                if (diff < 0.0f) diff = 0.0f;
                if (diff > 1.0f) diff = 1.0f;

                uint8_t final_a = (uint8_t)((float)a * diff);
                if (final_a > 0) safe_draw_pixel(px, py, r, g, b, final_a);
            }
        }
    }
}

void drawPie(int x, int y, int width, int height, int startAngle, int spanAngle, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (width <= 0 || height <= 0) return;

    int rx = width / 2;
    int ry = height / 2;
    int xc = x + rx;
    int yc = y + ry;

    for (int py = y; py < y + height; ++py) {
        for (int px = x; px < x + width; ++px) {
            if (!checkArcAngle(xc, yc, px, py, startAngle, spanAngle)) continue;

            float dx = (float)(px - xc);
            float dy = (float)(py - yc);
            float norm_x = dx / (float)rx;
            float norm_y = dy / (float)ry;
            float distance = norm_x * norm_x + norm_y * norm_y;

            if (distance <= 1.0f) {
                if (distance > 0.88f) {
                    float alpha_factor = (1.0f - distance) / (1.0f - 0.88f);
                    uint8_t final_a = (uint8_t)((float)a * alpha_factor);
                    if (final_a > 0) safe_draw_pixel(px, py, r, g, b, final_a);
                } else {
                    safe_draw_pixel(px, py, r, g, b, a);
                }
            }
        }
    }

    float radStart = (startAngle / 16.0f) * 0.0174532f;
    float radEnd = ((startAngle + spanAngle) / 16.0f) * 0.0174532f;
    int x1 = xc + (int)((width / 2) * local_cos(radStart));
    int y1 = yc - (int)((height / 2) * local_sin(radStart));
    int x2 = xc + (int)((width / 2) * local_cos(radEnd));
    int y2 = yc - (int)((height / 2) * local_sin(radEnd));

    drawLine(xc, yc, x1, y1, r, g, b, a);
    drawLine(xc, yc, x2, y2, r, g, b, a);
}

void drawChord(int x, int y, int width, int height, int startAngle, int spanAngle, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    drawArc(x, y, width, height, startAngle, spanAngle, r, g, b, a);

    int xc = x + width / 2;
    int yc = y + height / 2;
    float radStart = (startAngle / 16.0f) * 0.0174532f;
    float radEnd = ((startAngle + spanAngle) / 16.0f) * 0.0174532f;
    int x1 = xc + (int)((width / 2) * local_cos(radStart));
    int y1 = yc - (int)((height / 2) * local_sin(radStart));
    int x2 = xc + (int)((width / 2) * local_cos(radEnd));
    int y2 = yc - (int)((height / 2) * local_sin(radEnd));

    drawLine(x1, y1, x2, y2, r, g, b, a);
}

void drawPolygon(const QPoint *points, int pointCount, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (pointCount < 3) return;
    int minY = points[0].y, maxY = points[0].y;
    for(int i = 1; i < pointCount; ++i) {
        if(points[i].y < minY) minY = points[i].y;
        if(points[i].y > maxY) maxY = points[i].y;
    }

    int nodeX[128];
    for (int y = minY; y <= maxY; ++y) {
        int nodes = 0, j = pointCount - 1;
        for (int i = 0; i < pointCount; ++i) {
            if ((points[i].y < y && points[j].y >= y) || (points[j].y < y && points[i].y >= y)) {
                if (nodes < 128) {
                    nodeX[nodes++] = points[i].x + (y - points[i].y) * (points[j].x - points[i].x) / (points[j].y - points[i].y);
                }
            }
            j = i;
        }
        for (int i = 0; i < nodes - 1; ++i) {
            for (int k = 0; k < nodes - i - 1; ++k) {
                if (nodeX[k] > nodeX[k+1]) { int t = nodeX[k]; nodeX[k] = nodeX[k+1]; nodeX[k+1] = t; }
            }
        }
        for (int k = 0; k < nodes; k += 2) {
            if (k + 1 >= nodes) break;
            for (int px = nodeX[k]; px < nodeX[k+1]; ++px) safe_draw_pixel(px, y, r, g, b, a);
        }
    }
    drawPolyline(points, pointCount, r, g, b, a);
    drawLine(points[pointCount-1].x, points[pointCount-1].y, points[0].x, points[0].y, r, g, b, a);
}

void drawConvexPolygon(const QPoint *points, int pointCount, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    drawPolygon(points, pointCount, r, g, b, a);
}

void drawRoundedRect(int x, int y, int w, int h, int rx, int ry, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (w <= 0 || h <= 0) return;
    if (rx > w / 2) rx = w / 2;
    if (ry > h / 2) ry = h / 2;
    if (rx <= 0 || ry <= 0) {
        fillRect(x, y, w, h, r, g, b, a);
        return;
    }

    int inner_left   = x + rx;
    int inner_right  = x + w - rx;
    int inner_top    = y + ry;
    int inner_bottom = y + h - ry;

    for (int py = y; py < y + h; ++py) {
        for (int px = x; px < x + w; ++px) {

            bool is_left   = (px < inner_left);
            bool is_right  = (px >= inner_right);
            bool is_top    = (py < inner_top);
            bool is_bottom = (py >= inner_bottom);

            if ((is_left || is_right) && (is_top || is_bottom)) {
                float cx = is_left ? (float)inner_left : (float)inner_right;
                float cy = is_top  ? (float)inner_top  : (float)inner_bottom;

                float dx = ((float)px + 0.5f) - cx;
                float dy = ((float)py + 0.5f) - cy;

                float norm_x = dx / (float)rx;
                float norm_y = dy / (float)ry;
                float distance = norm_x * norm_x + norm_y * norm_y;

                if (distance <= 1.0f) {
                    if (distance > 0.88f) {
                        float alpha_factor = (1.0f - distance) / (1.0f - 0.88f);
                        uint8_t final_a = (uint8_t)((float)a * alpha_factor);
                        if (final_a > 0) safe_draw_pixel(px, py, r, g, b, final_a);
                    } else {
                        safe_draw_pixel(px, py, r, g, b, a);
                    }
                }
            } else {
                safe_draw_pixel(px, py, r, g, b, a);
            }
        }
    }
}
