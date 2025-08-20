#include "../0x_includes/glob.h"

void ensure_dump_folder(void) {
    struct stat st = {0};
    if (stat(".dumps", &st) == -1)
        mkdir(".dumps", 0700);
}

void write_to_file(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    if (!f) return;
    fputs(content, f);
    fclose(f);
}

void append_to_file(const char *path, const char *content) {
    FILE *f = fopen(path, "a");
    if (!f) return;
    fputs(content, f);
    fclose(f);
}