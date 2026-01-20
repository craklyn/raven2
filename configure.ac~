#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
# Started editing autoconf on May 11th 2009, which will eventually lead up
# to the complete autotools set for a more dynamic development platform -Xiuh

# Note the prereq version. The min version required to run this autoconf script.
AC_PREREQ(2.59)
AC_INIT([RavenMUD],[3.01],[admin@ravenmud.com])
AC_CANONICAL_TARGET

AC_CONFIG_SRCDIR([src/lib/general/comm.c])
AC_CONFIG_HEADERS([src/lib/general/conf.h])

# This DEFINE seems to serve no point, although it is scattered through the code.
AC_DEFINE([CIRCLE_UNIX], [1], [Define if we're compiling under any type of UNIX system.])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_RANLIB

# Checks for libraries.

# Checks for header files.
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([fcntl.h sys/fcntl.h errno.h net/errno.h limits.h sys/time.h])
AC_CHECK_HEADERS([sys/select.h crypt.h assert.h arpa/telnet.h arpa/inet.h sys/ioctl.h])
AC_CHECK_HEADERS([sys/socket.h sys/resource.h netinet/in.h netdb.h signal.h sys/uio.h mcheck.h math.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_TYPE(ssize_t, int)
AC_HEADER_TIME

# Check for the 'struct in_addr' definition. Ugly, yes.
if test $ac_cv_header_netinet_in_h = no; then
  ac_cv_struct_in_addr = no
else
  if test $ac_cv_header_sys_types_h = yes; then
    headers=`cat << EOF
#include <sys/types.h>
#include <netinet/in.h>
EOF
`
  else
    headers="#include <netinet/in.h>"
  fi

  AC_CACHE_CHECK([for struct in_addr], ac_cv_struct_in_addr,
    [ AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[$headers]], [[struct in_addr tp; tp.s_addr;]])],
    [ac_cv_struct_in_addr=yes],[ac_cv_struct_in_addr=no])])

  if test $ac_cv_struct_in_addr = yes; then
    AC_DEFINE([HAVE_STRUCT_IN_ADDR], [1], [Define is the system has struct in_addr.])
  fi
fi

# Check for the 'typedef socklen_t' definition. Sent to m4 file.
TYPE_SOCKLEN_T

# Checks for library functions.
AC_CHECK_FUNC([crypt], 
   [AC_DEFINE([CIRCLE_CRYPT], [1], [Function being used from the crypt library.])],
   [AC_CHECK_LIB([crypt], [crypt], 
       [AC_SUBST([CRYPTLIB], ["-lcrypt"])
        AC_DEFINE([CIRCLE_CRYPT], [1], [Crypt using library to maintain functionality.])
       ])
   ])

LIB_SOCKET_NSL
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_FUNC_REALLOC

AC_CHECK_FUNCS([gettimeofday snprintf strcasecmp])
AC_CHECK_FUNCS([strdup stricmp strlcpy strncasecmp strnicmp strstr vsnprintf])
AC_CHECK_FUNCS([inet_addr inet_aton])

AC_UNSAFE_CRYPT

#Original configure script macros, located in aclocal.m4. They're nasty.
AC_AUTO_DEPEND
AC_SHARED_LIBS
AC_RUNTIME_PROFILING

# If we're using gcc, use gcc options.
# If not, test for various common switches to make a 'cc' compiler
# compile ANSI C code.
if test $ac_cv_c_compiler_gnu = yes; then

  # Determine if gcc -Wall causes warnings on isascii(), etc.
  AC_CACHE_CHECK([whether ${CC-cc} -Wall also needs -Wno-char-subscripts],
	[ac_cv_char_warn],
  [
    OLDCFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Wall -Werror"
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <ctype.h>]], [[ int i; char c = '0';
         i = isascii(c);
         i = isdigit(c);
         i = isprint(c);
       ]])],[ac_cv_char_warn=no],[ac_cv_char_warn=yes])
    CFLAGS=$OLDCFLAGS
  ])

  # If Determine if gcc can accept -Wno-char-subscripts
  AC_CACHE_CHECK(whether ${CC-cc} accepts -Wno-char-subscripts, ac_cv_gcc_ncs,
  [
    OLDCFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -Wno-char-subscripts"
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])],[ac_cv_gcc_ncs=yes],[ac_cv_gcc_ncs=no])
    CFLAGS=$OLDCFLAGS
  ])

  # If Determine if gcc can accept -fno-builtin
  AC_CACHE_CHECK(whether ${CC-cc} accepts -fno-builtin, ac_cv_gcc_fnb,
  [
    OLDCFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -fno-builtin"
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[]], [[]])],[ac_cv_gcc_fnb=yes],[ac_cv_gcc_fnb=no])
    CFLAGS=$OLDCFLAGS
  ])

  # If gcc -Wall gives no warnings with isascii(), use "-Wall";
  # Otherwise, if gcc -Wall gives isascii warnings:
  #    If we can use -Wno-char-subscripts, use "-Wall -Wno-char-subscripts"
  #    If can't use -Wno-char-subscripts, use no flags at all.
  if test ${ac_cv_char_warn:-ERROR} = no; then
     AC_SUBST([MYFLAGS], ["-Wall"])
  else
    if test ${ac_cv_gcc_ncs:-ERROR} = yes; then
      AC_SUBST([MYFLAGS], ["-Wall -Wno-char-subscripts"])
    else
      AC_SUBST([MYFLAGS], [""])
    fi
  fi

else
  # We aren't using gcc so we can't assume any special flags.
  AC_SUBST([MYFLAGS], [""])
fi

# Check for prototypes
AC_CHECK_PROTO([strlcpy])

AC_CONFIG_FILES([Makedefs
                     Makerules
                     Makefile
	             src/Makefile
	             src/lib/Makefile
	             src/lib/general/Makefile
	             src/lib/actions/Makefile
	             src/lib/magic/Makefile
	             src/lib/olc/Makefile
	             src/lib/specials/Makefile
	             src/lib/scripts/Makefile
	             src/lib/util/Makefile
	             src/bin/Makefile
	             src/bin/binaries/Makefile])

AC_OUTPUT

echo \
"-------------------------------------------------
  ($PACKAGE_NAME) version $PACKAGE_VERSION
  Prefix.........: $prefix
  C Compiler.....: $CC $CFLAGS $CPPFLAGS
  Linker.........: $LD $LDFLAGS $LIBS
  Now type 'make @<:@<target>@:>@'
   where the optional <target> is:
     all                - build all binaries
     install            - install everything

--------------------------------------------------"
