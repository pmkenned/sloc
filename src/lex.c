#include "lex.h"
#include <assert.h>

static enum {
    ST_S0    ,
    ST_S1    ,
    ST_DQ    ,
    ST_DQS   ,
    ST_SQ    ,
    ST_SQS   ,
    ST_SQX   ,
    ST_SQXS  ,
    ST_FS    ,
    ST_FSFS  ,
    ST_FSA   ,
    ST_FSAA  ,
    ST_ERR   ,
} curr_state, next_state;

/*
      "     \'    \\    *     /     \n    else
S0    DQ    SQ    ERR   S0    FS    S0    S0
S1    DQ*   SQ*   ERR*  S0*   FS*   S0*   S0*
DQ    S0    DQ    DQS   DQ    DQ    ERR   DQ
DQS   DQ    DQ    DQ    DQ    DQ    DQ    DQ
SQ    SQX   ERR   SQS   SQX   SQX   ERR   SQX
SQS   SQX   SQX   SQX   SQX   SQX   SQ    SQX
SQX   ERR   S0    SQXS  ERR   ERR   ERR   ERR
SQXS  ERR   ERR   ERR   ERR   ERR   SQX   ERR
FS    S0    S0    S0    FSA*  FSFS* S0    S0
FSFS  FSFS* FSFS* FSFS* FSFS* FSFS* S0*   FSFS*
FSA   FSA*  FSA*  FSA*  FSAA* FSA*  FSA*  FSA*
FSAA  FSA*  FSA*  FSA*  FSAA* S1*   FSA*  FSA*
ERR   ERR   ERR   ERR   ERR   ERR   ERR   ERR
*/

void
lex_reset()
{
    curr_state = ST_S0;
}

