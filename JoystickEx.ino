#include "Keyboard.h"
#include "Mouse.h"

#include <vector>

static uint8_t buttons[] = {
    A0, A1, A2, A3, A4, A5, A6, A7
};
static bool buttonsState[8];

#define DEF_UP 0
#define DEF_DOWN 1
#define DEF_LEFT 2
#define DEF_RIGHT 3
static uint8_t joyL[] = { 52, 50 ,48, 46 };
static uint8_t joyR[] = { 53, 51, 49, 47 };
static bool joyLState[4];
static bool joyRState[4];

#define DEF_R 0
#define DEF_G 1
#define DEF_B 2
static uint8_t ledL[] = { 44, 42, 40 };
static uint8_t ledR[] = { 45, 43, 41 };
static bool ledLState[3];
static bool ledRState[3];
#define ledLON(v) ledLState[v] = true; digitalWrite(ledL[v], !ledLState[v]);
#define ledRON(v) ledRState[v] = true; digitalWrite(ledR[v], !ledRState[v]);
#define ledLOFF(v) ledLState[v] = false; digitalWrite(ledL[v], !ledLState[v]);
#define ledROFF(v) ledRState[v] = false; digitalWrite(ledR[v], !ledRState[v]);
#define ledLBIK(v) ledLState[v] = !ledLState[v]; digitalWrite(ledL[v], !ledLState[v]);
#define ledRBIK(v) ledRState[v] = !ledRState[v]; digitalWrite(ledR[v], !ledRState[v]);

static std::vector<uint8_t> tmpState(32);
void keybdPress(uint8_t v) {
    for (uint8_t i = 0; i < tmpState.size(); i++) {
        if (tmpState[i] == v) return;
    }
    tmpState.push_back(v);
    Keyboard.press(v);
}
void keybdRelease(uint8_t v) {
    for (uint8_t i = 0; i < tmpState.size(); i++) {
        if (tmpState[i] == v) {
            tmpState.erase(tmpState.begin() + i);
            Keyboard.release(v);
        }
    }
}

enum Mode {
    MODE_TOUHOU = 0,
    MODE_BESIEGE = 1,
    MODE_DIRT = 2,
    MODE_RAIL = 3,
    MODE_OSU = 4,
    MODE_MOUSE = 5,
    MODE_CHOICE = 7
};
static Mode mode = MODE_MOUSE;

enum Stall {
    DIRT_R = 0,
    DIRT_0 = 1,
    DIRT_1 = 2,
    DIRT_2 = 3,
    DIRT_3 = 4,
    DIRT_4 = 5,
    DIRT_5 = 6,
    DIRT_6 = 7
};
static Stall dirtStall = DIRT_0;

void setup() {
    uint8_t i;
    for (i = 0; i < 8; i++) {
        pinMode(buttons[i], INPUT_PULLUP);
        buttonsState[i] = false;
    }
    for (i = 0; i < 4; i++) {
        pinMode(joyL[i], INPUT_PULLUP);
        pinMode(joyR[i], INPUT_PULLUP);
        joyLState[i] = false;
        joyRState[i] = false;
    }
    for (i = 0; i < 3; i++) {
        pinMode(ledL[i], OUTPUT);
        digitalWrite(ledL[i], HIGH);
        pinMode(ledR[i], OUTPUT);
        digitalWrite(ledR[i], HIGH);
        ledLState[i] = false;
        ledRState[i] = false;
    }

    delay(3000);

    ledLON(DEF_G);
    ledRON(DEF_G);

    Mouse.begin();
    Keyboard.begin();
}

void scan() {
    #define check(v) digitalRead(v) == LOW
    uint8_t i;

    for (i = 0; i < 8; i++)
        buttonsState[i] = check(buttons[i]);
    for (i = 0; i < 4; i++) {
        joyLState[i] = check(joyL[i]);
        joyRState[i] = check(joyR[i]);
    }
}

