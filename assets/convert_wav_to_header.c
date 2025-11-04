#include <stdio.h>
#include <string.h>
#include "raylib.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <path_to_wav_file>\n", argv[0]);
        return 1;
    }

    Wave wave = LoadWave(argv[1]);

    char new_name[1024] = {0};
    strcat(new_name, argv[1]);
    strcat(new_name, ".h");

    ExportWaveAsCode(wave, new_name);
    UnloadWave(wave);
    return 0;
}
