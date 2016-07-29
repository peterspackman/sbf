#pragma once
/*
 * Essentially the same as MinUnit tests
 */
#define assert(message, test)                                                  \
    do {                                                                       \
        if (!(test))                                                           \
            return message;                                                    \
    } while (0)
#define run_unit_test(test)                                                    \
    do {                                                                       \
        char *message = test();                                                \
        tests_run++;                                                           \
        if (message)                                                           \
            return message;                                                    \
    } while (0)
extern int tests_run;
