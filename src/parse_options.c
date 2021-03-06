#include "parse_options.h"

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define MAX(a,b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
       _a > _b ? _a : _b; })

#define NELEM(X) (sizeof(X)/sizeof(X[0]))

int
parse_options(int argc, char * argv[], option_t * options, size_t num_options, char *** non_option_args, size_t * num_non_option_args)
{
    size_t i, j;
    char * opt;
    const char * program_name = argv[0];

    *num_non_option_args = 0;
    *non_option_args = malloc(sizeof(**non_option_args)*argc);

    /* initialize flags to 0 */
    for (i = 0; i < num_options; i++) {
        assert(options[i].optarg != NULL);
        assert((options[i].has_arg == ARG_NONE) == (options[i].arg_type == ARG_TYPE_FLAG));

        if (options[i].arg_type == ARG_TYPE_FLAG)
            *((int *) options[i].optarg) = 0;
    }

    for (i = 1; i < (size_t) argc; i++) {
        char * curr_arg = argv[i];
        int is_short = 0, is_long = 0, is_single = 0;
        if ((curr_arg[0] == '-') && (curr_arg[1] != '\0')) {
            is_long = (curr_arg[1] == '-') ? 1 : 0;
            is_short = !is_long;
            is_single = (curr_arg[2] == '\0') ? 1 : 0;
        }

        if(is_short)     opt = curr_arg+1;
        else if(is_long) opt = curr_arg+2;

        if (!is_short && !is_long) {
            (*non_option_args)[*num_non_option_args] = curr_arg;
            *num_non_option_args += 1;
            continue;
        }

        do {
            int no_option_arg_present = (i == (size_t) argc-1) || (argv[i+1][0] == '-');
            /* find which option matches */
            for (j = 0; j < num_options; j++) {
                if ((is_long && (strcmp(opt, options[j].long_opt) == 0)) || 
                    (is_short && (opt[0] == options[j].short_opt)))
                    break;
            }
            if (j == num_options) {
                if (is_long)  fprintf(stderr, "%s: unrecognized option '--%s'\n", program_name, opt);
                if (is_short) fprintf(stderr, "%s: unrecognized option '-%c'\n", program_name, opt[0]);
                return -1;
            }

            if (options[j].arg_type == ARG_TYPE_FLAG)
                *((int *) options[j].optarg) = 1;

            if (options[j].has_arg == ARG_REQUIRED) {
                if (no_option_arg_present) {
                    if (is_long)  fprintf(stderr, "%s: option '--%s' has mandatory argument\n", program_name, opt);
                    if (is_short)  fprintf(stderr, "%s: option '-%c' has mandatory argument\n", program_name, opt[0]);
                    return -1;
                }
            } else if (options[j].has_arg == ARG_OPTIONAL) {
                if (no_option_arg_present)
                    continue;
            }
            if (options[j].has_arg != ARG_NONE) {
                if (is_short && !is_single) {
                    fprintf(stderr, "%s: option '-%c' cannot be combined with other options\n", program_name, opt[0]);
                    return -1;
                }
                if (options[j].arg_type == ARG_TYPE_STR)      *((char **) options[j].optarg) = argv[i+1];
                else if (options[j].arg_type == ARG_TYPE_INT) *((int *) options[j].optarg) = atoi(argv[i+1]);
                i++; /* skip next argument (used as option argument) */
            }
        } while (is_short && ((++opt)[0] != '\0'));
    }
    return 0;
}

char *
gen_options_str(option_t * options, size_t num_options)
{
    size_t max_long_opt_len = 0;
    size_t i, j;
    size_t cap = 1; /* \0 */
    char * s, * s_end;
    if (num_options == 0)
        return NULL;
    for (i = 0; i < num_options; i++)
        max_long_opt_len = MAX(strlen(options[i].long_opt), max_long_opt_len);
    size_t desc_offset = max_long_opt_len + 12;
    for (i = 0; i < num_options; i++)
        cap += desc_offset + strlen(options[i].description) + 1;
    s = malloc(sizeof(*s)*cap);
    s_end = s;
    for (i = 0; i < num_options; i++) {
        int n;
        const char sdash = (options[i].short_opt != ' ') ? '-' : ' ';
        const char * ldash = (options[i].long_opt[0] != '\0') ? "--" : "  ";
        const char * arg = "";
        if (options[i].has_arg == ARG_REQUIRED)         arg = "ARG";
        else if (options[i].has_arg == ARG_OPTIONAL)    arg = "[ARG]";
        n = sprintf(s_end, "  %c%c %s%s %s", sdash, options[i].short_opt, ldash, options[i].long_opt, arg);
        s_end += n;
        /* align description */
        for (j = 0; j < (desc_offset - n); j++)
            (s_end++)[0] = ' ';
        s_end[0] = '\0';
        n = sprintf(s_end, "%s\n", options[i].description);
        s_end += n;
    }
    return s;
}

#ifdef TEST

#include "minunit.h"

static char *
test_parse_options__with_an_empty_options_list_any_arg_beginning_with_dash_is_rejected()
{
    option_t * options = NULL;
    char ** non_option_args;
    size_t num_non_option_args;

    char * argv[] = {"program", "-foo", NULL};
    size_t argc = NELEM(argv) - 1;

    mu_assert(-1 == parse_options(argc, argv, options, 0, &non_option_args, &num_non_option_args), "with an empty options list, any arg beginning with dash is rejected");

    return NULL;
}

static char *
test_parse_options__with_an_empty_options_list_non_option_args_are_returned()
{
    size_t i;
    option_t * options = NULL;
    char ** non_option_args;
    size_t num_non_option_args;

    char * argv[] = {"program", "foo", "bar", NULL};
    size_t argc = NELEM(argv) - 1;

    parse_options(argc, argv, options, 0, &non_option_args, &num_non_option_args);

    mu_assert(num_non_option_args == argc-1, "with an empty options list, number of non-option arguments is argc-1");

    for (i = 1; i < argc-1; i++)
        mu_assert(strcmp(non_option_args[i], argv[i+1]) == 0, "with an empty options list, non-option args are returned");

    return NULL;
}

static char *
test_gen_options_str__with_no_options_str_is_null()
{
    mu_assert(gen_options_str(NULL, 0) == NULL, "with no options, string is null ptr");
    return NULL;
}

char *
test_parse_options()
{
    // parse_options
    mu_run_test(test_parse_options__with_an_empty_options_list_any_arg_beginning_with_dash_is_rejected);
    mu_run_test(test_parse_options__with_an_empty_options_list_non_option_args_are_returned);

    // gen_options_str
    mu_run_test(test_gen_options_str__with_no_options_str_is_null);

    return NULL;
}

#endif
