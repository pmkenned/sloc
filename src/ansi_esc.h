#ifndef ANSI_ESC_H
#define ANSI_ESC_H

#define ANSI_ESC                "\033["
#define ANSI_RESET              "\033[0m"

#define ANSI_UNDERLINE          "4"
#define ANSI_UNDERLINE_OFF      "24"
#define ANSI_BOLD               "1"
#define ANSI_BOLD_OFF           "21"

#define ANSI_FG_BLACK           "30"
#define ANSI_FG_RED             "31"
#define ANSI_FG_GREEN           "32"
#define ANSI_FG_YELLOW          "33"
#define ANSI_FG_BLUE            "34"
#define ANSI_FG_MAGENTA         "35"
#define ANSI_FG_CYAN            "36"
#define ANSI_FG_WHITE           "37"
#define ANSI_FG_BRIGHT_BLACK    "90"
#define ANSI_FG_BRIGHT_RED      "91"
#define ANSI_FG_BRIGHT_GREEN    "92"
#define ANSI_FG_BRIGHT_YELLOW   "93"
#define ANSI_FG_BRIGHT_BLUE     "94"
#define ANSI_FG_BRIGHT_MAGENTA  "95"
#define ANSI_FG_BRIGHT_CYAN     "96"
#define ANSI_FG_BRIGHT_WHITE    "97"

#define ANSI_BG_BLACK           "40"
#define ANSI_BG_RED             "41"
#define ANSI_BG_GREEN           "42"
#define ANSI_BG_YELLOW          "43"
#define ANSI_BG_BLUE            "44"
#define ANSI_BG_MAGENTA         "45"
#define ANSI_BG_CYAN            "46"
#define ANSI_BG_WHITE           "47"
#define ANSI_BG_BRIGHT_BLACK    "100"
#define ANSI_BG_BRIGHT_RED      "101"
#define ANSI_BG_BRIGHT_GREEN    "102"
#define ANSI_BG_BRIGHT_YELLOW   "103"
#define ANSI_BG_BRIGHT_BLUE     "104"
#define ANSI_BG_BRIGHT_MAGENTA  "105"
#define ANSI_BG_BRIGHT_CYAN     "106"
#define ANSI_BG_BRIGHT_WHITE    "107"

void ansi_set(int n, ...);
void ansi_reset();

#endif /* ANSI_ESC_H */
