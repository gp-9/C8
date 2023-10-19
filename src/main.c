#include <math.h>
#include <stdio.h>

#include <raylib.h>
///#define RAYGUI_IMPLEMENTATION
///#include <raygui.h>

#define ORIGINAL_CHIP8
#include "chip8/chip8.h"

#define PIXEL_SIZE 10

#define SCREEN_WIDTH (PIXEL_SIZE * CHIP8_SCREEN_WIDTH)
#define SCREEN_HEIGHT (PIXEL_SIZE * CHIP8_SCREEN_HEIGHT)

//#define DEBUG

#define FPS 60

// Instruction per second
#define IPS 700
// Instruction per frame
#define IPF (IPS/FPS)

void draw_screen_buffer(Chip8State *const state) {
    for (int i = 0; i < CHIP8_SCREEN_HEIGHT; ++i) {
        for (int j = 0; j < CHIP8_SCREEN_WIDTH; ++j) {
            if (state->screen[i][j]) {
                DrawRectangle(j * PIXEL_SIZE, i * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE, RAYWHITE);
            }
        }
    }
}

void draw_grid() {
    for (int i = 0; i < GetRenderWidth(); i += PIXEL_SIZE) {
        DrawLine(i, 0, i, GetRenderHeight(), BLUE);
    }

    for (int i = 0; i < GetRenderHeight(); i += PIXEL_SIZE) {
        DrawLine(0, i, GetRenderWidth(), i, BLUE);
    }
}

void handle_input(Chip8State *state) {
    if (IsKeyPressed(KEY_ESCAPE)) {
        WindowShouldClose();
    }

    if (IsKeyPressed(KEY_SPACE)) {
        state->halt = !state->halt;
    }
}

void StateStatus(Chip8State const *const state) {
    printf("Registers: ");
    for (int i = 0; i < REGISTERS; ++i) {
        printf("%d ", state->registers[i]);
    }
    printf("\n");
    printf("Delay Timer: %d\n", state->delay_timer);
    printf("Sound Timer: %d\n", state->sound_timer);
    printf("Index Register: %d\n", state->ir);
    printf("Program Counter: %d\n", state->pc);
}

static const char font_path[] = "./assets/fonts/slkscr.ttf";

int main(void) {
#ifdef DEBUG
    SetTraceLogLevel(LOG_ERROR | LOG_WARNING);
#else
    SetTraceLogLevel(LOG_NONE);
#endif

    Chip8State state = Chip8Init();
    Chip8LoadFont(&state, NULL, 0);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Chip-8 Emulator");

    SetTargetFPS(FPS);
    
    Font font = LoadFont(font_path);
    const int font_size = 30;
    const int font_spacing = 0;
    char error_message[] = "Unsupported file type";
    char start_message[] = "Drag and Drop ROM file to start";
    Color message_color = LIGHTGRAY;
    char *message = start_message;

    Vector2 text_pos = {0};
    Vector2 text_dim = MeasureTextEx(font, message, font_size, font_spacing);
    text_pos.x = SCREEN_WIDTH/2.0f - text_dim.x/2.0f;
    text_pos.y = SCREEN_HEIGHT/2.0f - text_dim.y/2.0f;

    int instructions = 0;

    while (!WindowShouldClose()) {
        if (IsFileDropped()) {
            FilePathList list = LoadDroppedFiles();

            if (IsFileExtension(list.paths[0], ".ch8")) {
                Chip8ClearState(&state);
                unsigned int byte_read = 0;
                unsigned char *data = LoadFileData(list.paths[0], &byte_read);
                Chip8LoadProgram(&state, data, byte_read);
                UnloadFileData(data);
            } else {
                message = error_message;
                message_color = RED;
                Vector2 new_pos = MeasureTextEx(font, message, font_size, font_spacing);
                text_pos.x = SCREEN_WIDTH/2.0f - new_pos.x/2.0f;
                text_pos.y = SCREEN_HEIGHT/2.0f - new_pos.y/2.0f;
            }

            UnloadDroppedFiles(list);
        }

        BeginDrawing();
            handle_input(&state);
            while (!state.halt && instructions < IPF) {
                PollInputEvents();
                Chip8MakeCycle(&state);
                //StateStatus(&state);
                instructions++;
            }

            if (state.delay_timer > 0) {
                state.delay_timer--;
            }
            if (state.sound_timer > 0) {
                state.sound_timer--;
            }

            if (instructions >= IPF) {
                instructions = 0;
            }

            ClearBackground(BLACK);
            if (!state.halt) {
                draw_screen_buffer(&state);
            } else {
                DrawTextEx(font, message, text_pos, font_size, font_spacing, message_color);
            }
        EndDrawing();
    }

    UnloadFont(font);
    CloseWindow();

    return 0;

}
