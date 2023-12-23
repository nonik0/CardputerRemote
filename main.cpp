#include <Arduino.h>
#include <M5Cardputer.h>
#include <M5StackUpdater.h>

#include <IRremote.hpp>
#include <functional>

#include "gear_bmp.h"
#include "remote_keymap.h"

#define DISABLE_CODE_FOR_RECEIVER
#define SEND_PWM_y_TIMER
#define IR_TX_PIN 44

M5Canvas canvas(&M5Cardputer.Display);

Key *activeRemote;
uint activeRemoteKeyCount;
std::function<void(int, int)> activeRemoteIrSend;
// TODO: active button map/state for different layouts

enum RemoteType { Sony, Lg, Undef1, Undef2, End };
RemoteType remoteType = Sony;

int batteryPct = -1;

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

// define a bunch of display variables to make adjustments not a nightmare
// originally defined the square and made everything else relative
// may need to change in future for different layout support
int w = 250;  // width
int h = 135;  // height
int xm = 4;   // x margin
int ym = 4;   // y margin
int sm = 4;   // square margin, used for background graphics
int bw = 28;  // button width
int bm = 2;   // button margin

// background square (directional pad)
int sw = sm + bw + bm + bw + bm + bw + sm;  // square width
int sx = 240 - xm - sm - bw - sm - sw;
int sy = 135 - ym - sw;

// define rows and columns relative to square
int r1 = sy + sm;
int r2 = r1 + bw + bm;
int r3 = r2 + bw + bm;
int c1 = sx - sm - bw - bm - bw;
int c2 = c1 + bw + bm;
int c3 = sx + sm;
int c4 = c3 + bw + bm;
int c5 = c4 + bw + bm;
int c6 = sx + sw + sm;

// background rectangle, relative to square size
int rx = c1 - sm;
int ry = sy;
int rw = (c6 - c1) + bw + sm + sm;
int rh = sw;

// header rectangle, relative to rectangle
int hx = rx;
int hy = ym;
int hw = rw;
int hh = h - ym - rh - sm - ym;

// sidebar rectangle, relative to rectangle
int sbx = xm;
int sby = ym;
int sbw = rx - xm - sm;
int sbh = h - ym - ym;

struct Button {
  char key;
  int x;
  int y;
  int w;
  int h;
  String label;
  unsigned short color;
  bool pressed;
};

Button buttons[] = {
    {'`', c1, r1, bw, bw, "", COLOR_PURPLE, false},            // power
    {KEY_TAB, c1, r2, bw, bw, "", COLOR_PURPLE, false},        // mute
    {KEY_LEFT_CTRL, c1, r3, bw, bw, "", COLOR_PURPLE, false},  // settings
    {'s', c2, r1, bw, bw, "", COLOR_PURPLE, false},            // volup
    {'m', c2, r2, bw, bw, "", COLOR_PURPLE, false},            // mute
    {'z', c2, r3, bw, bw, "", COLOR_PURPLE, false},            // voldn
    {',', c3, r2, bw, bw, "", COLOR_BLUE, false},              // left
    {';', c4, r1, bw, bw, "", COLOR_BLUE, false},              // up
    {KEY_ENTER, c4, r2, bw, bw, "", COLOR_BLUE, false},        // OK
    {'.', c4, r3, bw, bw, "", COLOR_BLUE, false},              // down
    {'/', c5, r2, bw, bw, "", COLOR_BLUE, false},              // right
    {KEY_BACKSPACE, c6, r1, 27, 27, "", COLOR_BLUEGRAY, false},    // back
    {'\\', c6, r2, 27, 27, "", COLOR_BLUEGRAY, false},             // ?
    {' ', c6, r3, 27, 27, "", COLOR_BLUEGRAY, false},              // home
};
uint8_t buttonCount = sizeof(buttons) / sizeof(Button);

SPIClass SPI2;
void checkForMenuBoot() {
  M5Cardputer.update();

  if (M5Cardputer.Keyboard.isKeyPressed('a')) {
    SPI2.begin(M5.getPin(m5::pin_name_t::sd_spi_sclk),
               M5.getPin(m5::pin_name_t::sd_spi_miso),
               M5.getPin(m5::pin_name_t::sd_spi_mosi),
               M5.getPin(m5::pin_name_t::sd_spi_ss));
    while (!SD.begin(M5.getPin(m5::pin_name_t::sd_spi_ss), SPI2)) {
      delay(500);
    }

    updateFromFS(SD, "/menu.bin");
    ESP.restart();
  }
}

