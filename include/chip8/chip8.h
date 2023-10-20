#ifndef CHIP8_H_
#define CHIP8_H_

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "raylib.h"

#define MAX_MEM 4096
#define AVL_MEM (MAX_MEM - 0x200)
#define REGISTERS 16
#define MAX_STACK 1000

#ifdef ORIGINAL_CHIP8
#define CHIP8_SCREEN_WIDTH 64
#define CHIP8_SCREEN_HEIGHT 32
#else
#define CHIP8_SCREEN_WIDTH 128
#define CHIP8_SCREEN_HEIGHT 64
#endif

#define DEFAULT_FONT_ADDR 0x50

#define CHIP8_INVALID UINT16_MAX 

typedef uint16_t Chip8Inst; 

typedef struct Chip8State {
    uint8_t memory[MAX_MEM];
    uint8_t registers[REGISTERS];
    uint16_t pc; // Program counter
    uint16_t ir; // Index register
    uint16_t stack[MAX_STACK]; // Stack for calling subroutines
    size_t sp; // Stack pointer, not used in the original CHIP-8 but useful for not implementing a dynamic array
    bool screen[CHIP8_SCREEN_HEIGHT][CHIP8_SCREEN_WIDTH]; // Screen buffer
    uint8_t delay_timer;
    uint8_t sound_timer;
    bool halt; // "Switch" of the interpreter
} Chip8State;

typedef enum Chip8Res {
    CHIP8_ERROR,
    CHIP8_SUCCESS,
} Chip8Res;

extern const uint8_t Chip8DefaultFont[];       // Byte font used for some characters

typedef enum Chip8Key { // Key of the original CHIP-8 keyboard
    CHIP8_0_KEY,
    CHIP8_1_KEY,
    CHIP8_2_KEY,
    CHIP8_3_KEY,
    CHIP8_4_KEY,
    CHIP8_5_KEY,
    CHIP8_6_KEY,
    CHIP8_7_KEY,
    CHIP8_8_KEY,
    CHIP8_9_KEY,
    CHIP8_A_KEY,
    CHIP8_B_KEY,
    CHIP8_C_KEY,
    CHIP8_D_KEY,
    CHIP8_E_KEY,
    CHIP8_F_KEY,
} Chip8Key;

extern const KeyboardKey Chip8Mappings[]; // Change key values to change mappings

Chip8State Chip8Init(void);
bool Chip8ClearState(Chip8State *state); // Set all state variables to the default state
bool Chip8LoadProgram(Chip8State *state, unsigned char *const program, size_t size); // Load program from stream of byte
bool Chip8LoadFont(Chip8State *state, unsigned char *font, size_t font_size); // Provide NULL to load the default font
Chip8Res Chip8MakeCycle(Chip8State *state);
void Chip8DumpMemory(FILE *restrict file, Chip8State *const state, ...); // Print to the stream provided the content of memory in the range provided

#endif // CHIP8_H_
