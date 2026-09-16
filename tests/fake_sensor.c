/* Protocol/lifecycle fixture. Never links to a sensor library or fan driver. */
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
    HANDLE stop;
    char scenario[32];
    if (argc != 3) return 1;
    stop = (HANDLE)(ULONG_PTR)_strtoui64(argv[1], NULL, 10);
    GetEnvironmentVariableA("LFC_TEST_SCENARIO", scenario, sizeof(scenario));
    if (!strcmp(scenario, "exit")) return 0;
    if (!strcmp(scenario, "valid")) printf("71.5 63.0\n");
    if (!strcmp(scenario, "cpu-only")) printf("61.5 -1\n");
    if (!strcmp(scenario, "invalid")) printf("unavailable\n");
    if (!strcmp(scenario, "nan")) printf("nan 50\n");
    if (!strcmp(scenario, "extra")) printf("40 40 trailing\n");
    if (!strcmp(scenario, "overflow")) {
        for (int i = 0; i < 64; ++i) putchar('x');
        printf("40 40\n");
    }
    if (!strcmp(scenario, "fragmented")) {
        printf("71."); fflush(stdout); Sleep(100); printf("5 63.0\n");
    }
    fflush(stdout);
    WaitForSingleObject(stop, 30000);
    return 0;
}
