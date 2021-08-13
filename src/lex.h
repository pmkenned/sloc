#ifndef LEX_H
#define LEX_H

typedef enum {
    OUT_KEEP,
    OUT_DELETE
} output_t;

extern void lex_reset();

extern int lex_next(int input);

#endif /* LEX_H */
