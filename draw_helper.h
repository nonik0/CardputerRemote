#include <M5Cardputer.h>

#include "gear_bmp.h"
#include "remote_keymap.h"

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

enum Symbol { Power, Input, Gear, VolUp, Mute, VolDn, Left, Up, Ok, Down, Right, Back, Display, Home };

inline void draw_title_text(M5Canvas* canvas, int x, int y) {
  canvas->setTextColor(TFT_SILVER, COLOR_DARKGRAY);
  canvas->setTextDatum(middle_center);
  canvas->setTextSize(1.5);

  // bug? drawChar doesn't respect text datum
  canvas->drawString("N", x - 6, y);
  canvas->drawString("O", x - 6, y + 14);
  canvas->drawString("N", x - 6, y + 28);
  canvas->drawString("I", x - 6, y + 42);
  canvas->drawString("K", x - 6, y + 56);
  canvas->drawString("R", x + 6, y + 20);
  canvas->drawString("E", x + 6, y + 34);
  canvas->drawString("M", x + 6, y + 48);
  canvas->drawString("O", x + 6, y + 62);
  canvas->drawString("T", x + 6, y + 76);
  canvas->drawString("E", x + 6, y + 90);
}

inline void draw_remote_type_indicators(M5Canvas* canvas, int x, int y, int m, RemoteType remoteType) {
  int w = 36;
  int h = 17;
  y -= h / 2;
  unsigned short bc = remoteType == Sony ? COLOR_ORANGE : COLOR_MEDGRAY;
  unsigned short tc = remoteType == Sony ? TFT_BLACK : TFT_SILVER;
  canvas->setTextColor(TFT_SILVER, COLOR_MEDGRAY);
  canvas->setTextSize(1.2);
  canvas->fillRoundRect(x, y, w, h, 3, bc);
  canvas->setTextColor(tc, bc);
  canvas->drawString("SONY", x + w / 2, y + h / 2);

  x = x + w + m;
  bc = remoteType == Lg ? COLOR_ORANGE : COLOR_MEDGRAY;
  tc = remoteType == Lg ? TFT_BLACK : TFT_SILVER;
  canvas->fillRoundRect(x, y, w, h, 3, bc);
  canvas->setTextColor(tc, bc);
  canvas->drawString("LG", x + w / 2, y + h / 2);

  x = x + w + m;
  bc = remoteType == Undef1 ? COLOR_ORANGE : COLOR_MEDGRAY;
  tc = remoteType == Undef1 ? TFT_BLACK : TFT_SILVER;
  canvas->fillRoundRect(x, y, w, h, 3, bc);
  canvas->setTextColor(tc, bc);
  canvas->drawString("TBD", x + w / 2, y + h / 2);

  x = x + w + m;
  bc = remoteType == Undef2 ? COLOR_ORANGE : COLOR_MEDGRAY;
  tc = remoteType == Undef2 ? TFT_BLACK : TFT_SILVER;
  canvas->fillRoundRect(x, y, w, h, 3, bc);
  canvas->setTextColor(tc, bc);
  canvas->drawString("TBD", x + w / 2, y + h / 2);
}

inline void draw_battery_indicator(M5Canvas* canvas, int x, int y, int batteryPct) {  // draw battery indicator
  int battw = 24;
  int batth = 11;

  int ya = y - batth / 2;

  // determine battery color and charge width from charge level
  int chgw = (battw - 2) * batteryPct / 100;
  uint16_t batColor = COLOR_TEAL;
  if (batteryPct < 100)
  {
    int r = ((100 - batteryPct) / 100.0) * 256;
    int g = (batteryPct / 100.0) * 256;
    batColor = canvas->color565(r, g, 0);
  }
  canvas->fillRoundRect(x, ya, battw, batth, 2, TFT_SILVER);
  canvas->fillRect(x - 2, y - 2, 2, 4, TFT_SILVER);
  canvas->fillRect(x + 1, ya + 1, battw - 2 - chgw, batth - 2,
                  COLOR_DARKGRAY); // 1px margin from outer battery
  canvas->fillRect(x + 1 + battw - 2 - chgw, ya + 1, chgw, batth - 2,
                  batColor); // 1px margin from outer battery
}

inline void draw_power_symbol(M5Canvas* canvas, int x, int y)
{
    canvas->fillArc(x, y, 9, 7, 0, 230, TFT_RED);
    canvas->fillArc(x, y, 9, 7, 310, 359, TFT_RED);
    canvas->fillRect(x - 1, y - 11, 3, 10, TFT_RED);
}

inline void draw_input_symbol(M5Canvas* canvas, int x, int y, int bw)
{
    canvas->fillTriangle(x + 5, y, x - 3, y - 5, x - 3, y + 5, TFT_SILVER);
    canvas->fillRect(x - 10, y - 2, 10, 4, TFT_SILVER);
    for (int i = 0; i < 3; i++)
    {
        int w = bw - 6 - i;
        canvas->drawRoundRect(x - w / 2, y - w / 2, w, w, 3, TFT_SILVER);
    }
}

