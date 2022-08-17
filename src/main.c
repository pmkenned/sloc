#define _POSIX_C_SOURCE 1
#include "ansi_esc.h"
#include "parse_options.h"
#include "lex.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define NELEM(X) (sizeof(X)/sizeof(X[0]))

static int help_flag = 0;
static int version_flag = 0;
static int total_flag = 0;
static int blank_flag= 0;
static int comment_flag= 0;
static int source_flag= 0;

static char version_str[] = "0.1";
static char * program_name;
static char * options_str;

typedef struct {
    char * p;
    size_t n;
} buffer_t;

typedef struct {
    size_t num;
    size_t cap;
    char ** strs;
} str_arr_t;

void * xmalloc(size_t sz)
{
    void * p = malloc(sz);
    if (p == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    return p;
}

void * xrealloc(void * p, size_t sz)
{
    void * new_p = realloc(p, sz);
    if (new_p == NULL) {
        free(p);
        perror("realloc");
        exit(EXIT_FAILURE);
    }
    return new_p;
}

static int
strnc(const char * s, int c)
{
    int n = 0;
    if (!s) return 0;
    while (*s)
        if (*s++ == c)
            n++;
    return n;
}

#if 1
static void
remove_comments(char * buffer)
{
    /* strip comments */
    size_t i;
    int in_string_literal = 0;
    int in_c_comment = 0;
    int in_cpp_comment = 0;
    char prev_char = '\0';
    if (!buffer) return;
    size_t n = strlen(buffer);
    if (n < 1) return;
    for (i = 0; i < n-1; i++) {
        if (in_cpp_comment) {
            if (buffer[i] == '\n') {
                in_cpp_comment = 0;
            } else {
                buffer[i] = ' ';
            }
        } else if (in_c_comment) {
            if (buffer[i] == '*' && buffer[i+1] == '/' && prev_char != '/') {
                in_c_comment = 0;
                buffer[i] = ' ';
                buffer[i+1] = ' ';
            }
            if (buffer[i] != '\n')
                buffer[i] = ' ';
        } else if (in_string_literal) {
            if (buffer[i] == '"' && prev_char != '\\' && prev_char != '\'') {
                in_string_literal = 0;
            }
        } else {
            if (buffer[i] == '"') {
                in_string_literal = 1;
            } else if (buffer[i] == '/' && buffer[i+1] == '/') {
                in_cpp_comment = 1;
                buffer[i] = ' ';
                buffer[i+1] = ' ';
            } else if (buffer[i] == '/' && buffer[i+1] == '*') {
                in_c_comment = 1;
                buffer[i] = ' ';
                buffer[i+1] = ' ';
            }
        }
        prev_char = buffer[i];
    }
}
#endif

#if 0
static void
remove_comments(char * buffer)
{
    lex_reset();
    int c;
    if (!buffer) return;
    while ((c = *buffer)) {
        if (lex_next(c) == OUT_DELETE)
            if (*(buffer-1) != '\n' && *(buffer-1) != '\r')
                *(buffer-1) = ' ';
        buffer++;
    }
}
#endif

static str_arr_t
split_into_lines(char * s)
{
    str_arr_t sa;
    sa.num = 0;
    sa.cap = strnc(s, '\n')+1; // +1 in case file has no newlines
    sa.strs = xmalloc(sizeof(*sa.strs)*sa.cap);

    char * tok;
    while ((tok = strtok(s, "\r\n")) != NULL) {
        s = NULL;
        sa.strs[sa.num++] = tok;
        if (sa.num >= sa.cap) {
            sa.cap *= 2;
            sa.strs = xrealloc(sa.strs, sizeof(*sa.strs)*sa.cap);
        }
    }
    return sa;
}

static buffer_t
read_file(const char * filename)
{
    buffer_t buffer;
    FILE * fp = fopen(filename, "r");
    if (fp == NULL) {
        perror(filename);
        exit(EXIT_FAILURE);
    }
    int fd = fileno(fp);
    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror(filename);
        exit(EXIT_FAILURE);
    }
    buffer.n = (size_t) (sb.st_size+1);
    buffer.p = malloc(sizeof(*buffer.p)*buffer.n);
    if (fread(buffer.p, 1, buffer.n, fp) < buffer.n) {
        if (ferror(fp)) {
            perror(filename);
            exit(EXIT_FAILURE);
        }
    }
    assert(feof(fp));
    fclose(fp);
    buffer.p[buffer.n-1] = '\0';
    return buffer;
}

static void
usage(int exit_code)
{
    fprintf(stderr, "Usage: %s [OPTION]... FILE...\n", program_name);
    fprintf(stderr, "Prints number of lines, blanks lines, comments, and source lines of specified file\n");
    fprintf(stderr, "Prints totals if multiple files are specified \n");
    fprintf(stderr, "%s", options_str);
    exit(exit_code);
}

