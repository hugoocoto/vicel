/* ANSI escape code
 *
 * Author: Hugo Coto Florez
 * Github: hugoocoto
 *
 * Api usage: Printf it
 *
 * Referece: https://en.wikipedia.org/wiki/ANSI_escape_code
 * */


/* C0 control codes */
#define T_BEL "\x07"
#define T_BS "\x08"
#define T_TAB "\x09"
#define T_LF "\x0a"
#define T_FF "\x0c"
#define T_CR "\x0d"
#define T_ESC "\x1b"

/* Fe Escape sequences */
#define T_SS2 T_ESC "N"
#define T_SS3 T_ESC "O"
#define T_DCS T_ESC "P"
#define T_CSI T_ESC "["
#define T_ST T_ESC "\\"
#define T_OSC T_ESC "]"
#define T_SOS T_ESC "X"
#define T_PM T_ESC "^"
#define T_APC T_ESC "_"

#define N(n, ...) #n
#define M(n, ...) __VA_OPT__(N(__VA_ARGS__))

/* Control Secuente Introducer commands */
#define T_CUU(n) printf(T_CSI "%dA", n)
#define T_CUD(n) printf(T_CSI "%dB", n)
#define T_CUF(n) printf(T_CSI "%dC", n)
#define T_CUB(n) printf(T_CSI "%dD", n)
#define T_CNL(n) printf(T_CSI "%dE", n)
#define T_CPL(n) printf(T_CSI "%dF", n)
#define T_CHA(n) printf(T_CSI "%dG", n)
#define T_CUP(r, c) printf(T_CSI "%d;%dH", r, c)
#define T_ED(n) printf(T_CSI "%dJ", n)
#define T_EL(n) printf(T_CSI "%dK", n)
#define T_SU(n) printf(T_CSI "%dS", n)
#define T_SD(n) printf(T_CSI "%dT", n)
#define T_HVP(r, c) printf(T_CSI "%d;%df", r, c)
#define T_SGR(n) printf(T_CSI "%d", n)

#define T_APON() printf(T_CSI "5i")
#define T_APOFF() printf(T_CSI "4i")
#define T_DSR() printf(T_CSI "6n")

/* Private sequences */
#define T_SCP() printf(T_CSI "s")       // save current cursor position
#define T_RCP() printf(T_CSI "u")       // restore saved cursor position
#define T_CUSHW() printf(T_CSI "?25h")  // show cursor
#define T_CUHDE() printf(T_CSI "?25l")  // hide cursor
#define T_RFE() printf(T_CSI "?1004h")  // enable report focus. Enter focus as `ESC [I`, exit as `ESC [O`
#define T_RFD() printf(T_CSI "?1004l")  // disable report focus.
#define T_ASBE() printf(T_CSI "?1049h") // enable alternative screen buffer
#define T_ASBD() printf(T_CSI "?1049l") // disable alternative screen buffer
#define T_BPME() printf(T_CSI "?2004h") // enable bracketed paste mode
#define T_BPMD() printf(T_CSI "?2004l") // disable bracketed paste mode

/* Select Graphic Rendition parameters */
#define _EFJOIN00(ef, ...) #ef __VA_OPT__(";" _EFJOIN01(__VA_ARGS__))
#define _EFJOIN01(ef, ...) #ef __VA_OPT__(";" _EFJOIN02(__VA_ARGS__))
#define _EFJOIN02(ef, ...) #ef __VA_OPT__(";" _EFJOIN03(__VA_ARGS__))
#define _EFJOIN03(ef, ...) #ef __VA_OPT__(";" _EFJOIN04(__VA_ARGS__))
#define _EFJOIN04(ef, ...) #ef __VA_OPT__(";" _EFJOIN05(__VA_ARGS__))
#define _EFJOIN05(ef, ...) #ef __VA_OPT__(";" _EFJOIN06(__VA_ARGS__))
#define _EFJOIN06(ef, ...) #ef __VA_OPT__(";" _EFJOIN07(__VA_ARGS__))
#define _EFJOIN07(ef, ...) #ef __VA_OPT__(";" _EFJOIN08(__VA_ARGS__))
#define _EFJOIN08(ef, ...) #ef __VA_OPT__(";" _EFJOIN09(__VA_ARGS__))
#define _EFJOIN09(ef, ...) #ef __VA_OPT__(";" _EFJOIN10(__VA_ARGS__))
#define _EFJOIN10(ef, ...) #ef __VA_OPT__(";" _EFJOIN11(__VA_ARGS__))
#define _EFJOIN11(ef, ...) #ef

