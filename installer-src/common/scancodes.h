// Define special internal scan code values

#define ISC_F1       0x01	// F1 key
#define ISC_F2       0x02	// F2 key
#define ISC_F3       0x03	// F3 key
#define ISC_F4       0x04	// F4 key
#define ISC_F5       0x05	// F5 key
#define ISC_F6       0x06	// F6 key
#define ISC_F7       0x07	// F7 key
#define ISC_F8       0x08	// F8 key
#define ISC_F9       0x09	// F9 key
#define ISC_F10      0x0A	// F10 key
#define ISC_F11      0x0B	// F11 key
#define ISC_F12      0x0C	// F12 key
#define ISC_ESC      0x10	// Escape
#define ISC_INS      0x11	// Insert
#define ISC_HOME     0x12	// Home
#define ISC_PGUP     0x13	// Page up
#define ISC_DELETE   0x14	// Delete
#define ISC_END      0x15	// End
#define ISC_PGDN     0x16	// Page down
#define ISC_UPARW    0x17	// Up arrow
#define ISC_LTARW    0x18	// Left arrow
#define ISC_DNARW    0x19	// Down arrow
#define ISC_RTARW    0x1A	// Right arrow
#define ISC_KP0      0x20	// Keypad 0
#define ISC_KPINS    0x20	// Keypad insert
#define ISC_KP1      0x21	// Keypad 1
#define ISC_KPEND    0x21	// Keypad end
#define ISC_KP2      0x22	// Keypad 2
#define ISC_KPDNARW  0x22	// Keypad down arrow
#define ISC_KP3      0x23	// Keypad 3
#define ISC_KPPGDN   0x23	// Keypad page down
#define ISC_KP4      0x24	// Keypad 4
#define ISC_KPLTARW  0x24	// Keypad left arrow
#define ISC_KP5      0x25	// Keypad 5
#define ISC_KP6      0x26	// Keypad 6
#define ISC_KPRTARW  0x26	// Keypad right arrow
#define ISC_KP7      0x27	// Keypad 7
#define ISC_KPHOME   0x27	// Keypad home
#define ISC_KP8      0x28	// Keypad 8
#define ISC_KPUPARW  0x28	// Keypad up arrow
#define ISC_KP9      0x29	// Keypad 9
#define ISC_KPPGUP   0x29	// Keypad page up
#define ISC_KPPER    0x2A	// Keypad .
#define ISC_KPDELETE 0x2A	// Keypad Del
#define ISC_KPSLSH   0x2B	// Keypad /
#define ISC_KPSTAR   0x2C	// Keypad *
#define ISC_KPMINUS  0x2D	// Keypad - (!!!)
#define ISC_KPPLUS   0x2E	// Keypad +
#define ISC_KPENT    0x2F	// Keypad enter
#define ISC_ENT      0x38	// Enter
#define ISC_LF       0x39	// Line feed
#define ISC_BS       0x3A	// Backspace
#define ISC_HT       0x3B	// Tab
#define ISC_DEL      0x3C	// DEL
#define ISC_GRAVE    0x40	// Grave or ~
#define ISC_1        0x41	// 1 or !
#define ISC_2        0x42	// 2 or @
#define ISC_3        0x43	// 3 or #
#define ISC_4        0x44	// 4 or $
#define ISC_5        0x45	// 5 or %
#define ISC_6        0x46	// 6 or ^
#define ISC_7        0x47	// 7 or &
#define ISC_8        0x48	// 8 or *
#define ISC_9        0x49	// 9 or (
#define ISC_0        0x4A	// 0 or )
#define ISC_MINUS    0x4B	// - or _
#define ISC_EQUAL    0x4C	// = or +
#define ISC_Q        0x4D	// Q
#define ISC_W        0x4E	// W
#define ISC_E        0x4F	// E
#define ISC_R        0x50	// R
#define ISC_T        0x51	// T
#define ISC_Y        0x52	// Y
#define ISC_U        0x53	// U
#define ISC_I        0x54	// I
#define ISC_O        0x55	// O
#define ISC_P        0x56	// P
#define ISC_LBRKT    0x57	// [ or {
#define ISC_RBRKT    0x58	// ] or }
#define ISC_BSLSH    0x59	// \ or |
#define ISC_A        0x5A	// A
#define ISC_S        0x5B	// S
#define ISC_D        0x5C	// D
#define ISC_F        0x5D	// F
#define ISC_G        0x5E	// G
#define ISC_H        0x5F	// H
#define ISC_J        0x60	// J
#define ISC_K        0x61	// K
#define ISC_L        0x62	// L
#define ISC_SEMI     0x63	// ;
#define ISC_QUOTE    0x64	// ' or "
#define ISC_Z	     0x65	// Z
#define ISC_X        0x66	// X
#define ISC_C        0x67	// C
#define ISC_V        0x68	// V
#define ISC_B        0x69	// B
#define ISC_N        0x6A	// N
#define ISC_M        0x6B	// M
#define ISC_COMMA    0x6C	// , or <
#define ISC_PER      0x6D	// . or >
#define ISC_SLSH     0x6E	// / or ?
#define ISC_SPACE    0x6F	// Space
#define ISC_SCLLCK   0x73	// Scroll lock
#define ISC_NUMLCK   0x74	// Num lock
#define ISC_CAPLCK   0x75	// Caps lock
#define ISC_LFTSHF   0x76	// Left shift
#define ISC_RHTSHF   0x77	// Right shift
#define ISC_LFTALT   0x78	// Left Alt
#define ISC_RHTALT   0x79	// Right Alt
#define ISC_LFTCTL   0x7A	// Left control
#define ISC_RHTCTL   0x7B	// Right control
#define ISC_PRTSCN   0x7C	// Print screen
#define ISC_BREAK    0x7D	// Break
#define ISC_PAUSE    0x7E	// Pause
#define ISC_SYSSHF   0x7F	// System shift