int
#ifdef TEST
test_main
#else
main
#endif
(int argc, char * argv[])
{
    program_name = argv[0];

    size_t i;
    char ** non_option_args;
    size_t num_non_option_args;
    option_t options[] = {
        {"help",    'h',  "show this help",             ARG_NONE, ARG_TYPE_FLAG, &help_flag     },
        {"version", ' ',  "version number",             ARG_NONE, ARG_TYPE_FLAG, &version_flag  },
        {"total",   't',  "total number of lines",      ARG_NONE, ARG_TYPE_FLAG, &total_flag    },
        {"blank",   'b',  "number of blank lines",      ARG_NONE, ARG_TYPE_FLAG, &blank_flag    },
        {"comment", 'c',  "number of comment lines",    ARG_NONE, ARG_TYPE_FLAG, &comment_flag  },
        {"source",  's',  "number of source lines",     ARG_NONE, ARG_TYPE_FLAG, &source_flag   },
    };
    const size_t num_options = NELEM(options);

    options_str = gen_options_str(options, num_options);

    if (parse_options(argc, argv, options, num_options, &non_option_args, &num_non_option_args))
        usage(EXIT_FAILURE);

    if (help_flag)
        usage(EXIT_SUCCESS);

    if (version_flag) {
        printf("version v%s\n", version_str);
        exit(EXIT_SUCCESS);
    }

    int show_all = (!total_flag && !blank_flag && !comment_flag && !source_flag) ? 1 : 0;

    if (num_non_option_args < 1)
        usage(EXIT_FAILURE);

    int total_lines = 0;
    int total_blanks = 0;
    int total_comments = 0;
    int total_sloc = 0;

    for (i = 0; i < num_non_option_args; i++) {

        char * filename = non_option_args[i];

        buffer_t buffer = read_file(filename);

        remove_comments(buffer.p);

        str_arr_t sa = split_into_lines(buffer.p);

        int num_lines = sa.cap;
        int num_blanks = sa.cap - sa.num;

        size_t j;
        int num_comments = 0;
        int nsloc = 0;
        for (j = 0; j < sa.num; j++) {
            if (strlen(sa.strs[j]) == strspn(sa.strs[j], " \t"))
                num_comments++;
            else
                nsloc++;
        }
        if (total_flag || show_all)     printf("%5d ", num_lines);
        if (blank_flag || show_all)     printf("%5d ", num_blanks);
        if (comment_flag || show_all)   printf("%5d ", num_comments);
        if (source_flag || show_all)    printf("%5d ", nsloc);
        printf("%s\n", filename);

        // TODO: reuse these buffers
        free(buffer.p);
        free(sa.strs);

        total_lines += num_lines;
        total_blanks += num_blanks;
        total_comments += num_comments;
        total_sloc += nsloc;
    }

    if (num_non_option_args > 1) {
        if (total_flag || show_all)     printf("%5d ", total_lines);
        if (blank_flag || show_all)     printf("%5d ", total_blanks);
        if (comment_flag || show_all)   printf("%5d ", total_comments);
        if (source_flag || show_all)    printf("%5d ", total_sloc);
        printf("\n");
    }

    free(non_option_args);
    free(options_str);

    return 0;
}

#ifdef TEST

#include "minunit.h"

char err_msg[MAX_ERR_MSG];
int tests_run = 0;

static char * test_strnc__empty_str_has_zero_of_any_char()
{
    mu_assert(strnc("", 'a') == 0, "an empty string has 0 of any character");
    return NULL;
}

static char * test_strnc__null_ptr_returns_zero()
{
    mu_assert(strnc(NULL, 'a') == 0, "a null ptr string argument returns 0");
    return NULL;
}

static char * test_strnc__str_with_none_of_sought_char_returns_0()
{
    mu_assert(strnc("Abcd", 'a') == 0, "string with none of sought char returns 0");
    return NULL;
}

static char * test_strnc()
{
    mu_run_test(test_strnc__empty_str_has_zero_of_any_char);
    mu_run_test(test_strnc__null_ptr_returns_zero);
    mu_run_test(test_strnc__str_with_none_of_sought_char_returns_0);
    return NULL;
}

static char * test_remove_comments__nothing_is_done_with_null_ptr()
{
    remove_comments(NULL);
    mu_assert(1, "nothing is done with null ptr");
    return NULL;
}

static char * test_remove_comments__an_empty_str_is_unchanged_when_comments_are_removed()
{
    char s[] = "";
    remove_comments(s);
    mu_assert(strcmp(s, "") == 0, "an empty string is unchanged when comments are removed");
    return NULL;
}

