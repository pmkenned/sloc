#ifndef MINUNIT_H
#define MINUNIT_H

#define XSTR(X) STR(X)
#define STR(X) #X

#define MAX_ERR_MSG 1024
extern char err_msg[MAX_ERR_MSG];
extern int tests_run;

// see: https://gcc.gnu.org/onlinedocs/cpp/Variadic-Macros.html

#define mu_assert(test, msg) \
    do { \
        if (!(test)) { \
            return msg; \
        } \
    } while (0)

#define mu_assert_vaarg(test, fmt, ...) \
    do { \
        if (!(test)) { \
            snprintf(err_msg, MAX_ERR_MSG, __FILE__ ":" XSTR(__LINE__) ": " fmt, __VA_ARGS__); \
            return err_msg; \
        } \
    } while (0)

#define mu_run_test(test) _mu_run_test(test, 1)

#define mu_run_test_suite(test)  _mu_run_test(test, 0)

#define _mu_run_test(test, inc_test_count)\
    do { \
        char *message = test(); \
        tests_run += inc_test_count; \
        if (message) \
            return message; \
    } while (0)

#endif
