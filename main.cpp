#include <Arduino.h>
#include <M5Cardputer.h>
#include <M5StackUpdater.h>

#include <IRremote.hpp>
#include <functional>

#include "draw_helper.h"
#include "remote_keymap.h"

#define DISABLE_CODE_FOR_RECEIVER
#define SEND_PWM_y_TIMER
#define IR_TX_PIN 44

M5Canvas canvas(&M5Cardputer.Display);

Key *activeRemote;
uint activeRemoteKeyCount;
;
std::function<void(int, int)> activeRemoteIrSend;
// TODO: active button map/state for different layouts

enum RemoteType
{
  Sony,
  Lg,
  Undef1,
  Undef2,
  End
};
RemoteType remoteType = Sony;

int batteryPct = M5Cardputer.Power.getBatteryLevel();
int batteryDelay = 0;

// define a bunch of display variables to make adjustments not a nightmare
int w = 240; // width
int h = 135; // height
int xm = 4;  // x margin
int ym = 4;  // y margin
int rm = 4;  // rect margin, used for background graphics
int bw = 28; // button width
int bm = 2;  // button margin

// main background rectangle, for remote buttons
int rw = 6 * rm + 6 * bw + 3 * bm;
int rh = 2 * rm + 3 * bw + 2 * bm;
int rx = w - xm - rw;
int ry = h - ym - rh;
// header rectangle
int hx = rx;
int hy = ym;
int hw = rw;
int hh = h - ym - rh - rm - ym;
// sidebar rectangle
int sx = xm;
int sy = ym;
int sw = rx - xm - rm;
int sh = h - ym - ym;

// specific to button layout
// TODO: refactor to be more dynamic
int r1 = ry + rm;
int r2 = r1 + bw + bm;
int r3 = r2 + bw + bm;
int c1 = rx + rm;
int c2 = c1 + bw + bm;
int c3 = c2 + bw + 2 * rm;
int c4 = c3 + bw + bm;
int c5 = c4 + bw + bm;
int c6 = c5 + bw + 2 * rm;
// directional square
int dw = c6 - c3;
int dx = c3 - rm;
int dy = ry;

struct Button
{
  char key;
  int x;
  int y;
  int w;
  int h;
  Symbol symbol;
  unsigned short color;
  bool pressed;
};

Button buttons[] = {
    {'`', c1, r1, bw, bw, Power, COLOR_PURPLE, false},             // power
    {KEY_TAB, c1, r2, bw, bw, Input, COLOR_PURPLE, false},         // input
    {KEY_LEFT_CTRL, c1, r3, bw, bw, Gear, COLOR_PURPLE, false},   // settings
    {'s', c2, r1, bw, bw, VolUp, COLOR_PURPLE, false},             // volup
    {'m', c2, r2, bw, bw, Mute, COLOR_PURPLE, false},             // mute
    {'z', c2, r3, bw, bw, VolDn, COLOR_PURPLE, false},             // voldn
    {',', c3, r2, bw, bw, Left, COLOR_BLUE, false},               // left
    {';', c4, r1, bw, bw, Up, COLOR_BLUE, false},               // up
    {KEY_ENTER, c4, r2, bw, bw, Ok, COLOR_BLUE, false},         // OK
    {'.', c4, r3, bw, bw, Down, COLOR_BLUE, false},               // down
    {'/', c5, r2, bw, bw, Right, COLOR_BLUE, false},               // right
    {KEY_BACKSPACE, c6, r1, bw, bw, Back, COLOR_BLUEGRAY, false}, // back
    {'\\', c6, r2, bw, bw, Display, COLOR_BLUEGRAY, false},          // ?
    {' ', c6, r3, bw, bw, Home, COLOR_BLUEGRAY, false},           // home
};
uint8_t buttonCount = sizeof(buttons) / sizeof(Button);

SPIClass SPI2;
void checkForMenuBoot()
{
  M5Cardputer.update();

  if (M5Cardputer.Keyboard.isKeyPressed('a'))
  {
    SPI2.begin(M5.getPin(m5::pin_name_t::sd_spi_sclk),
               M5.getPin(m5::pin_name_t::sd_spi_miso),
               M5.getPin(m5::pin_name_t::sd_spi_mosi),
               M5.getPin(m5::pin_name_t::sd_spi_ss));
    while (!SD.begin(M5.getPin(m5::pin_name_t::sd_spi_ss), SPI2))
    {
      delay(500);
    }

    updateFromFS(SD, "/menu.bin");
    ESP.restart();
  }
}

void setRemoteType(RemoteType type)
{
  switch (remoteType)
  {
  case Sony:
    activeRemote = SonyKeyMap;
    activeRemoteKeyCount = SonyKeyMapSize;
    activeRemoteIrSend = [](int address, int command)
    {
      IrSender.sendSony(address, command, 1, 12);
    };
    break;
  case Lg:
    activeRemote = LgKeyMap;
    activeRemoteKeyCount = LgKeyMapSize;
    activeRemoteIrSend = [](int address, int command)
    {
      IrSender.sendNEC(address, command, 1);
    };
    break;
  }
}

