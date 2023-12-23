#include <M5Cardputer.h>

struct Key
{
  char key;
  uint16_t address;
  uint8_t command;
  unsigned long data;
  // TODO: also support raw data
  // uint8_t dataBits;
  // uint8_t keyId;
};

Key LgKeyMap[] = {
    {'`', 0x4, 0x8, 0},            // power, KEY_ESC
    {'s', 0x4, 0x2, 0},            // volup
    {'z', 0x4, 0x3, 0},            // voldn
    {'m', 0x4, 0x9, 0},            // mute
    {';', 0x4, 0x40, 0},           // up
    {'.', 0x4, 0x41, 0},           // down
    {',', 0x4, 0x7, 0},            // left
    {'/', 0x4, 0x6, 0},            // right
    {' ', 0x4, 0x7C, 0},           // home
    {KEY_ENTER, 0x4, 0x44, 0},     // enter
    {KEY_BACKSPACE, 0x4, 0x28, 0}, // back
    {KEY_TAB, 0x4, 0xB, 0},        // input
    {KEY_LEFT_CTRL, 0x4, 0x43, 0}, // settings
};
const uint LgKeyMapSize = sizeof(LgKeyMap) / sizeof(Key);

Key SonyKeyMap[] = {
    {'`', 0x1, 0x15, 0},       // power, KEY_ESC
    {'s', 0x1, 0x12, 0},       // volup
    {'z', 0x1, 0x13, 0},       // voldn
    {'m', 0x1, 0x14, 0},       // mute
    {';', 0x1, 0x74, 0},       // up
    {'.', 0x1, 0x75, 0},       // down
    {',', 0x1, 0x34, 0},       // left
    {'/', 0x1, 0x33, 0},       // right
    {' ', 0x1, 0x60, 0},       // home
    {KEY_ENTER, 0x1, 0x65, 0}, // enter
    //{KEY_BACKSPACE, 0x97, 0x23, 0}, // back
    {KEY_BACKSPACE, 0x1, 0x63, 0}, // back
    {KEY_TAB, 0x1, 0x25, 0},       // input
    {KEY_LEFT_CTRL, 0x1, 0x36, 0}, // settings
};
const uint SonyKeyMapSize = sizeof(SonyKeyMap) / sizeof(Key);