static char * test_remove_comments__str_with_no_comments_is_unchanged()
{
    char s[] = "hello\n";
    remove_comments(s);
    mu_assert(strcmp(s, "hello\n") == 0, "a string with no comments is unchanged");
    return NULL;
}

static char * test_remove_comments__c_comment_is_replaced_with_spaces()
{
    char s[] = "hello /* how are you */\nthis is the second line\n";
    remove_comments(s);
    mu_assert(strcmp(s, "hello                  \nthis is the second line\n") == 0, "c comment is replaced with spaces");
    return NULL;
}

static char * test_remove_comments__cpp_comment_is_replaced_with_spaces()
{
    char s[] = "hello // how are you\nthis is the second line\n";
    remove_comments(s);
    mu_assert(strcmp(s, "hello               \nthis is the second line\n") == 0, "c++ comment is replaced with spaces");
    return NULL;
}

static char * test_remove_comments__newlines_in_c_comments_are_not_removed()
{
    char s[] = "hello /* this comment\nspans multiple lines */\n";
    remove_comments(s);
    mu_assert(strcmp(s, "hello                \n                       \n") == 0, "newlines in c comments are not removed");
    return NULL;
}

static char * test_remove_comments__c_comments_containing_cpp_comments_are_removed()
{
#define INPUT "a /* b\nc // d */e\n"
    char input[] = INPUT;
    char s[] = INPUT;
    const char * correct = "a     \n         e\n";
    remove_comments(s);
    mu_assert_vaarg(strcmp(s, correct) == 0, "c comments containing c++ comments are removed; given:\n%s\nexpected:\n%s\nbut got:\n%s\n", input, correct, s);
    return NULL;
#undef INPUT
}

static char * test_remove_comments__cpp_comments_containing_c_comments_are_removed()
{
    char s[] = "a// b /* c */ d\ne\n";
    remove_comments(s);
    mu_assert(strcmp(s, "a              \ne\n") == 0, "c++ comments containing c comments are removed");
    return NULL;
}

static char * test_remove_comments__c_comments_in_str_literals_are_not_removed()
{
    char s[] = "a\"b/*c*/d\"e\n";
    remove_comments(s);
    mu_assert(strcmp(s, "a\"b/*c*/d\"e\n") == 0, "c comments in str literals are not removed");
    return NULL;
}

static char * test_remove_comments__edge_cases()
{
    char s[] = "a / / b */ c /**/ d /***/ e /*/ f */ g\n";
    remove_comments(s);
    mu_assert(strcmp(s, "a / / b */ c      d       e          g\n") == 0, "edge cases");
    return NULL;
}

static char * test_remove_comments()
{
    mu_run_test(test_remove_comments__nothing_is_done_with_null_ptr);
    mu_run_test(test_remove_comments__an_empty_str_is_unchanged_when_comments_are_removed);
    mu_run_test(test_remove_comments__str_with_no_comments_is_unchanged);
    mu_run_test(test_remove_comments__c_comment_is_replaced_with_spaces);
    mu_run_test(test_remove_comments__cpp_comment_is_replaced_with_spaces);
    mu_run_test(test_remove_comments__newlines_in_c_comments_are_not_removed);
    mu_run_test(test_remove_comments__c_comments_containing_cpp_comments_are_removed);
    mu_run_test(test_remove_comments__cpp_comments_containing_c_comments_are_removed);
    mu_run_test(test_remove_comments__c_comments_in_str_literals_are_not_removed);
    mu_run_test(test_remove_comments__edge_cases);
    // TODO:
    //   c++ comments in string literals are not removed
    //   comments may contain escape sequences
    //   c comments don't nest
    //   various newline conventions are handled
    return NULL;
}

static char * all_tests()
{
    mu_run_test_suite(test_parse_options);
    mu_run_test_suite(test_strnc);
    mu_run_test_suite(test_remove_comments);
    return NULL;
}

int main()
{

    char * test_results = all_tests();

    if (test_results != NULL) {
        if (isatty(STDOUT_FILENO)) ansi_set(2, ANSI_BOLD, ANSI_FG_RED);
        printf("Test failed: %s\n", test_results);
        if (isatty(STDOUT_FILENO)) ansi_reset();
    } else {
        if (isatty(STDOUT_FILENO)) ansi_set(2, ANSI_BOLD, ANSI_FG_GREEN);
        printf("All tests passed! (%d total)\n", tests_run);
        if (isatty(STDOUT_FILENO)) ansi_reset();
    }

    // TODO: whole-program tests

#if 0
    char * argv[] = { "./build/sloc", "./src/main.c", "--source", NULL };
    int argc = NELEM(argv)-1;
    test_main(argc, argv);
#endif

    return 0;
}
#endif
