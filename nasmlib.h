/* nasmlib.h	header file for nasmlib.c
 *
 * The Netwide Assembler is copyright (C) 1996 Simon Tatham and
 * Julian Hall. All rights reserved. The software is
 * redistributable under the license given in the file "LICENSE"
 * distributed in the NASM archive.
 */

#ifndef NASM_NASMLIB_H
#define NASM_NASMLIB_H

#include "compiler.h"

#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

/*
 * If this is defined, the wrappers around malloc et al will
 * transform into logging variants, which will cause NASM to create
 * a file called `malloc.log' when run, and spew details of all its
 * memory management into that. That can then be analysed to detect
 * memory leaks and potentially other problems too.
 */
/* #define LOGALLOC */

/*
 * -------------------------
 * Error reporting functions
 * -------------------------
 */

/*
 * An error reporting function should look like this.
 */
typedef void (*efunc) (int severity, const char *fmt, ...);
extern efunc nasm_malloc_error;

/*
 * These are the error severity codes which get passed as the first
 * argument to an efunc.
 */

#define ERR_DEBUG	0x00000008      /* put out debugging message */
#define ERR_WARNING	0x00000000      /* warn only: no further action */
#define ERR_NONFATAL	0x00000001      /* terminate assembly after phase */
#define ERR_FATAL	0x00000002      /* instantly fatal: exit with error */
#define ERR_PANIC	0x00000003      /* internal error: panic instantly
                                         * and dump core for reference */
#define ERR_MASK	0x0000000F      /* mask off the above codes */
#define ERR_NOFILE	0x00000010      /* don't give source file name/line */
#define ERR_USAGE	0x00000020      /* print a usage message */
#define ERR_PASS1	0x00000040      /* only print this error on pass one */

/*
 * These codes define specific types of suppressible warning.
 */

#define ERR_WARN_MASK	0x0000FF00      /* the mask for this feature */
#define ERR_WARN_SHR  8			/* how far to shift right */

#define WARN(x) ((x) << ERR_WARN_SHR)

#define ERR_WARN_MNP		WARN(1) /* macro-num-parameters warning */
#define ERR_WARN_MSR		WARN(2) /* macro self-reference */
#define ERR_WARN_OL		WARN(3)	/* orphan label (no colon, and
                                         * alone on line) */
#define ERR_WARN_NOV		WARN(4)	/* numeric overflow */
#define ERR_WARN_GNUELF		WARN(5)	/* using GNU ELF extensions */
#define ERR_WARN_FL_OVERFLOW	WARN(6) /* FP overflow */
#define ERR_WARN_FL_DENORM	WARN(7) /* FP denormal */
#define ERR_WARN_FL_UNDERFLOW	WARN(8)	/* FP underflow */
#define ERR_WARN_FL_TOOLONG	WARN(9) /* FP too many digits */
#define ERR_WARN_MAX	9		/* the highest numbered one */

/*
 * Wrappers around malloc, realloc and free. nasm_malloc will
 * fatal-error and die rather than return NULL; nasm_realloc will
 * do likewise, and will also guarantee to work right on being
 * passed a NULL pointer; nasm_free will do nothing if it is passed
 * a NULL pointer.
 */
void nasm_set_malloc_error(efunc);
#ifndef LOGALLOC
void *nasm_malloc(size_t);
void *nasm_zalloc(size_t);
void *nasm_realloc(void *, size_t);
void nasm_free(void *);
char *nasm_strdup(const char *);
char *nasm_strndup(char *, size_t);
#else
void *nasm_malloc_log(char *, int, size_t);
void *nasm_zalloc_log(char *, int, size_t);
void *nasm_realloc_log(char *, int, void *, size_t);
void nasm_free_log(char *, int, void *);
char *nasm_strdup_log(char *, int, const char *);
char *nasm_strndup_log(char *, int, char *, size_t);
#define nasm_malloc(x) nasm_malloc_log(__FILE__,__LINE__,x)
#define nasm_zalloc(x) nasm_zalloc_log(__FILE__,__LINE__,x)
#define nasm_realloc(x,y) nasm_realloc_log(__FILE__,__LINE__,x,y)
#define nasm_free(x) nasm_free_log(__FILE__,__LINE__,x)
#define nasm_strdup(x) nasm_strdup_log(__FILE__,__LINE__,x)
#define nasm_strndup(x,y) nasm_strndup_log(__FILE__,__LINE__,x,y)
#endif

/*
 * ANSI doesn't guarantee the presence of `stricmp' or
 * `strcasecmp'.
 */
#if defined(HAVE_STRCASECMP)
#define nasm_stricmp strcasecmp
#elif defined(HAVE_STRICMP)
#define nasm_stricmp stricmp
#else
int nasm_stricmp(const char *, const char *);
#endif

#if defined(HAVE_STRNCASECMP)
#define nasm_strnicmp strncasecmp
#elif defined(HAVE_STRNICMP)
#define nasm_strnicmp strnicmp
#else
int nasm_strnicmp(const char *, const char *, size_t);
#endif

int nasm_memicmp(const char *, const char *, size_t);

#if defined(HAVE_STRSEP)
#define nasm_strsep strsep
#else
char *nasm_strsep(char **stringp, const char *delim);
#endif


/*
 * Convert a string into a number, using NASM number rules. Sets
 * `*error' to true if an error occurs, and false otherwise.
 */