#define EFFECT(...) printf(T_CSI __VA_OPT__(_EFJOIN00(__VA_ARGS__)) "m")
#define COL(c) printf(T_CSI "%dm", c)

#define RESET 0
#define NORMAL 0
#define BOLD 1
#define FAINT 2
#define ITALIC 3
#define UNDERLINE 4
#define SBLINK 5
#define RBLINK 6
#define INVERT 7
#define REVERSE 7
#define REVERSED 7
#define CONCEAL 8
#define STRIKE 9
#define FONT_1 10
#define FONT_2 11
#define FONT_3 12
#define FONT_4 13
#define FONT_5 14
#define FONT_6 15
#define FONT_7 16
#define FONT_8 17
#define FONT_9 18
#define FONT_10 19
#define FRAKTUR 20
#define DOUBLY_UNDERLINE 21
#define DISABLE_BOLD 21
#define BOLD_OFF 22
#define ITALIC_OFF 23
#define UNDERLINE_OFF 24
#define BLINK_OFF 25
#define PROPOTIONAL_SPACING 26
#define REVERSED_OFF 27
#define REVERSE_OFF 27
#define CONCEAL_OFF 28
#define STRIKE_OFF 29
#define FG 30
#define FG_BLACK 30
#define FG_RED 31
#define FG_GREEN 32
#define FG_YELLOW 33
#define FG_BLUE 34
#define FG_MAGENTA 35
#define FG_CYAN 36
#define FG_WHITE 37
#define FG_SET 38
#define FG_DEFAULT 39
#define BG 40
#define BG_BLACK 40
#define BG_RED 41
#define BG_GREEN 42
#define BG_YELLOW 43
#define BG_BLUE 44
#define BG_MAGENTA 45
#define BG_CYAN 46
#define BG_WHITE 47
#define BG_SET 48
#define BG_DEFAULT 49
#define PROPOTIONAL_SPACING_OFF 50
#define FRAMED 51
#define ENCIRCLED 52
#define OVERLINED 53
#define FRAMED_OFF 54
#define ENCIRCLED_OFF 54
#define OVERLINED_OFF 55
#define UNDERLINE_SET 58
#define UNDERLINE_DEFAULT 59
#define IDEOGRAM_UNDERLINE 60
#define RIGHT_SIDE_LINE 60
#define IDEOGRAM_DOUBLE_UNDERLINE 61
#define RIGHT_SIDE_DOUBLE_LINE 61
#define IDEOGRAM_OVERLINE 62
#define LEFT_SIDE_LINE 62
#define IDEOGRAM_DOUBLE_OVERLINE 63
#define LEFT_SIDE_DOUBLE_LINE 63
#define IDEOGRAM_STRESS_MARKING 64
#define NO_IDEOGRAM_ATTRS 65
#define SUPERSCRIPT 73
#define SUBSCRIPT 74
#define SUPERSCRIPT_OFF 75
#define SUBSCRIPT_OFF 75
#define BRFG 90
#define BRFG_BLACK 90
#define BRFG_RED 91
#define BRFG_GREEN 92
#define BRFG_YELLOW 93
#define BRFG_BLUE 94
#define BRFG_MAGENTA 95
#define BRFG_CYAN 96
#define BRFG_WHITE 97
#define BRFG_SET 98
#define BRFG_DEFAULT 99
#define BRBG 100
#define BRBG_BLACK 100
#define BRBG_RED 101
#define BRBG_GREEN 102
#define BRBG_YELLOW 103
#define BRBG_BLUE 104
#define BRBG_MAGENTA 105
#define BRBG_CYAN 106
#define BRBG_WHITE 107
#define BRBG_SET 108
#define BRBG_DEFAULT 109

#define BLACK 0
#define RED 1
#define GREEN 2
#define YELLOW 3
#define BLUE 4
#define MAGENTA 5
#define CYAN 6
#define WHITE 7

#define FG_RGB(r, g, b) FG_SET "38;2;" #r ";" #g ";" #b "m"
#define BG_RGB(r, g, b) FG_SET "48;2;" #r ";" #g ";" #b "m"

#define COL_FG_RGB(r, g, b) printf(FG_SET "38;2;%d;%d;%d;m", r, g, b)
#define COL_BG_RGB(r, g, b) printf(FG_SET "48;2;%d;%d;%d;m", r, g, b)

#define SET_WINDOW_TITLE(title) printf(T_OSC "0;%s " T_BEL, title)
#define HYPRLINK(link) printf(T_OSC "8;;%s " T_ST, link)
#define SET_PALETTE(n, rr, gg, bb) printf(T_OSC "P %d %d %d %d " ST, n, rr, gg, bb)

#define clear_screen() printf("\033[K\033[2J"); // clear screen