int
lex_next(int input)
{
    int output = 0;

    switch (curr_state) {
        case ST_S0:
            if (input == '"')       { next_state = ST_DQ  ; output = 0; }
            else if (input == '\'') { next_state = ST_SQ  ; output = 0; }
            else if (input == '\\') { next_state = ST_ERR ; output = 0; }
            else if (input == '*')  { next_state = ST_S0  ; output = 0; }
            else if (input == '/')  { next_state = ST_FS  ; output = 0; }
            else if (input == '\n') { next_state = ST_S0  ; output = 0; }
            else                    { next_state = ST_S0  ; output = 0; }
            break;
        case ST_S1:
            if (input == '"')       { next_state = ST_DQ  ; output = 1; }
            else if (input == '\'') { next_state = ST_SQ  ; output = 1; }
            else if (input == '\\') { next_state = ST_ERR ; output = 1; }
            else if (input == '*')  { next_state = ST_S0  ; output = 1; }
            else if (input == '/')  { next_state = ST_FS  ; output = 1; }
            else if (input == '\n') { next_state = ST_S0  ; output = 1; }
            else                    { next_state = ST_S0  ; output = 1; }
            break;
        case ST_DQ:
            if (input == '"')       { next_state = ST_S0  ; output = 0; }
            else if (input == '\'') { next_state = ST_DQ  ; output = 0; }
            else if (input == '\\') { next_state = ST_DQS ; output = 0; }
            else if (input == '*')  { next_state = ST_DQ  ; output = 0; }
            else if (input == '/')  { next_state = ST_DQ  ; output = 0; }
            else if (input == '\n') { next_state = ST_ERR ; output = 0; }
            else                    { next_state = ST_DQ  ; output = 0; }
            break;
        case ST_DQS:
            if (input == '"')       { next_state = ST_DQ  ; output = 0; }
            else if (input == '\'') { next_state = ST_DQ  ; output = 0; }
            else if (input == '\\') { next_state = ST_DQ  ; output = 0; }
            else if (input == '*')  { next_state = ST_DQ  ; output = 0; }
            else if (input == '/')  { next_state = ST_DQ  ; output = 0; }
            else if (input == '\n') { next_state = ST_DQ  ; output = 0; }
            else                    { next_state = ST_DQ  ; output = 0; }
            break;
        case ST_SQ:
            if (input == '"')       { next_state = ST_SQX ; output = 0; }
            else if (input == '\'') { next_state = ST_ERR ; output = 0; }
            else if (input == '\\') { next_state = ST_SQS ; output = 0; }
            else if (input == '*')  { next_state = ST_SQX ; output = 0; }
            else if (input == '/')  { next_state = ST_SQX ; output = 0; }
            else if (input == '\n') { next_state = ST_ERR ; output = 0; }
            else                    { next_state = ST_SQX ; output = 0; }
            break;
        case ST_SQS:
            if (input == '"')       { next_state = ST_SQX ; output = 0; }
            else if (input == '\'') { next_state = ST_SQX ; output = 0; }
            else if (input == '\\') { next_state = ST_SQX ; output = 0; }
            else if (input == '*')  { next_state = ST_SQX ; output = 0; }
            else if (input == '/')  { next_state = ST_SQX ; output = 0; }
            else if (input == '\n') { next_state = ST_SQ  ; output = 0; }
            else                    { next_state = ST_SQX ; output = 0; }
            break;
        case ST_SQX:
            if (input == '"')       { next_state = ST_ERR ; output = 0; }
            else if (input == '\'') { next_state = ST_S0  ; output = 0; }
            else if (input == '\\') { next_state = ST_SQXS; output = 0; }
            else if (input == '*')  { next_state = ST_ERR ; output = 0; }
            else if (input == '/')  { next_state = ST_ERR ; output = 0; }
            else if (input == '\n') { next_state = ST_ERR ; output = 0; }
            else                    { next_state = ST_ERR ; output = 0; }
            break;
        case ST_SQXS:
            if (input == '"')       { next_state = ST_ERR ; output = 0; }
            else if (input == '\'') { next_state = ST_ERR ; output = 0; }
            else if (input == '\\') { next_state = ST_ERR ; output = 0; }
            else if (input == '*')  { next_state = ST_ERR ; output = 0; }
            else if (input == '/')  { next_state = ST_ERR ; output = 0; }
            else if (input == '\n') { next_state = ST_SQX ; output = 0; }
            else                    { next_state = ST_ERR ; output = 0; }
            break;
        case ST_FS:
            if (input == '"')       { next_state = ST_S0  ; output = 0; }
            else if (input == '\'') { next_state = ST_S0  ; output = 0; }
            else if (input == '\\') { next_state = ST_S0  ; output = 0; }
            else if (input == '*')  { next_state = ST_FSA ; output = 1; }
            else if (input == '/')  { next_state = ST_FSFS; output = 1; }
            else if (input == '\n') { next_state = ST_S0  ; output = 0; }
            else                    { next_state = ST_S0  ; output = 0; }
            break;
        case ST_FSFS:
            if (input == '"')       { next_state = ST_FSFS; output = 1; }
            else if (input == '\'') { next_state = ST_FSFS; output = 1; }
            else if (input == '\\') { next_state = ST_FSFS; output = 1; }
            else if (input == '*')  { next_state = ST_FSFS; output = 1; }
            else if (input == '/')  { next_state = ST_FSFS; output = 1; }
            else if (input == '\n') { next_state = ST_S0  ; output = 1; }
            else                    { next_state = ST_FSFS; output = 1; }
            break;
        case ST_FSA:
            if (input == '"')       { next_state = ST_FSA ; output = 1; }
            else if (input == '\'') { next_state = ST_FSA ; output = 1; }
            else if (input == '\\') { next_state = ST_FSA ; output = 1; }
            else if (input == '*')  { next_state = ST_FSAA; output = 1; }
            else if (input == '/')  { next_state = ST_FSA ; output = 1; }
            else if (input == '\n') { next_state = ST_FSA ; output = 1; }
            else                    { next_state = ST_FSA ; output = 1; }
            break;
        case ST_FSAA:
            if (input == '"')       { next_state = ST_FSA ; output = 1; }
            else if (input == '\'') { next_state = ST_FSA ; output = 1; }
            else if (input == '\\') { next_state = ST_FSA ; output = 1; }
            else if (input == '*')  { next_state = ST_FSAA; output = 1; }
            else if (input == '/')  { next_state = ST_S1  ; output = 1; }
            else if (input == '\n') { next_state = ST_FSA ; output = 1; }
            else                    { next_state = ST_FSA ; output = 1; }
            break;
        case ST_ERR:
            if (input == '"')       { next_state = ST_ERR ; output = 0; }
            else if (input == '\'') { next_state = ST_ERR ; output = 0; }
            else if (input == '\\') { next_state = ST_ERR ; output = 0; }
            else if (input == '*')  { next_state = ST_ERR ; output = 0; }
            else if (input == '/')  { next_state = ST_ERR ; output = 0; }
            else if (input == '\n') { next_state = ST_ERR ; output = 0; }
            else                    { next_state = ST_ERR ; output = 0; }
            break;
        default:
            assert(0);
            break;
    }

    curr_state = next_state;
    return output;
}
