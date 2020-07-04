#ifndef UNIT_H
#define UNIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNIT_MESSAGE_LEN 1024

static char unit_message[UNIT_MESSAGE_LEN];

static int unit_run = 0;
static int unit_assert = 0;
static int unit_fail = 0;
static int unit_status = 0;

static void (*unit_setup)() = NULL;
static void (*unit_teardown)() = NULL;

#define UNIT_TEST(test_name)        static void TEST_##test_name()
#define UNIT_TEST_SUITE(suite_name) static void SUITE_##suite_name()

#define UNIT_EXIT_CODE unit_fail

#define UNIT_RUN_SUITE(suite_name)                                             \
    do {                                                                       \
        SUITE_##suite_name();                                                  \
        unit_setup = NULL;                                                     \
        unit_teardown = NULL;                                                  \
    } while (0)

#define UNIT_SUITE_SETUP(setup_func)                                           \
    do {                                                                       \
        unit_setup = setup_func;                                               \
    } while (0)

#define UNIT_SUITE_TEARDOWN(teardown_func)                                     \
    do {                                                                       \
        unit_teardown = teardown_func;                                         \
    } while (0)

#define UNIT_RUN_TEST(test_name)                                               \
    do {                                                                       \
        if (unit_setup) {                                                      \
            unit_setup();                                                      \
        }                                                                      \
        unit_status = 0;                                                       \
        TEST_##test_name();                                                    \
        ++unit_run;                                                            \
        if (unit_status) {                                                     \
            ++unit_fail;                                                       \
            fprintf(stderr, "%s failed:\n    %s\n", #test_name, unit_message); \
            fflush(stderr);                                                    \
        }                                                                      \
        if (unit_teardown) {                                                   \
            unit_teardown();                                                   \
        }                                                                      \
    } while (0)

#define UNIT_REPORT()                                                          \
    do {                                                                       \
        printf("\n%d tests, %d assertions, %d failures\n",                     \
            unit_run,                                                          \
            unit_assert,                                                       \
            unit_fail);                                                        \
    } while (0)

#define unit_assert_true(test)                                                 \
    do {                                                                       \
        ++unit_assert;                                                         \
        if (!(test)) {                                                         \
            unit_status = 1;                                                   \
            snprintf(unit_message,                                             \
                UNIT_MESSAGE_LEN,                                              \
                "%s:%d '%s' was not true\n",                                   \
                __FILE__,                                                      \
                __LINE__,                                                      \
                #test);                                                        \
        }                                                                      \
    } while (0)

#define unit_assert_false(test)                                                \
    do {                                                                       \
        ++unit_assert;                                                         \
        if ((test)) {                                                          \
            unit_status = 1;                                                   \
            snprintf(unit_message,                                             \
                UNIT_MESSAGE_LEN,                                              \
                "%s:%d '%s' was not false\n",                                  \
                __FILE__,                                                      \
                __LINE__,                                                      \
                #test);                                                        \
        }                                                                      \
    } while (0)

#define unit_assert_int_eq(expected, result)                                   \
    do {                                                                       \
        ++unit_assert;                                                         \
        int tmp_result = (int)(result);                                        \
        int tmp_expected = (int)(expected);                                    \
        fflush(stdout);                                                        \
        if (tmp_expected != tmp_result) {                                      \
            unit_status = 1;                                                   \
            snprintf(unit_message,                                             \
                UNIT_MESSAGE_LEN,                                              \
                "%s:%d %d expected but was %d\n",                              \
                __FILE__,                                                      \
                __LINE__,                                                      \
                tmp_expected,                                                  \
                tmp_result);                                                   \
        }                                                                      \
    } while (0)

#define unit_assert_hex_eq(expected, result)                                   \
    do {                                                                       \
        ++unit_assert;                                                         \
        int tmp_result = (int)(result);                                        \
        int tmp_expected = (int)(expected);                                    \
        fflush(stdout);                                                        \
        if (tmp_expected != tmp_result) {                                      \
            unit_status = 1;                                                   \
            snprintf(unit_message,                                             \
                UNIT_MESSAGE_LEN,                                              \
                "%s:%d %04X expected but was %04X\n",                          \
                __FILE__,                                                      \
                __LINE__,                                                      \
                tmp_expected,                                                  \
                tmp_result);                                                   \
        }                                                                      \
    } while (0)

#define unit_assert_double_eq(expected, result)                                \
    do {                                                                       \
        ++unit_assert;                                                         \
        double tmp_result = (double)(result);                                  \
        double tmp_expected = (double)(expected);                              \
        fflush(stdout);                                                        \
        if (tmp_expected != tmp_result) {                                      \
            unit_status = 1;                                                   \
            snprintf(unit_message,                                             \
                UNIT_MESSAGE_LEN,                                              \
                "%s:%d %f expected but was %f\n",                              \
                __FILE__,                                                      \
                __LINE__,                                                      \
                tmp_expected,                                                  \
                tmp_result);                                                   \
        }                                                                      \
    } while (0)

#define unit_assert_string_eq(expected, result)                                \
    do {                                                                       \
        ++unit_assert;                                                         \
        const char * tmp_result = (const char *)(result);                      \
        const char * tmp_expected = (const char *)(expected);                  \
        fflush(stdout);                                                        \
        if (strcmp(tmp_expected, tmp_result)) {                                \
            unit_status = 1;                                                   \
            if (!tmp_expected) {                                               \
                tmp_expected = "<null>";                                       \
            }                                                                  \
            if (!tmp_result) {                                                 \
                tmp_result = "<null>";                                         \
            }                                                                  \
            snprintf(unit_message,                                             \
                UNIT_MESSAGE_LEN,                                              \
                "%s:%d %s expected but was %s\n",                              \
                __FILE__,                                                      \
                __LINE__,                                                      \
                tmp_expected,                                                  \
                tmp_result);                                                   \
        }                                                                      \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif // UNIT_H
