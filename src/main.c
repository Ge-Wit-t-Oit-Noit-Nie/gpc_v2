#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "clogger.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        clog_error(__FUNCTION__, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *filename = argv[1];
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        clog_error(__FUNCTION__, "Error operning file \"%s\" (%d)\n", filename, errno);
        return EXIT_FAILURE;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        printf("%s", buffer);
    }

    if (fclose(file) != 0) {
        clog_error(__FUNCTION__, "Error operning file \"%s\" (%d)\n", filename, errno);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}   