void draw() {
  canvas.fillSprite(BLACK);

  // draw background graphics
  canvas.fillRoundRect(hx, hy, hw, hh, 6, COLOR_DARKGRAY);
  canvas.fillRoundRect(sbx, sby, sbw, sbh, 6, COLOR_DARKGRAY);
  canvas.fillRoundRect(rx, ry, rw, rh, 6, COLOR_DARKGRAY);
  canvas.fillRoundRect(sx, sy, sw, sw, 8, COLOR_MEDGRAY);

  // reused vars for drawing symbols
  int x, y, w, h, bc, tc;

  // draw title text
  x = sbx + (sbw / 2);
  y = sby + 20;
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
  x = hx + sm;
  y = hy + (hh - h) / 2;
  bc = remoteType == Sony ? COLOR_ORANGE : COLOR_MEDGRAY;
  tc = remoteType == Sony ? TFT_BLACK : TFT_SILVER;
  canvas.setTextColor(TFT_SILVER, COLOR_MEDGRAY);
  canvas.setTextSize(1.2);
  canvas.fillRoundRect(x, y, w, h, 3, bc);
  canvas.setTextColor(tc, bc);
  canvas.drawString("SONY", x + w / 2, y + h / 2);

  x = x + w + sm;
  bc = remoteType == Lg ? COLOR_ORANGE : COLOR_MEDGRAY;
  tc = remoteType == Lg ? TFT_BLACK : TFT_SILVER;
  canvas.fillRoundRect(x, y, w, h, 3, bc);
  canvas.setTextColor(tc, bc);
  canvas.drawString("LG", x + w / 2, y + h / 2);

  x = x + w + sm;
  bc = remoteType == Undef1 ? COLOR_ORANGE : COLOR_MEDGRAY;
  tc = remoteType == Undef1 ? TFT_BLACK : TFT_SILVER;
  canvas.fillRoundRect(x, y, w, h, 3, bc);
  canvas.setTextColor(tc, bc);
  canvas.drawString("TBD", x + w / 2, y + h / 2);

  x = x + w + sm;
  bc = remoteType == Undef2 ? COLOR_ORANGE : COLOR_MEDGRAY;
  tc = remoteType == Undef2 ? TFT_BLACK : TFT_SILVER;
  canvas.fillRoundRect(x, y, w, h, 3, bc);
  canvas.setTextColor(tc, bc);
  canvas.drawString("TBD", x + w / 2, y + h / 2);

  // draw battery indicator
  int battw = bw - 4;
  int batth = 11;
  x = c6 + 2;
  y = hy + (hh - batth) / 2;
  // determine battery color and charge width from charge level
  int chgw = (battw - 2) * batteryPct / 100;
  uint16_t batColor = COLOR_TEAL;
  if (batteryPct < 100) {
    int r = ((100 - batteryPct) / 100.0) * 256;
    int g = (batteryPct / 100.0) * 256;
    batColor = canvas.color565(r, g, 0);
  }
  canvas.fillRoundRect(x, y, battw, batth, 2, TFT_SILVER);
  canvas.fillRect(x - 2, hy + (hh / 2) - 2, 2, 4, TFT_SILVER);
  canvas.fillRect(x + 1, y + 1, battw - 2 - chgw, batth - 2,
                  COLOR_DARKGRAY);  // 1px margin from outer battery
  canvas.fillRect(x + 1 + battw - 2 - chgw, y + 1, chgw, batth - 2,
                  batColor);  // 1px margin from outer battery

  // TODO: different button layouts for different remotes
  // TODO: would need to redefine layout vars above (not based on square size)

  // draw all buttons for remote
  for (auto button : buttons) {
    unsigned short color = button.pressed ? TFT_ORANGE : button.color;
    canvas.fillRoundRect(button.x, button.y, button.w, button.h, 3, color);
    // canvas.setTextColor(TFT_SILVER, color);
    // canvas.drawString(button.label, button.x + button.w / 2, button.y +
    // button.h / 2);
  }

  // draw power symbol
  x = c1 + bw / 2;
  y = r1 + bw / 2;
  canvas.fillArc(x, y, 9, 7, 0, 230, TFT_RED);
  canvas.fillArc(x, y, 9, 7, 310, 359, TFT_RED);
  canvas.fillRect(x - 1, y - 11, 3, 10, TFT_RED);

  // draw input symbol
  x = c1 + bw / 2;
  y = r2 + bw / 2;
  canvas.fillTriangle(x + 5, y, x - 3, y - 5, x - 3, y + 5, TFT_SILVER);
  canvas.fillRect(x - 10, y - 2, 10, 4, TFT_SILVER);
  canvas.drawRoundRect(c1 + 4, r2 + 4, bw - 8, bw - 8, 3, TFT_SILVER);
  canvas.drawRoundRect(c1 + 3, r2 + 3, bw - 6, bw - 6, 3, TFT_SILVER);
  // canvas.drawRoundRect(c1 + 2, r2 + 2, bw - 4, bw - 4, 3, TFT_SILVER);
  // canvas.drawRoundRect(c1 + 1, r2 + 1, bw - 2, bw - 2, 3, TFT_SILVER);

  // draw gear symbol
  x = c1;
  y = r3;
  int o = (bw - gearWidth) / 2;
  canvas.pushImage(x + o, y + o, gearWidth, gearHeight, (uint16_t *)gearData,
                   gearTransparency);

  // draw volup symbol
  x = c2 + bw / 2;
  y = r1 + bw / 2;
  canvas.fillTriangle(x - 8, y, x, y + 8, x, y - 8, TFT_SILVER);
  canvas.fillRect(x - 10, y - 3, 8, 6, TFT_SILVER);
  canvas.fillRect(x + 3, y - 1, 8, 2, TFT_SILVER);
  canvas.fillRect(x + 6, y - 4, 2, 8, TFT_SILVER);

  // draw mute symbol
  x = c2 + bw / 2;
  y = r2 + bw / 2;
  canvas.fillTriangle(x - 8, y, x, y + 8, x, y - 8, TFT_SILVER);
  canvas.fillRect(x - 10, y - 3, 8, 6, TFT_SILVER);

  // draw voldn symbol
  x = c2 + bw / 2;
  y = r3 + bw / 2;
  canvas.fillTriangle(x - 8, y, x, y + 8, x, y - 8, TFT_SILVER);
  canvas.fillRect(x - 10, y - 3, 8, 6, TFT_SILVER);
  canvas.fillRect(x + 3, y - 1, 8, 2, TFT_SILVER);

  // draw left symbol
  x = c3 + bw / 2;
  y = r2 + bw / 2;
  canvas.fillTriangle(x - 5, y, x + 2, y + 5, x + 2, y - 5, TFT_SILVER);

  // draw up symbol
  x = c4 + bw / 2;
  y = r1 + bw / 2;
  canvas.fillTriangle(x, y - 3, x - 5, y + 4, x + 5, y + 4, TFT_SILVER);

  // draw ok symbol
  x = c4 + bw / 2;
  y = r2 + bw / 2;
  canvas.fillArc(x, y, 7, 5, 0, 360, TFT_SILVER);

  // draw down symbol
  x = c4 + bw / 2;
  y = r3 + bw / 2;
  canvas.fillTriangle(x, y + 3, x - 5, y - 4, x + 5, y - 4, TFT_SILVER);

  // draw right symbol
  x = c5 + bw / 2;
  y = r2 + bw / 2;
  canvas.fillTriangle(x + 5, y, x - 2, y - 5, x - 2, y + 5, TFT_SILVER);

  // draw back symbol
  x = c6 + bw / 2;
  y = r1 + bw / 2;
  canvas.fillTriangle(x - 8, y - 5, x, y - 10, x, y, TFT_SILVER);
  canvas.fillArc(x + 1, y + 1, 8, 5, 255, 65, TFT_SILVER);

  // draw display symbol
  canvas.drawRoundRect(c6 + 6, r2 + 6, bw - 12, bw - 12, 3, TFT_SILVER);
  canvas.drawRoundRect(c6 + 5, r2 + 5, bw - 10, bw - 10, 3, TFT_SILVER);
  // canvas.drawRoundRect(c6 + 4, r2 + 4, bw - 8, bw - 8, 3, TFT_SILVER);

  // draw home symbol
  x = c6 + bw / 2;
  y = r3 + bw / 2;
  canvas.fillTriangle(x - 9, y - 3, x + 9, y - 3, x, y - 9, TFT_SILVER);
  canvas.fillRect(x - 6, y - 3, 12, 12, TFT_SILVER);
  canvas.fillRect(x - 3, y + 1, 5, 9, COLOR_DARKRED);

  canvas.pushSprite(0, 0);
}

