/**
* @file sysdep.h
* Machine-specific defs based on values in conf.h (from configure)
*
* Part of the core tbaMUD source code distribution, which is a derivative
* of, and continuation of, CircleMUD.
*
* All rights reserved.  See license for complete information.
* Copyright (C) 1993, 94 by the Trustees of the Johns Hopkins University
* CircleMUD is based on DikuMUD, Copyright (C) 1990, 1991.
*/
#ifndef _SYSDEP_H_
#define _SYSDEP_H_

/* Configurables: tbaMUD uses the crypt(3) function to encrypt player passwords
 * in the players file so that they are never stored in plaintext form. However,
 * due to U.S. export restrictions on machine-readable cryptographic software,
 * the crypt() function is not available on some operating systems such as
 * FreeBSD.  By default, the 'configure' script will determine if you have
 * crypt() available and enable or disable password encryption appropriately.
 * #define NOCRYPT (by uncommenting the line below) if you'd like to explicitly
 * disable password encryption (i.e., if you have moved your MUD from an OS that
 * does not support encryption to one that does). */
/* #define NOCRYPT */

/* If you are porting tbaMUD to a new (untested) platform and you find that
 * POSIX-standard non-blocking I/O does *not* work, you can define the constant
 * below to work around the problem.  Not having non-blocking I/O can cause the
 * MUD to freeze if someone types part of a command while the MUD waits for the
 * remainder of the command.
 *
 * NOTE: **DO** **NOT** use this constant unless you are SURE you understand
 * exactly what non-blocking I/O is, and you are SURE that your operating system
 * does NOT have it!  (The only UNIX system I've ever seen that has broken POSIX
 * non-blocking I/O is AIX 3.2.)  If your MUD is freezing but you're not sure
 * why, do NOT use this constant.  Use this constant ONLY if you're sure that
 * your MUD is freezing because of a non-blocking I/O problem. */
/* #define POSIX_NONBLOCK_BROKEN */

/* The code prototypes library functions to avoid compiler warnings. (Operating
 * system header files *should* do this, but sometimes don't.) However, Circle's
 * prototypes cause the compilation to fail under some combinations of operating
 * systems and compilers. If your compiler reports "conflicting types" for
 * functions, you need to define this constant to turn off library function
 * prototyping.  Note, **DO** **NOT** blindly turn on this constant unless you
 * are sure the problem is type conflicts between my header files and the header
 * files of your operating system.  The error message will look something like
 * this: In file included from comm.c:14:
 *    sysdep.h:207: conflicting types for `random'
 * /usr/local/lib/gcc-lib/alpha-dec-osf3.2/2.7.2/include/stdlib.h:253:
 *    previous declaration of `random' */
/* #define NO_LIBRARY_PROTOTYPES */

/* If using the GNU C library, version 2+, then you can have it trace memory
 * allocations to check for leaks, uninitialized uses, and bogus free() calls.
 * To see if your version supports it, run:
 * info libc 'Allocation Debugging' 'Tracing malloc'
 * Example usage (Bourne shell):
 *      MALLOC_TRACE=/tmp/circle-trace bin/circle
 * Read the entire "Allocation Debugging" section of the GNU C library
 * documentation before setting this to '1'. */
#define CIRCLE_GNU_LIBC_MEMORY_TRACK	0	/* 0 = off, 1 = on */

/* Do not change anything below this line. */

/* Set up various machine-specific things based on the values determined from
 * configure and conf.h. */

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifdef HAVE_MATH_H
#include <math.h>
#endif

#if     (defined (STDC_HEADERS) || defined (__GNU_LIBRARY__))
#include <stdlib.h>

#else   /* No standard headers.  */

#ifdef  HAVE_MEMORY_H
#include <memory.h>
#endif

extern char *malloc(), *calloc(), *realloc();
extern void free ();

extern void abort (), exit ();

#endif  /* Standard headers.  */

/* POSIX compliance */
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#ifdef CIRCLE_WINDOWS
# include <sys\types.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

/* Now, we #define POSIX if we have a POSIX system. */

#ifdef HAVE_UNISTD_H
/* Ultrix's unistd.h always defines _POSIX_VERSION, but you only get
   POSIX.1 behavior with `cc -YPOSIX', which predefines POSIX itself!  */
#if defined (_POSIX_VERSION) && !defined (ultrix)
#define POSIX
#endif

/* Some systems define _POSIX_VERSION but are not really POSIX.1.  */
#if (defined (butterfly) || defined (__arm) || \
     (defined (__mips) && defined (_SYSTYPE_SVR3)) || \
     (defined (sequent) && defined (i386)))
#undef POSIX
#endif
#endif /* HAVE_UNISTD_H */

#if !defined (POSIX) && defined (_AIX) && defined (_POSIX_SOURCE)
#define POSIX
#endif

#if defined(_AIX)
#define POSIX_NONBLOCK_BROKEN
#endif

/* Header files common to all source files */
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_NET_ERRNO_H
#include <net/errno.h>
#endif

/* Macintosh */
#ifdef HAVE_SYS_ERRNO_H
#include <sys/errno.h>
#endif

#ifdef HAVE_CRYPT_H
#include <crypt.h>
#endif

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef HAVE_ASSERT_H
#include <assert.h>
#else
#define assert(arg)
#endif

/* Header files only used in comm.c and some of the utils */
#if defined(__COMM_C__) || defined(__AUTOWIZ_C__)