int64_t readnum(char *str, bool *error);

/*
 * Convert a character constant into a number. Sets
 * `*warn' to true if an overflow occurs, and false otherwise.
 * str points to and length covers the middle of the string,
 * without the quotes.
 */
int64_t readstrnum(char *str, int length, bool *warn);

/*
 * seg_init: Initialise the segment-number allocator.
 * seg_alloc: allocate a hitherto unused segment number.
 */
void seg_init(void);
int32_t seg_alloc(void);

/*
 * many output formats will be able to make use of this: a standard
 * function to add an extension to the name of the input file
 */
#ifdef NASM_NASM_H
void standard_extension(char *inname, char *outname, char *extension,
                        efunc error);
#endif

/*
 * Utility macros...
 *
 * This is a useful #define which I keep meaning to use more often:
 * the number of elements of a statically defined array.
 */

#define elements(x)     ( sizeof(x) / sizeof(*(x)) )


/*
 * some handy macros that will probably be of use in more than one
 * output format: convert integers into little-endian byte packed
 * format in memory
 */

#if X86_MEMORY

#define WRITECHAR(p,v)				\
    do {					\
	*(uint8_t *)(p) = (v);			\
	(p) += 1;				\
    } while (0)

#define WRITESHORT(p,v)				\
    do {					\
	*(uint16_t *)(p) = (v);			\
	(p) += 2;				\
    } while (0)

#define WRITELONG(p,v)				\
    do {					\
	*(uint32_t *)(p) = (v);			\
	(p) += 4;				\
    } while (0)

#define WRITEDLONG(p,v)				\
    do {					\
	*(uint64_t *)(p) = (v);			\
	(p) += 8;				\
    } while (0)

#define WRITEADDR(p,v,s)			\
    do {					\
	uint64_t _wa_v = (v);			\
	memcpy((p), &_wa_v, (s));		\
	(p) += (s);				\
    } while (0)

#else /* !X86_MEMORY */

#define WRITECHAR(p,v)				\
    do {					\
	uint8_t *_wc_p = (uint8_t *)(p);	\
	uint8_t _wc_v = (v);			\
	_wc_p[0] = _wc_v;			\
	(p) = (void *)(_wc_p + 1);		\
    } while (0)

#define WRITESHORT(p,v)				\
    do {					\
	uint8_t *_ws_p = (uint8_t *)(p);	\
	uint16_t _ws_v = (v);			\
	_ws_p[0] = _ws_v;			\
	_ws_p[1] = _ws_v >> 8;			\
	(p) = (void *)(_ws_p + 2);		\
    } while (0)

#define WRITELONG(p,v)				\
    do {					\
	uint8_t *_wl_p = (uint8_t *)(p);	\
	uint32_t _wl_v = (v);			\
	_wl_p[0] = _wl_v;			\
	_wl_p[1] = _wl_v >> 8;			\
	_wl_p[2] = _wl_v >> 16;			\
	_wl_p[3] = _wl_v >> 24;			\
	(p) = (void *)(_wl_p + 4);		\
    } while (0)

#define WRITEDLONG(p,v)				\
    do {					\
	uint8_t *_wq_p = (uint8_t *)(p);	\
	uint64_t _wq_v = (v);			\
	_wq_p[0] = _wq_v;			\
	_wq_p[1] = _wq_v >> 8;			\
	_wq_p[2] = _wq_v >> 16;			\
	_wq_p[3] = _wq_v >> 24;			\
	_wq_p[4] = _wq_v >> 32;			\
	_wq_p[5] = _wq_v >> 40;			\
	_wq_p[6] = _wq_v >> 48;			\
	_wq_p[7] = _wq_v >> 56;			\
	(p) = (void *)(_wq_p + 8);		\
    } while (0)

#define WRITEADDR(p,v,s)			\
    do {					\
	int _wa_s = (s);			\
	uint64_t _wa_v = (v);			\
	while (_wa_s--) {			\
	    WRITECHAR(p,_wa_v);			\
	    _wa_v >>= 8;			\
	}					\
    } while(0)

#endif

/*
 * and routines to do the same thing to a file
 */
#define fwriteint8_t(d,f) putc(d,f)
void fwriteint16_t(uint16_t data, FILE * fp);
void fwriteint32_t(uint32_t data, FILE * fp);
void fwriteint64_t(uint64_t data, FILE * fp);
void fwriteaddr(uint64_t data, int size, FILE * fp);

/*
 * Binary search routine. Returns index into `array' of an entry
 * matching `string', or <0 if no match. `array' is taken to
 * contain `size' elements.
 *
 * bsi() is case sensitive, bsii() is case insensitive.
 */
int bsi(const char *string, const char **array, int size);
int bsii(const char *string, const char **array, int size);

char *src_set_fname(char *newname);
int32_t src_set_linnum(int32_t newline);
int32_t src_get_linnum(void);
/*
 * src_get may be used if you simply want to know the source file and line.
 * It is also used if you maintain private status about the source location
 * It return 0 if the information was the same as the last time you
 * checked, -1 if the name changed and (new-old) if just the line changed.
 */
int src_get(int32_t *xline, char **xname);

char *nasm_strcat(char *one, char *two);

void null_debug_routine(const char *directive, const char *params);
extern struct dfmt null_debug_form;
extern struct dfmt *null_debug_arr[2];

const char *prefix_name(int);

#endif
