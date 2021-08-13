#include "parse_options.h"
#include "lex.h"
#define _POSIX_C_SOURCE 1
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
    while (*s)
        if (*s++ == c)
            n++;
    return n;
}

#if 0
static void
remove_comments(char * buffer, size_t n)
{
    /* strip comments */
    size_t i;
    int in_string_literal = 0;
    int in_c_comment = 0;
    int in_cpp_comment = 0;
    char prev_char = '\0';
    for (i = 0; i < n-1; i++) {
        //printf("%c", buffer[i]);
        if (in_cpp_comment) {
            if (buffer[i] == '\n') {
                in_cpp_comment = 0;
                //printf("(end c++ comment)");
            } else {
                buffer[i] = ' ';
            }
        } else if (in_c_comment) {
            if (buffer[i] == '*' && buffer[i+1] == '/' && prev_char != '/') {
                in_c_comment = 0;
                //printf("(end c comment)");
                buffer[i] = ' ';
                buffer[i+1] = ' ';
            }
            if (buffer[i] != '\n')
                buffer[i] = ' ';
        } else if (in_string_literal) {
            if (buffer[i] == '"' && prev_char != '\\' && prev_char != '\'') {
                in_string_literal = 0;
                //printf("(end string literal)");
            }
            //buffer[i] = '!';
        } else {
            if (buffer[i] == '"') {
                in_string_literal = 1;
                //printf("(start string literal)");
            } else if (buffer[i] == '/' && buffer[i+1] == '/') {
                in_cpp_comment = 1;
                //printf("(start c++ comment)");
                buffer[i] = ' ';
                buffer[i+1] = ' ';
            } else if (buffer[i] == '/' && buffer[i+1] == '*') {
                in_c_comment = 1;
                //printf("(start c comment)");
                buffer[i] = ' ';
                buffer[i+1] = ' ';
            }
        }
        prev_char = buffer[i];
    }
}
#endif

static void
remove_comments(char * buffer)
{
    lex_reset();
    int c;
    while ((c = *buffer)) {
        if (lex_next(c) == OUT_DELETE)
            if (*(buffer-1) != '\n' && *(buffer-1) != '\r')
                *(buffer-1) = ' ';
        buffer++;
    }
}

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

        //remove_comments(buffer.p, buffer.n);
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

    //free(non_option_args);
    //free(options_str);

    return 0;
}

#ifdef TEST
int main()
{
#if 1
    char * argv[] = { "./build/sloc", "./src/main.c", "--source", NULL };
    int argc = NELEM(argv)-1;
    test_main(argc, argv);
#else
    char * argv[] = { "./build/sloc", "./src/main.c", "--source", "5", NULL };
    int argc = NELEM(argv)-1;
    char ** non_option_args;
    size_t num_non_option_args;
    option_t options[] = {
        {"help",    'h',  "show this help",             ARG_NONE,       ARG_TYPE_FLAG, &help_flag     },
        {"version", ' ',  "version number",             ARG_NONE,       ARG_TYPE_FLAG, &version_flag  },
        {"total",   't',  "total number of lines",      ARG_NONE,       ARG_TYPE_FLAG, &total_flag    },
        {"blank",   'b',  "number of blank lines",      ARG_NONE,       ARG_TYPE_FLAG, &blank_flag    },
        {"comment", 'c',  "number of comment lines",    ARG_NONE,       ARG_TYPE_FLAG, &comment_flag  },
        {"source",  's',  "number of source lines",     ARG_REQUIRED,   ARG_TYPE_INT,  &source_flag   },
    };
    const size_t num_options = NELEM(options);
    options_str = gen_options_str(options, num_options);
    parse_options(argc, argv, options, num_options, &non_option_args, &num_non_option_args);
#endif

    test_parse_options();

}
#endif