void sniff() {
    scan();

    if (
        joyLState[DEF_RIGHT] && joyRState[DEF_LEFT] &&
        buttonsState[4] && buttonsState[7]
    ) {
        short count = 0;
        Keyboard.releaseAll();
        Mouse.release(MOUSE_LEFT);
        Mouse.release(MOUSE_RIGHT);
        ledLOFF(DEF_G); ledROFF(DEF_G);
        while (
            joyLState[DEF_RIGHT] && joyRState[DEF_LEFT] &&
            buttonsState[4] && buttonsState[7]
        ) {
            delay(1);
            scan();
            count += 1;

            if (count % 500 == 0) {
                ledLBIK(DEF_B);
                ledRBIK(DEF_B);
            }

            if (count > 3000) {
                ledLON(DEF_R); ledRON(DEF_R);
                while (
                    joyLState[DEF_RIGHT] && joyRState[DEF_LEFT] &&
                    buttonsState[4] && buttonsState[7]
                ) scan();
                mode = MODE_CHOICE;
                delay(500);
                ledLOFF(DEF_B); ledROFF(DEF_B);
                break;
            }
        }
        ledLON(DEF_G); ledRON(DEF_G);
    }
}

void operate() {
    #define KEYBD(v, s) if (s) keybdPress(v); else keybdRelease(v);
    #define MOUSE(v, s) if (s && !Mouse.isPressed(v)) Mouse.press(v); else if (!s && Mouse.isPressed(v)) Mouse.release(v);

    switch (mode) {
        case MODE_TOUHOU:
            KEYBD(KEY_UP_ARROW, joyLState[DEF_UP] || joyRState[DEF_UP]);
            KEYBD(KEY_DOWN_ARROW, joyLState[DEF_DOWN] || joyRState[DEF_DOWN]);
            KEYBD(KEY_LEFT_ARROW, joyLState[DEF_LEFT] || joyRState[DEF_LEFT]);
            KEYBD(KEY_RIGHT_ARROW, joyLState[DEF_RIGHT] || joyRState[DEF_RIGHT]);
            KEYBD('a', buttonsState[0]); KEYBD('z', buttonsState[4]);
            KEYBD('s', buttonsState[1]); KEYBD('x', buttonsState[5]);
            KEYBD('d', buttonsState[2]); KEYBD('c', buttonsState[6]);
            KEYBD('f', buttonsState[3]); KEYBD('v', buttonsState[7]);
            KEYBD(KEY_ESC, buttonsState[3] && buttonsState[7]);
            break;
        case MODE_BESIEGE:
            KEYBD('t', joyLState[DEF_UP]);
            KEYBD('g', joyLState[DEF_DOWN]);
            KEYBD('f', joyLState[DEF_LEFT]);
            KEYBD('h', joyLState[DEF_RIGHT]);
            KEYBD(KEY_UP_ARROW, joyRState[DEF_UP]);
            KEYBD(KEY_DOWN_ARROW, joyRState[DEF_DOWN]);
            KEYBD(KEY_LEFT_ARROW, joyRState[DEF_LEFT]);
            KEYBD(KEY_RIGHT_ARROW, joyRState[DEF_RIGHT]);
            KEYBD('u', buttonsState[0]); KEYBD('j', buttonsState[4]);
            KEYBD('i', buttonsState[1]); KEYBD('k', buttonsState[5]);
            KEYBD('o', buttonsState[2]); KEYBD('l', buttonsState[6]);
            KEYBD('p', buttonsState[3]); KEYBD(';', buttonsState[7]);
            KEYBD(' ', buttonsState[3] && buttonsState[7]);
            break;
        case MODE_DIRT:
            if (joyLState[DEF_LEFT]) {
                delay(20);
                while (joyLState[DEF_LEFT]) {
                    scan(); delay(20);
                }
                dirtStall = ((dirtStall > DIRT_R) ? (Stall)(dirtStall - 1) : DIRT_R);
            }
            if (joyLState[DEF_RIGHT]) {
                delay(20);
                while (joyLState[DEF_RIGHT]) {
                    scan(); delay(20);
                }
                dirtStall = ((dirtStall < DIRT_6) ? (Stall)(dirtStall + 1) : DIRT_6);
            }
            KEYBD('`', dirtStall == DIRT_R);
            KEYBD('1', dirtStall == DIRT_1);
            KEYBD('2', dirtStall == DIRT_2);
            KEYBD('3', dirtStall == DIRT_3);
            KEYBD('4', dirtStall == DIRT_4);
            KEYBD('5', dirtStall == DIRT_5);
            KEYBD('6', dirtStall == DIRT_6);
            KEYBD(KEY_UP_ARROW, joyLState[DEF_UP]);
            KEYBD(KEY_DOWN_ARROW, joyLState[DEF_DOWN]);
            KEYBD(KEY_END, joyRState[DEF_UP]);
            KEYBD(' ', joyRState[DEF_DOWN]);
            KEYBD(KEY_LEFT_ARROW, joyRState[DEF_LEFT]);
            KEYBD(KEY_RIGHT_ARROW, joyRState[DEF_RIGHT]);
            KEYBD('a', buttonsState[0]); KEYBD('z', buttonsState[4]);
            KEYBD('s', buttonsState[1]); KEYBD('x', buttonsState[5]);
            KEYBD('d', buttonsState[2]); KEYBD('c', buttonsState[6]);
            KEYBD('f', buttonsState[3]); KEYBD('v', buttonsState[7]);
            KEYBD(KEY_ESC, buttonsState[3] && buttonsState[7]);
            break;
        case MODE_RAIL:
            KEYBD('w', joyLState[DEF_UP]);
            KEYBD('s', joyLState[DEF_DOWN]);
            KEYBD(',', joyLState[DEF_LEFT]);
            KEYBD('.', joyLState[DEF_RIGHT]);
            KEYBD('d', joyRState[DEF_UP]);
            KEYBD('a', joyRState[DEF_DOWN]);
            KEYBD(';', joyRState[DEF_LEFT]);
            KEYBD('\'', joyRState[DEF_RIGHT]);
            KEYBD('x', buttonsState[0]); KEYBD('b', buttonsState[4]);
            KEYBD('h', buttonsState[1]); KEYBD('l', buttonsState[5]);
            KEYBD('[', buttonsState[2]); KEYBD(']', buttonsState[6]);
            KEYBD('p', buttonsState[3]); KEYBD('g', buttonsState[7]);
            KEYBD(KEY_ESC, buttonsState[3] && buttonsState[7]);
            break;
        case MODE_OSU:
            KEYBD(KEY_UP_ARROW, joyLState[DEF_UP] || joyRState[DEF_UP]);
            KEYBD(KEY_DOWN_ARROW, joyLState[DEF_DOWN] || joyRState[DEF_DOWN]);
            KEYBD(KEY_LEFT_ARROW, joyLState[DEF_LEFT] || joyRState[DEF_LEFT]);
            KEYBD(KEY_RIGHT_ARROW, joyLState[DEF_RIGHT] || joyRState[DEF_RIGHT]);
            KEYBD('d', buttonsState[0]); KEYBD('s', buttonsState[4]);
            KEYBD('f', buttonsState[1]); KEYBD(' ', buttonsState[5]);
            KEYBD('j', buttonsState[2]); KEYBD(' ', buttonsState[6]);
            KEYBD('k', buttonsState[3]); KEYBD('l', buttonsState[7]);
            KEYBD(KEY_ESC, buttonsState[3] && buttonsState[7]);
            break;
        case MODE_MOUSE:
            if (joyLState[DEF_UP]) Mouse.move(0, -1, 0);
            if (joyLState[DEF_DOWN]) Mouse.move(0, 1, 0);
            if (joyLState[DEF_LEFT]) Mouse.move(-1, 0, 0);
            if (joyLState[DEF_RIGHT]) Mouse.move(1, 0, 0);
            
            if (joyRState[DEF_UP]) Mouse.move(0, -1, 0);
            if (joyRState[DEF_DOWN]) Mouse.move(0, 1, 0);
            if (joyRState[DEF_LEFT]) Mouse.move(-1, 0, 0);
            if (joyRState[DEF_RIGHT]) Mouse.move(1, 0, 0);

            MOUSE(MOUSE_LEFT, (buttonsState[0] || buttonsState[1]));
            MOUSE(MOUSE_RIGHT, (buttonsState[2] || buttonsState[3]));
            MOUSE(MOUSE_LEFT, (buttonsState[4] || buttonsState[5]));
            MOUSE(MOUSE_RIGHT, (buttonsState[6] || buttonsState[7]));

            delay(2);
            break;
        default:
            break;
    }
}

void loop() {
    uint8_t i;

    sniff();
    operate();

    if (mode == MODE_CHOICE) {
        tmpState.clear();
        scan();
        for (i = 0; i < 8; i++) {
            if (buttonsState[i]) {
                while (buttonsState[i]) scan();
                mode = (Mode)i;
                ledLOFF(DEF_R); ledROFF(DEF_R);
            }
        }
    }
}
