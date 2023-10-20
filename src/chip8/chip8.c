#include <string.h>

#include "chip8/chip8.h"

const uint8_t Chip8DefaultFont[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

const KeyboardKey Chip8Mappings[] = { 
    [CHIP8_0_KEY] = KEY_X,
    [CHIP8_1_KEY] = KEY_ONE,
    [CHIP8_2_KEY] = KEY_TWO,
    [CHIP8_3_KEY] = KEY_THREE,
    [CHIP8_4_KEY] = KEY_Q,
    [CHIP8_5_KEY] = KEY_W,
    [CHIP8_6_KEY] = KEY_E,
    [CHIP8_7_KEY] = KEY_A,
    [CHIP8_8_KEY] = KEY_S,
    [CHIP8_9_KEY] = KEY_D,
    [CHIP8_A_KEY] = KEY_Z,
    [CHIP8_B_KEY] = KEY_C,
    [CHIP8_C_KEY] = KEY_FOUR,
    [CHIP8_D_KEY] = KEY_R,
    [CHIP8_E_KEY] = KEY_F,
    [CHIP8_F_KEY] = KEY_V,
};

typedef Chip8Res (*chip8_decode_function)(Chip8State *, Chip8Inst);

static inline Chip8Res decode_zero(Chip8State *state, Chip8Inst instruction) {
    if (instruction == 0x00E0) {
        for (size_t i = 0; i < CHIP8_SCREEN_HEIGHT; ++i) {
            for (size_t j = 0; j < CHIP8_SCREEN_WIDTH; ++j) {
                state->screen[i][j] = false;
            }
        }
        return CHIP8_SUCCESS;
    }
    
    if (instruction == 0x00EE) {
        if (state->sp > 0) {
            state->pc = state->stack[state->sp--]; 
            return CHIP8_SUCCESS;
        }
    }

    return CHIP8_ERROR;
}

static inline Chip8Res decode_one(Chip8State *state, Chip8Inst instruction) {
    uint16_t addr = instruction & 0x0fff;
    state->pc = addr;
    return CHIP8_SUCCESS;
}

static inline Chip8Res decode_two(Chip8State *state, Chip8Inst instruction) {
    if (state->sp < MAX_STACK - 1) {
        uint16_t addr = instruction & 0x0fff;
        state->stack[++(state->sp)] = state->pc;
        state->pc = addr;
        return CHIP8_SUCCESS;
    }
    return CHIP8_ERROR;
}

static inline Chip8Res decode_three(Chip8State *state, Chip8Inst instruction) {
    uint8_t reg = (instruction & 0x0f00) >> 8;
    uint8_t value = instruction & 0x00ff;
    if (state->registers[reg] == value) {
        state->pc += 2;
    }
    return CHIP8_SUCCESS;
}

static inline Chip8Res decode_four(Chip8State *state, Chip8Inst instruction) {
    uint8_t reg = (instruction & 0x0f00) >> 8;
    uint8_t value = instruction & 0x00ff;
    if (state->registers[reg] != value) {
        state->pc += 2;
    }
    return CHIP8_SUCCESS;
}

static inline Chip8Res decode_five(Chip8State *state, Chip8Inst instruction) {
    uint8_t last_nibble = instruction & 0x000f;

    if (last_nibble == 0) {
        uint8_t reg1 = (instruction & 0x0f00) >> 8;
        uint8_t reg2 = (instruction & 0x00f0) >> 4;
        if (state->registers[reg1] == state->registers[reg2]) {
            state->pc += 2;
        }
        return CHIP8_SUCCESS;
    }

    return CHIP8_ERROR;
}

static inline Chip8Res decode_six(Chip8State *state, Chip8Inst instruction) {
    uint8_t reg = (instruction & 0x0f00) >> 8;
    uint8_t val = instruction & 0x00ff;
    state->registers[reg] = val;
    return CHIP8_SUCCESS;
}

static inline Chip8Res decode_seven(Chip8State *state, Chip8Inst instruction) {
    uint8_t reg = (instruction & 0x0f00) >> 8;
    uint8_t val = instruction & 0x00ff;
    state->registers[reg] += val;
    return CHIP8_SUCCESS;
}

static inline Chip8Res decode_eight(Chip8State *state, Chip8Inst instruction) {
    uint8_t last_nibble = instruction & 0x000f;
    uint8_t reg1 = (instruction & 0x0f00) >> 8;
    uint8_t reg2 = (instruction & 0x00f0) >> 4;
    uint8_t *val1p = &state->registers[reg1];
    uint8_t *val2p = &state->registers[reg2];

    Chip8Res return_val;
    switch (last_nibble) {
        case 0: {
                *val1p = *val2p;
                return_val = CHIP8_SUCCESS;
            } break;
        case 1: {
                *val1p |= *val2p;
                return_val = CHIP8_SUCCESS;
            } break;
        case 2: {
                *val1p &= *val2p;
                return_val = CHIP8_SUCCESS;
            } break;
        case 3: {
                *val1p ^= *val2p;
                return_val = CHIP8_SUCCESS;
            } break;
        case 4: {
                uint8_t test = *val1p + *val2p;
                if (test < *val1p || test < *val2p) {
                    state->registers[0xF] = 1;
                }
                *val1p += *val2p;
                return_val = CHIP8_SUCCESS;
            } break;
        case 5: {
                if (*val1p > *val2p) state->registers[0xF] = 1;
                if (*val2p > *val1p) state->registers[0xF] = 0;
                *val1p -= *val2p;
                return_val = CHIP8_SUCCESS;
            } break;
        case 6: {
#ifdef ORIGINAL_CHIP8
                *val1p = *val2p;
#endif
                state->registers[0xF] = *val1p & 0x01;
                *val1p >>= 1;
                return_val = CHIP8_SUCCESS;
            } break;
        case 7: {
                if (*val1p > *val2p) state->registers[0xF] = 1;
                if (*val2p > *val1p) state->registers[0xF] = 0;
                *val1p = *val2p - *val1p;
                return_val = CHIP8_SUCCESS;
            } break;
        case 0xE: {
#ifdef ORIGINAL_CHIP8
                *val1p = *val2p;
#endif
                state->registers[0xF] = *val1p & 0x80;
                *val1p <<= 1;
                return_val = CHIP8_SUCCESS;
            } break;
        default:
            return_val = CHIP8_ERROR;
    }

    return return_val;
}

static inline Chip8Res decode_nine(Chip8State *state, Chip8Inst instruction) {
    uint8_t last_nibble = instruction & 0x000f;

    if (last_nibble == 0) {
        uint8_t reg1 = (instruction & 0x0f00) >> 8;
        uint8_t reg2 = (instruction & 0x00f0) >> 4;
        if (state->registers[reg1] != state->registers[reg2]) {
            state->pc += 2;
        }
        return CHIP8_SUCCESS;
    }

    return CHIP8_ERROR;
}

static inline Chip8Res decode_A(Chip8State *state, Chip8Inst instruction) {
    uint16_t addr = instruction & 0x0fff;
    state->ir = addr;
    return CHIP8_SUCCESS;
}

static inline Chip8Res decode_B(Chip8State *state, Chip8Inst instruction) {
    uint16_t addr = instruction & 0x0fff;

#ifdef ORIGINAL_CHIP8
        state->pc = addr + state->registers[0];
#else
        uint8_t reg = (instruction & 0x0f00) >> 8;
        state->pc = addr + state->registers[reg];
#endif
    return CHIP8_SUCCESS;
}

static inline Chip8Res decode_C(Chip8State *state, Chip8Inst instruction) {
    uint8_t reg = (instruction & 0x0f00) >> 8;
    uint8_t val = instruction & 0x00ff;
    state->registers[reg] = val & GetRandomValue(0, UINT8_MAX);
    return CHIP8_SUCCESS;
}

static inline Chip8Res decode_D(Chip8State *state, Chip8Inst instruction) {
    uint8_t size = instruction & 0x000f;
    uint8_t x_reg = (instruction & 0x0f00) >> 8;
    uint8_t y_reg = (instruction & 0x00f0) >> 4;
    uint8_t x_pos = state->registers[x_reg] % CHIP8_SCREEN_WIDTH;
    uint8_t y_pos = state->registers[y_reg] % CHIP8_SCREEN_HEIGHT;
    state->registers[0xF] = 0;

    for (int i = 0; i < size; ++i) {
        if ((y_pos + i) >= CHIP8_SCREEN_HEIGHT) break;
        uint8_t sprite_data = state->memory[state->ir + i];
        for (int j = 0; j < 8; ++j) {
            if ((x_pos + j) >= CHIP8_SCREEN_WIDTH) break;
            bool curr_pixel = sprite_data & (0x80 >> j);
            bool *pixel = &(state->screen[y_pos + i][x_pos + j]);
            if (curr_pixel && *pixel) {
                *pixel = false;
                state->registers[0xF] = 1;
            } else if (curr_pixel && !*pixel) {
                *pixel = true;
            }
        }
    }

    return CHIP8_SUCCESS;
}

static inline Chip8Res decode_E(Chip8State *state, Chip8Inst instruction) {
    uint8_t last_byte = instruction & 0x00ff;
    uint8_t reg = (instruction & 0x0f00) >> 8;
    uint8_t *val = &state->registers[reg];

    if (last_byte == 0x9E) {
        if (IsKeyDown(Chip8Mappings[*val])) state->pc += 2;
        return CHIP8_SUCCESS;
    } else if (last_byte == 0xA1) {
        if (IsKeyUp(Chip8Mappings[*val])) state->pc += 2;
        return CHIP8_SUCCESS;
    }

    return CHIP8_ERROR;
}

static inline Chip8Res decode_F(Chip8State *state, Chip8Inst instruction) {
    uint8_t last_byte = instruction & 0x00ff;
    uint8_t reg = (instruction & 0x0f00) >> 8;
    uint8_t *val = &state->registers[reg];
    Chip8Res returned;

    switch (last_byte) {
        case 0x07: {
                *val = state->delay_timer;
                returned = CHIP8_SUCCESS;
            } break;

        case 0x15: {
                state->delay_timer = *val;
                returned = CHIP8_SUCCESS;
            } break;
            
        case 0x18: {
                state->sound_timer = *val;
                returned = CHIP8_SUCCESS;
            } break;

        case 0x1E: {
                state->ir += *val;
                if (state->ir > 0x1000) state->registers[0xF] = 1;
                returned = CHIP8_SUCCESS;
            } break;

        case 0x0A: {
                KeyboardKey key = GetKeyPressed();
                if (key != 0) {
                    for (int i = 0; i < 0xF; ++i) {
                        if (key == Chip8Mappings[i]) {
                            *val = i;
                        }
                    }
                } else {
                    state->pc -= 2;
                }

                returned = CHIP8_SUCCESS;
            } break;

        case 0x29: {
                uint8_t character = *val & 0x0f;
                state->ir = DEFAULT_FONT_ADDR + character * 5;

                returned = CHIP8_SUCCESS;
            } break;

        case 0x33: {
                int n1 = *val % 10;
                int n2 = (*val / 10) % 10;
                int n3 = (*val / 100) % 10;
                state->memory[state->ir] = n3;
                state->memory[state->ir + 1] = n2;
                state->memory[state->ir + 2] = n1;
                returned = CHIP8_SUCCESS;
            } break;

        case 0x55: {
                for (int i = 0; i <= reg; ++i) {
                    state->memory[state->ir + i] = state->registers[i];
                }
                returned = CHIP8_SUCCESS;
            } break;

        case 0x65: {
                for (int i = 0; i <= reg; ++i) {
                    state->registers[i] = state->memory[state->ir + i];
                }
                returned = CHIP8_SUCCESS;
            } break;

        default:
            returned = CHIP8_ERROR;
            break;
    }

    return returned;
}

static const chip8_decode_function decode_functions[] = {
    decode_zero,
    decode_one,
    decode_two,
    decode_three,
    decode_four,
    decode_five,
    decode_six,
    decode_seven,
    decode_eight,
    decode_nine,
    decode_A,
    decode_B,
    decode_C,
    decode_D,
    decode_E,
    decode_F,
};

static inline bool fetch_next_instruction(Chip8State *state, Chip8Inst *instruction) {
    if (!state || !instruction) return false;

    *instruction = ((uint16_t)state->memory[state->pc] << 8) | ((uint16_t)state->memory[state->pc + 1]);
    if (state->pc < MAX_MEM - 2) {
        state->pc += 2;
    }

    return true;
}


static inline Chip8Res decode_next_instruction(Chip8State *state, Chip8Inst instruction) {
    if (!state) return CHIP8_ERROR;
    uint8_t instruction_type = (instruction & 0xf000) >> 12;
    return decode_functions[instruction_type](state, instruction);
}

Chip8State Chip8Init(void) {
    Chip8State state = {
        .memory = {0},
        .registers = {0},
        .pc = 0x200,
        .ir = 0,
        .stack = {0},
        .sp = 0,
        .screen = {{false}},
        .delay_timer = 0,
        .sound_timer = 0,
        .halt = true,
    };

    return state;
}

bool Chip8ClearState(Chip8State *state) {
    if (!state) return false;
    state->pc = 0x200;
    state->ir = 0;
    state->sp = 0;
    state->delay_timer = 0;
    state->sound_timer = 0;
    state->halt = true;

    for (size_t i = state->pc; i < MAX_MEM; ++i) {
        state->memory[i] = 0;
    }

    for (size_t i = 0; i < MAX_STACK; ++i) {
        state->stack[i] = 0;
    }

    for (size_t i = 0; i < REGISTERS; ++i) {
        state->registers[i] = 0;
    }

    for (size_t i = 0; i < CHIP8_SCREEN_HEIGHT; ++i) {
        for (size_t j = 0; j < CHIP8_SCREEN_WIDTH; ++j) {
            state->screen[i][j] = false;
        }
    }

    return true;
}

bool Chip8LoadProgram(Chip8State *state, unsigned char *const program, size_t size) {
    if (!state || size > AVL_MEM) return false;

    for (size_t i = 0; i < size; ++i) {
        state->memory[state->pc + i] = (uint8_t)program[i];
    }
    return true;
}

bool Chip8LoadFont(Chip8State *state, unsigned char *font, size_t font_size) {
    if (!state) return false;

    if (!font) {
        memcpy(&state->memory[DEFAULT_FONT_ADDR], Chip8DefaultFont, sizeof(Chip8DefaultFont));
    } else {
        memcpy(&state->memory[DEFAULT_FONT_ADDR], font, font_size);
    }
    return true;
}

Chip8Res Chip8MakeCycle(Chip8State *state) {
    Chip8Inst next_instruction = 0;
    if (!fetch_next_instruction(state, &next_instruction)) return CHIP8_ERROR;
    return decode_next_instruction(state, next_instruction);
}

void Chip8DumpMemory(FILE *restrict file, Chip8State *const state, ...) {
    size_t start = 0;
    size_t end = MAX_MEM;

    va_list args;
    va_start(args, state);
    size_t new_start = va_arg(args, size_t);
    if (new_start > start) start = new_start;
    size_t new_end = va_arg(args, size_t);
    if (new_end < end) end = new_end;
    va_end(args);

    char ascii_vals[16] = {0};
    for (size_t i = start; i < end; i += 16) {
        fprintf(file, "0x%012zX   ", i);
        for (size_t j = 0; j < 16; ++j) {
            uint8_t val = state->memory[i + j];
            fprintf(file, "%02x ", val);
            if (j == 7) fprintf(file, " ");
            ascii_vals[j] = (val < 0x20 || val > 0x7e) ? '.' : val;
        }
        fprintf(file, "  |%s|\n", ascii_vals);
    }
}