void draw()
{
  canvas.fillSprite(BLACK);

  // draw background graphics
  canvas.fillRoundRect(hx, hy, hw, hh, 8, COLOR_DARKGRAY);
  canvas.fillRoundRect(sx, sy, sw, sh, 8, COLOR_DARKGRAY);
  canvas.fillRoundRect(rx, ry, rw, rh, 8, COLOR_DARKGRAY);
  canvas.fillRoundRect(dx, dy, dw, dw, 10, COLOR_MEDGRAY);

  // reused vars for drawing symbols
  int x, y, w, h, bc, tc;

  // draw title text
  x = sx + (sw / 2);
  y = sy + 20;
  canvas.setTextColor(TFT_SILVER, COLOR_DARKGRAY);
  canvas.setTextDatum(middle_center);
  canvas.setTextSize(1.5);

  // bug? drawChar doesn't respect text datum
  canvas.drawString("N", x - 6, y);
  canvas.drawString("O", x - 6, y + 14);
  canvas.drawString("N", x - 6, y + 28);
  canvas.drawString("I", x - 6, y + 42);
  canvas.drawString("K", x - 6, y + 56);
  canvas.drawString("R", x + 6, y + 20);
  canvas.drawString("E", x + 6, y + 34);
  canvas.drawString("M", x + 6, y + 48);
  canvas.drawString("O", x + 6, y + 62);
  canvas.drawString("T", x + 6, y + 76);
  canvas.drawString("E", x + 6, y + 90);

  // TODO: seperate canvas for top bar?

  // draw remote type indicators
  w = 36;
  h = 17;
  x = hx + rm;
  y = hy + (hh - h) / 2;
  bc = remoteType == Sony ? COLOR_ORANGE : COLOR_MEDGRAY;
  tc = remoteType == Sony ? TFT_BLACK : TFT_SILVER;
  canvas.setTextColor(TFT_SILVER, COLOR_MEDGRAY);
  canvas.setTextSize(1.2);
  canvas.fillRoundRect(x, y, w, h, 3, bc);
  canvas.setTextColor(tc, bc);
  canvas.drawString("SONY", x + w / 2, y + h / 2);

  x = x + w + rm;
  bc = remoteType == Lg ? COLOR_ORANGE : COLOR_MEDGRAY;
  tc = remoteType == Lg ? TFT_BLACK : TFT_SILVER;
  canvas.fillRoundRect(x, y, w, h, 3, bc);
  canvas.setTextColor(tc, bc);
  canvas.drawString("LG", x + w / 2, y + h / 2);

  x = x + w + rm;
  bc = remoteType == Undef1 ? COLOR_ORANGE : COLOR_MEDGRAY;
  tc = remoteType == Undef1 ? TFT_BLACK : TFT_SILVER;
  canvas.fillRoundRect(x, y, w, h, 3, bc);
  canvas.setTextColor(tc, bc);
  canvas.drawString("TBD", x + w / 2, y + h / 2);

  x = x + w + rm;
  bc = remoteType == Undef2 ? COLOR_ORANGE : COLOR_MEDGRAY;
  tc = remoteType == Undef2 ? TFT_BLACK : TFT_SILVER;
  canvas.fillRoundRect(x, y, w, h, 3, bc);
  canvas.setTextColor(tc, bc);
  canvas.drawString("TBD", x + w / 2, y + h / 2);

  draw_battery_indicator(&canvas, c6 + 2, hy + (hh / 2), batteryPct);

  // TODO: different button layouts for different remotes

  // draw all buttons for remote
  for (auto button : buttons)
  {
    unsigned short color = button.pressed ? TFT_ORANGE : button.color;
    canvas.fillRoundRect(button.x, button.y, button.w, button.h, 3, color);
    draw_button_symbol(&canvas, button.symbol, button.x + button.w / 2,
                button.y + button.h / 2, button.w);
  }

  canvas.pushSprite(0, 0);
}

void setup()
{
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);

  checkForMenuBoot();

  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setBrightness(100);
  canvas.createSprite(240, 135);

  IrSender.begin(DISABLE_LED_FEEDBACK); // Start with IR_SEND_PIN as send pin
  IrSender.setSendPin(IR_TX_PIN);

  setRemoteType(Sony);
  draw();
}

void loop()
{
  M5Cardputer.update();
  if (M5Cardputer.Keyboard.isChange())
  {
    if (M5Cardputer.Keyboard.isKeyPressed(KEY_FN))
    {
      remoteType = (RemoteType)(remoteType + 1);
      if (remoteType == Undef1)
        remoteType = Sony;

      setRemoteType(remoteType);
    }

    for (int i = 0; i < activeRemoteKeyCount; i++)
    {
      Key key = activeRemote[i];

      if (M5Cardputer.Keyboard.isKeyPressed(key.key))
      {
        activeRemoteIrSend(key.address, key.command);

        // set button to pressed for drawing
        for (int i = 0; i < buttonCount; i++)
        {
          if (key.key == buttons[i].key)
          {
            buttons[i].pressed = true;
          }
        }
      }
    }

    draw();
    delay(60);
    for (int i = 0; i < buttonCount; i++)
      buttons[i].pressed = false;
    draw();
  }

  if (millis() > batteryDelay)
  {
    batteryDelay = millis() + 60000;
    batteryPct = M5Cardputer.Power.getBatteryLevel();
    draw();
  }
}
