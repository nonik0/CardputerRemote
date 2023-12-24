#include <M5Cardputer.h>

#include "gear_bmp.h"

// RBG565 colors
const unsigned short COLOR_BLACK = 0x18E3;
const unsigned short COLOR_DARKGRAY = 0x0861;
const unsigned short COLOR_MEDGRAY = 0x2104;
const unsigned short COLOR_DARKRED = 0x5800;
const unsigned short COLOR_ORANGE = 0xC401;
const unsigned short COLOR_TEAL = 0x07CC;
const unsigned short COLOR_BLUEGRAY = 0x0B0C;
const unsigned short COLOR_BLUE = 0x026E;
const unsigned short COLOR_PURPLE = 0x7075;

inline void draw_power_symbol(M5Canvas canvas, int x, int y)
{
    canvas.fillArc(x, y, 9, 7, 0, 230, TFT_RED);
    canvas.fillArc(x, y, 9, 7, 310, 359, TFT_RED);
    canvas.fillRect(x - 1, y - 11, 3, 10, TFT_RED);
}

inline void draw_input_symbol(M5Canvas canvas, int x, int y, int bw)
{
    canvas.fillTriangle(x + 5, y, x - 3, y - 5, x - 3, y + 5, TFT_SILVER);
    canvas.fillRect(x - 10, y - 2, 10, 4, TFT_SILVER);
    for (int i = 0; i < 3; i++)
    {
        int w = bw - 6 - i;
        canvas.drawRoundRect(x - w / 2, y - w / 2, w, w, 3, TFT_SILVER);
    }
}

inline void draw_gear_symbol(M5Canvas canvas, int x, int y)
{
    canvas.pushImage(x - gearWidth / 2, y - gearWidth / 2, gearWidth, gearHeight, (uint16_t *)gearData, gearTransparency);
}

inline void draw_volup_symbol(M5Canvas canvas, int x, int y)
{
    canvas.fillTriangle(x - 8, y, x, y + 8, x, y - 8, TFT_SILVER);
    canvas.fillRect(x - 10, y - 3, 8, 6, TFT_SILVER);
    canvas.fillRect(x + 3, y - 1, 8, 2, TFT_SILVER);
    canvas.fillRect(x + 6, y - 4, 2, 8, TFT_SILVER);
}

inline void draw_mute_symbol(M5Canvas canvas, int x, int y)
{
    canvas.fillTriangle(x - 8, y, x, y + 8, x, y - 8, TFT_SILVER);
    canvas.fillRect(x - 10, y - 3, 8, 6, TFT_SILVER);
}

inline void draw_voldn_symbol(M5Canvas canvas, int x, int y)
{
    canvas.fillTriangle(x - 8, y, x, y + 8, x, y - 8, TFT_SILVER);
    canvas.fillRect(x - 10, y - 3, 8, 6, TFT_SILVER);
    canvas.fillRect(x + 3, y - 1, 8, 2, TFT_SILVER);
}

inline void draw_left_symbol(M5Canvas canvas, int x, int y)
{
    canvas.fillTriangle(x - 5, y, x + 2, y + 5, x + 2, y - 5, TFT_SILVER);
}

inline void draw_up_symbol(M5Canvas canvas, int x, int y)
{
    canvas.fillTriangle(x, y - 3, x - 5, y + 4, x + 5, y + 4, TFT_SILVER);
}

inline void draw_ok_symbol(M5Canvas canvas, int x, int y)
{
    canvas.fillArc(x, y, 7, 5, 0, 360, TFT_SILVER);
}

inline void draw_down_symbol(M5Canvas canvas, int x, int y)
{
    canvas.fillTriangle(x, y + 3, x - 5, y - 4, x + 5, y - 4, TFT_SILVER);
}

inline void draw_right_symbol(M5Canvas canvas, int x, int y)
{
    canvas.fillTriangle(x + 5, y, x - 2, y - 5, x - 2, y + 5, TFT_SILVER);
}

inline void draw_back_symbol(M5Canvas canvas, int x, int y)
{
    canvas.fillTriangle(x - 8, y - 5, x, y - 10, x, y, TFT_SILVER);
    canvas.fillArc(x + 1, y + 1, 8, 5, 255, 65, TFT_SILVER);
}

inline void draw_display_symbol(M5Canvas canvas, int x, int y, int bw)
{
    for (int i = 0; i < 3; i++)
    {
        int w = bw - 10 - i;
        canvas.drawRoundRect(x - w / 2, y - w / 2, w, w, 3, TFT_SILVER);
    }
}

inline void draw_home_symbol(M5Canvas canvas, int x, int y)
{
    canvas.fillTriangle(x - 9, y - 3, x + 9, y - 3, x, y - 9, TFT_SILVER);
    canvas.fillRect(x - 6, y - 3, 12, 12, TFT_SILVER);
    canvas.fillRect(x - 3, y + 1, 5, 9, COLOR_DARKRED);
}