void setup() {
  auto cfg = M5.config();
  M5Cardputer.begin(cfg, true);
  M5Cardputer.Display.setRotation(1);
  M5Cardputer.Display.setBrightness(90);
  canvas.createSprite(240, 135);

  checkForMenuBoot();

  IrSender.begin(DISABLE_LED_FEEDBACK);  // Start with IR_SEND_PIN as send pin
  IrSender.setSendPin(IR_TX_PIN);

  draw();
}

void loop() {
  M5Cardputer.update();
  if (M5Cardputer.Keyboard.isChange()) {
    if (M5Cardputer.Keyboard.isKeyPressed(KEY_FN)) {
      remoteType = (RemoteType)(remoteType + 1);
      if (remoteType == Undef1) remoteType = Sony;

      switch (remoteType) {
        case Sony:
          activeRemote = SonyKeyMap;
          activeRemoteKeyCount = SonyKeyMapSize;
          activeRemoteIrSend = [](int address, int command) {
            IrSender.sendSony(address, command, 1, 12);
          };
          break;
        case Lg:
          activeRemote = LgKeyMap;
          activeRemoteKeyCount = LgKeyMapSize;
          activeRemoteIrSend = [](int address, int command) {
            IrSender.sendNEC(address, command, 32);
          };
          break;
      }
    }

    for (int i = 0; i < activeRemoteKeyCount; i++) {
      Key key = activeRemote[i];

      if (M5Cardputer.Keyboard.isKeyPressed(key.key)) {
        activeRemoteIrSend(key.address, key.command);

        // set button to pressed for drawing
        for (int i = 0; i < buttonCount; i++) {
          if (key.key == buttons[i].key) {
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

  // TODO: debounce battery check
  int newBatteryPct = M5.Power.getBatteryLevel();
  if (batteryPct != newBatteryPct) {
    batteryPct = newBatteryPct;
    draw();
  }
}
