#ifndef PARSE_OPTIONS_H
#define PARSE_OPTIONS_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    ARG_NONE = 0,
    ARG_OPTIONAL,
    ARG_REQUIRED
};

typedef enum {
    ARG_TYPE_STR,
    ARG_TYPE_INT,
    ARG_TYPE_FLAG
} arg_type_t;

typedef struct {
    const char *    long_opt;
    char            short_opt;
    const char *    description;
    int             has_arg;
    arg_type_t      arg_type;
    void *          optarg;
} option_t;

int     parse_options(int argc, char * argv[], option_t * options, size_t num_options, char *** non_option_args, size_t * num_non_option_args);
char *  gen_options_str(option_t * options, size_t num_options);

#ifdef TEST
int test_parse_options();
#endif

#ifdef __cplusplus
}
#endif

#endif /* PARSE_OPTIONS_H */