inline void draw_gear_symbol(M5Canvas* canvas, int x, int y)
{
    canvas->pushImage(x - gearWidth / 2, y - gearWidth / 2, gearWidth, gearHeight, (uint16_t *)gearData, gearTransparency);
}

inline void draw_volup_symbol(M5Canvas* canvas, int x, int y)
{    canvas->fillTriangle(x - 8, y, x, y + 8, x, y - 8, TFT_SILVER);
    canvas->fillRect(x - 10, y - 3, 8, 6, TFT_SILVER);
    canvas->fillRect(x + 3, y - 1, 8, 2, TFT_SILVER);
    canvas->fillRect(x + 6, y - 4, 2, 8, TFT_SILVER);
}

inline void draw_mute_symbol(M5Canvas* canvas, int x, int y)
{
    canvas->fillTriangle(x - 8, y, x, y + 8, x, y - 8, TFT_SILVER);
    canvas->fillRect(x - 10, y - 3, 8, 6, TFT_SILVER);
}

inline void draw_voldn_symbol(M5Canvas* canvas, int x, int y)
{
    canvas->fillTriangle(x - 8, y, x, y + 8, x, y - 8, TFT_SILVER);
    canvas->fillRect(x - 10, y - 3, 8, 6, TFT_SILVER);
    canvas->fillRect(x + 3, y - 1, 8, 2, TFT_SILVER);
}

inline void draw_left_symbol(M5Canvas* canvas, int x, int y)
{
    canvas->fillTriangle(x - 5, y, x + 2, y + 5, x + 2, y - 5, TFT_SILVER);
}

inline void draw_up_symbol(M5Canvas* canvas, int x, int y)
{
    canvas->fillTriangle(x, y - 3, x - 5, y + 4, x + 5, y + 4, TFT_SILVER);
}

inline void draw_ok_symbol(M5Canvas* canvas, int x, int y)
{
    canvas->fillArc(x, y, 7, 5, 0, 360, TFT_SILVER);
}

inline void draw_down_symbol(M5Canvas* canvas, int x, int y)
{
    canvas->fillTriangle(x, y + 3, x - 5, y - 4, x + 5, y - 4, TFT_SILVER);
}

inline void draw_right_symbol(M5Canvas* canvas, int x, int y)
{
    canvas->fillTriangle(x + 5, y, x - 2, y - 5, x - 2, y + 5, TFT_SILVER);
}

inline void draw_back_symbol(M5Canvas* canvas, int x, int y)
{
    canvas->fillTriangle(x - 8, y - 5, x, y - 10, x, y, TFT_SILVER);
    canvas->fillArc(x + 1, y + 1, 8, 5, 255, 65, TFT_SILVER);
}

inline void draw_display_symbol(M5Canvas* canvas, int x, int y, int bw)
{
    for (int i = 0; i < 3; i++)
    {
        int w = bw - 10 - i;
        canvas->drawRoundRect(x - w / 2, y - w / 2, w, w, 3, TFT_SILVER);
    }
}

inline void draw_home_symbol(M5Canvas* canvas, int x, int y)
{
    canvas->fillTriangle(x - 9, y - 3, x + 9, y - 3, x, y - 9, TFT_SILVER);
    canvas->fillRect(x - 6, y - 3, 12, 12, TFT_SILVER);
    canvas->fillRect(x - 3, y + 1, 5, 9, COLOR_DARKRED);
}

inline void draw_button_symbol(M5Canvas* canvas, Symbol symbol, int x, int y, int bw) {
    switch (symbol) {
        case Power:
            draw_power_symbol(canvas, x, y);
            break;
        case Input:
            draw_input_symbol(canvas, x, y, bw);
            break;
        case Gear:
            draw_gear_symbol(canvas, x, y);
            break;
        case VolUp:
            draw_volup_symbol(canvas, x, y);
            break;
        case Mute:
            draw_mute_symbol(canvas, x, y);
            break;
        case VolDn:
            draw_voldn_symbol(canvas, x, y);
            break;
        case Left:
            draw_left_symbol(canvas, x, y);
            break;
        case Up:
            draw_up_symbol(canvas, x, y);
            break;
        case Ok:
            draw_ok_symbol(canvas, x, y);
            break;
        case Down:
            draw_down_symbol(canvas, x, y);
            break;
        case Right:
            draw_right_symbol(canvas, x, y);
            break;
        case Back:
            draw_back_symbol(canvas, x, y);
            break;
        case Display:
            draw_display_symbol(canvas, x, y, bw);
            break;
        case Home:
            draw_home_symbol(canvas, x, y);
            break;
    }
}