#ifndef HAVE_STRUCT_IN_ADDR
struct in_addr {
  unsigned long int s_addr;	/* for inet_addr, etc. */
}
#endif

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#ifdef HAVE_SYS_RESOURCE_H
# include <sys/resource.h>
#endif

#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif

#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif

#ifdef HAVE_SIGNAL_H
# ifndef _POSIX_C_SOURCE
#  define _POSIX_C_SOURCE 2
#  include <signal.h>
#  undef _POSIX_C_SOURCE
# else
#  include <signal.h>	/* GNU libc 6 already defines _POSIX_C_SOURCE. */
# endif
#endif

#ifdef HAVE_SYS_UIO_H
# include <sys/uio.h>
#endif

#endif /* __COMM_C__ && CIRCLE_UNIX */

/* Header files that are only used in act.other.c */
#ifdef __OTHER_C__

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#endif /* __OTHER_C__ */

/* Basic system dependencies. */
#if CIRCLE_GNU_LIBC_MEMORY_TRACK && !defined(HAVE_MCHECK_H)
#error "Cannot use GNU C library memory tracking without <mcheck.h>"
#endif

/* strcasecmp -> stricmp -> str_cmp */
#if defined(HAVE_STRCASECMP)
# define str_cmp strcasecmp
#elif defined(HAVE_STRICMP)
# define str_cmp stricmp
#endif

/* strncasecmp -> strnicmp -> strn_cmp */
#if defined(HAVE_STRNCASECMP)
# define strn_cmp strncasecmp
#elif defined(HAVE_STRNICMP)
# define strn_cmp strnicmp
#endif

#if !defined(__GNUC__)
# define __attribute__(x)	/* nothing */
#endif

#if defined(__MWERKS__)
# define isascii(c)	(((c) & ~0x7f) == 0)	/* So easy to have, but ... */
#endif

/* Socket/header miscellany. */

#if defined(CIRCLE_WINDOWS)	/* Definitions for Win32 */

# define snprintf _snprintf
# define vsnprintf _vsnprintf
# define PATH_MAX MAX_PATH

# if !defined(__BORLANDC__) && !defined(LCC_WIN32)	/* MSVC */
#  define chdir _chdir
#  pragma warning(disable:4761)		/* Integral size mismatch. */
#  pragma warning(disable:4244)		/* Possible loss of data. */
# endif

# if defined(__BORLANDC__)	/* Silence warnings we don't care about. */
#  pragma warn -par	/* to turn off >parameter< 'ident' is never used. */
#  pragma warn -pia	/* to turn off possibly incorrect assignment. 'if (!(x=a))' */
#  pragma warn -sig	/* to turn off conversion may lose significant digits. */
# endif

# ifndef _WINSOCK2API_	/* Winsock1 and Winsock 2 conflict. */
#  include <winsock.h>
# endif

# ifndef FD_SETSIZE	/* MSVC 6 is reported to have 64. */
#  define FD_SETSIZE		1024
# endif

#elif defined(CIRCLE_VMS)

/* Necessary Definitions For DEC C With DEC C Sockets Under OpenVMS. */
# if defined(DECC)
#  include <stdio.h>
#  include <time.h>
#  include <stropts.h>
#  include <unixio.h>
# endif

#elif !defined(CIRCLE_MACINTOSH) && !defined(CIRCLE_UNIX) && !defined(CIRCLE_ACORN)
# error "You forgot to include conf.h or do not have a valid system define."
#endif

/* SOCKET -- must be after the winsock.h #include. */
#ifdef CIRCLE_WINDOWS
# define CLOSE_SOCKET(sock)	closesocket(sock)
  typedef SOCKET		socket_t;
#else
# define CLOSE_SOCKET(sock)	close(sock)
  typedef int			socket_t;
#endif

#if defined(__cplusplus)	/* C++ */
#define cpp_extern	extern
#else				/* C */
#define cpp_extern	/* Nothing */
#endif

/* Guess if we have the getrlimit()/setrlimit() functions */
#if defined(RLIMIT_NOFILE) || defined (RLIMIT_OFILE)
#define HAS_RLIMIT
#if !defined (RLIMIT_NOFILE)
# define RLIMIT_NOFILE RLIMIT_OFILE
#endif
#endif

/* Make sure we have STDERR_FILENO */
#ifndef STDERR_FILENO
#define STDERR_FILENO 2
#endif

/* Make sure we have STDOUT_FILENO too. */
#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

  /* Function prototypes. */
/* Header files of many OS's do not contain function prototypes for the
 * standard C library functions.  This produces annoying warning messages
 * (sometimes, a lot of them) on such OS's when compiling with gcc's -Wall.
 *
 * Configuration script has been changed to detect which prototypes exist
 * already; this header file only prototypes functions that aren't already
 * prototyped by the system headers.  A clash should be impossible.  This
 * should give us our strong type-checking back. */

#ifndef NO_LIBRARY_PROTOTYPES

#ifdef NEED_STRLCPY_PROTO
   size_t strlcpy(char *dest, const char *src, size_t copylen);
#endif

/* Function prototypes that are only used in comm.c and some of the utils */
#if defined(__COMM_C__) || defined(CIRCLE_UTIL)

#ifdef NEED_SELECT_PROTO
   int select(int nfds, fd_set *readfds, fd_set *writefds,
          fd_set *exceptfds, struct timeval *timeout);
#endif
   
#endif /* __COMM_C__ */

#endif /* NO_LIBRARY_PROTOTYPES */

#endif /* _SYSDEP_H_ */

