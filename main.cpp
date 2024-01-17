#include <Arduino.h>
#include <M5Cardputer.h>
#include <M5StackUpdater.h>

#include <IRremote.hpp>
#include <functional>

#include "draw_helper.h"
#include "remote_keymap.h"

#define DISABLE_CODE_FOR_RECEIVER
#define SEND_PWM_BY_TIMER
#define IR_TX_PIN 44

M5Canvas canvas(&M5Cardputer.Display);

RemoteType remoteType = Sony;
Key *activeRemote;
uint activeRemoteKeyCount;
std::function<void(int, int)> activeRemoteIrSend;
// TODO: active button map/state for different layouts

int batteryPct = M5Cardputer.Power.getBatteryLevel();
int updateDelay = 0;

// define a bunch of display variables to make adjustments not a nightmare
int w = 240; // width
int h = 135; // height
int xm = 4;  // x margin
int ym = 4;  // y margin
int rm = 4;  // rect margin, used for background graphics
int bw = 28; // button width
int im = 2;  // button margin

// main background rectangle, for remote buttons
int rw = 6 * rm + 6 * bw + 3 * im;
int rh = 2 * rm + 3 * bw + 2 * im;
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
int r1 = ry + rm;
int r2 = r1 + bw + im;
int r3 = r2 + bw + im;
int c1 = rx + rm;
int c2 = c1 + bw + im;
int c3 = c2 + bw + 2 * rm;
int c4 = c3 + bw + im;
int c5 = c4 + bw + im;
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

  // draw background, sidebar, header graphics
  canvas.fillRoundRect(hx, hy, hw, hh, 8, COLOR_DARKGRAY);
  canvas.fillRoundRect(sx, sy, sw, sh, 8, COLOR_DARKGRAY);
  canvas.fillRoundRect(rx, ry, rw, rh, 8, COLOR_DARKGRAY);

  draw_title_text(&canvas, sx + (sw / 2), sy + 20);
  draw_remote_type_indicators(&canvas, hx + rm, hy + hh / 2, rm, remoteType);
  draw_battery_indicator(&canvas, c6 + 2, hy + (hh / 2), batteryPct);

  // TODO: different button layouts for different remotes

  // draw all layout for remote
  canvas.fillRoundRect(dx, dy, dw, dw, 10, COLOR_MEDGRAY);
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
  canvas.createSprite(w, h);

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

  if (millis() > updateDelay)
  {
    updateDelay = millis() + 2000;
    int newBatteryPct = M5Cardputer.Power.getBatteryLevel();
    if (newBatteryPct != batteryPct)
    {
      batteryPct = newBatteryPct;
      draw();
    }
  }
}
