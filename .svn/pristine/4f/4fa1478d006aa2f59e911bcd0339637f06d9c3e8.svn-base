# Custom Macros for the RavenMUD system configure.
AC_DEFUN([AC_CHECK_PROTO],
[
ac_safe=translit($1, './+-', '__p_');

AC_MSG_CHECKING([if $1 is prototyped])
AC_CACHE_VAL(ac_cv_prototype_$ac_safe, [#
  if test $ac_cv_gcc_fnb = yes; then
    OLDCFLAGS=$CFLAGS
    CFLAGS="$CFLAGS -fno-builtin"
  fi
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
#define NO_LIBRARY_PROTOTYPES
#define __COMM_C__
#define __OTHER_C__
#include "$HOME/RavenMUD/src/lib/general/sysdep.h"
#ifdef $1
  error - already defined!
#endif
void $1(int a, char b, int c, char d, int e, char f, int g, char h);
]],
)],[eval "ac_cv_prototype_$ac_safe=no"],[eval "ac_cv_prototype_$ac_safe=yes"])
  if test $ac_cv_gcc_fnb = yes; then
    CFLAGS=$OLDCFLAGS
  fi
])

if eval "test \"`echo '$ac_cv_prototype_'$ac_safe`\" = yes"; then
  AC_MSG_RESULT(yes)
else
  AC_DEFINE(builtin(format, NEED_%s_PROTO, translit($1, 'a-z', 'A-Z')), , Check for a prototype to $1.)
  AC_MSG_RESULT(no)
fi
])

dnl @@@t1="MAKE_PROTO_SAFE($1)"; t2="MAKE_PROTO_NAME($t1)"; literals="$literals $t2"@@@])

AC_DEFUN([AC_UNSAFE_CRYPT], [
  AC_CACHE_CHECK([whether crypt needs over 10 characters], ac_cv_unsafe_crypt, [
    if test ${ac_cv_header_crypt_h-no} = yes; then
      use_crypt_header="#include <crypt.h>"
    fi
    if test ${ac_cv_lib_crypt_crypt-no} = yes; then
      ORIGLIBS=$LIBS
      LIBS="-lcrypt $LIBS"
    fi
    AC_TRY_RUN(
changequote(<<, >>)dnl
<<
#define _XOPEN_SOURCE
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
$use_crypt_header

int main(void)
{
  char pwd[11], pwd2[11];

  strncpy(pwd, (char *)crypt("FooBar", "BazQux"), 10);
  pwd[10] = '\0';
  strncpy(pwd2, (char *)crypt("xyzzy", "BazQux"), 10);
  pwd2[10] = '\0';
  if (strcmp(pwd, pwd2) == 0)
    exit(0);
  exit(1);
}
>>
changequote([, ])dnl
, ac_cv_unsafe_crypt=yes, ac_cv_unsafe_crypt=no, ac_cv_unsafe_crypt=no)])
if test $ac_cv_unsafe_crypt = yes; then
    AC_DEFINE([HAVE_UNSAFE_CRYPT], [1], [Define if we don't have proper support for the system's crypt.])
fi
if test ${ac_cv_lib_crypt_crypt-no} = yes; then
  LIBS=$ORIGLIBS
fi
])

# -- AC_AUTO_DEPEND ---------------------------------------------------------
#
# This will allow the developer to test for special cases where a simple
# -M switch isn't sufficient for building dependencies.
#
# Calls: Nothing
#
# Returns: Nothing
#
# Substitutes: @dep_flags@ @noautodeps@
#
# Sets: ac_cv_auto_dependencies
#
# -- AC_AUTO_DEPEND ---------------------------------------------------------
#
AC_DEFUN([AC_AUTO_DEPEND], [AC_MSG_CHECKING( for compiler autodepency generation)
ac_cv_auto_dependencies=yes
noautodeps=

  case "$target" in
    *-solaris*)
      dep_flags=-xM ;;

    *-hpux9.*)
      if test $ac_cv_c_compiler_gnu = no; then
        noautodeps="1"
        dep_flags=
        ac_cv_auto_dependencies=no
      fi ;;

    *-hpux10.*)
      if test $ac_cv_c_compiler_gnu = no; then
        noautodeps="1"
        dep_flags=
        ac_cv_auto_dependencies=no
      fi ;;
    *)
      dep_flags=-M ;;
    esac

if test $ac_cv_c_compiler_gnu = yes; then
  dep_flags="-M"
  CPPFLAGS="-M"
fi

AC_SUBST([dep_flags]) dnl
AC_SUBST([noautodeps]) dnl
AC_MSG_RESULT($ac_cv_auto_dependencies)

])
# -- AC_AUTO_DEPEND ---------------------------------------------------------

# -- AC_SHARED_LIBS ---------------------------------------------------------
#
# This macros tries to set and export a group of macros to aid in building
# shared libraries on various architectures. A special check needs to be
# for the GNU gcc compiler.
#
# -- AC_SHARED_LIBS ---------------------------------------------------------
#
AC_DEFUN([AC_SHARED_LIBS], [AC_MSG_CHECKING( for shared library support)
  shlib_support=yes
  shlib_compile_flags=
  shlib_link_flags=
  shexe_rpath_flags=
  nosharedlibs=

  if test -n "$ac_vxworks"; then
    acsite_os_def="_VXWORKS"
    nosharedlibs="1"
    shlib_support=no
    shlib_compile_flags=
    shlib_link_flags=
    shexe_rpath_flags=
  else
  case "$target" in

    alpha-*-osf*)
      acsite_os_def="_OSF1AXP"
      shlib_compile_flags=
      shlib_link_flags="-shared -expect_unresolved '*'"
      shexe_rpath_flags="-rpath $(LIB)" ;;

    *-solaris*)
      acsite_os_def="_SOLARIS"
      if test $ac_cv_c_compiler_gnu = no; then
        shlib_compile_flags="-KPIC -mt"
      fi
      shlib_link_flags="-G"
      shexe_rpath_flags="-R $(LIB)" ;;

    *-hpux*)
      acsite_os_def="_HPUX"
      nosharedlibs="1"
      shlib_support=no
      shlib_compile_flags=
      shlib_link_flags=
      shexe_rpath_flags= ;;

    *-irix*.*)
      acsite_os_def="_IRIX"
      shlib_compile_flags="-KPIC"
      shlib_link_flags="-shared"
      shexe_rpath_flags="-rpath $(LIB)" ;;

    *-freebsd*)
      acsite_os_def="_FREEBSD"
      nosharedlibs="1"
      shlib_support=no
      shlib_compile_flags=
      shlib_link_flags=
      shexe_rpath_flags= ;;

    i?86-pc-linux*)
      acsite_os_def="_LINUX"
      nosharedlibs="1"
      if test $ac_cv_c_compiler_gnu = no; then
        shlib_compile_flags="-KPIC"
      fi
      shlib_link_flags=
      shexe_rpath_flags= ;;

    i?86-pc-cygwin*)
      acsite_os_def="_CYGWIN32"
      nosharedlibs="1"
      shlib_support=no
      shlib_compile_flags=
      shlib_link_flags=
      shexe_rpath_flags= ;;

    *)
      AC_MSG_WARN( [$host_os is not currently supported.] )
      shlib_support=no ;;
  esac
  fi

  AC_SUBST([acsite_os_def]) dnl
  AC_MSG_RESULT($shlib_support) dnl
  AC_SUBST([shlib_compile_flags]) dnl
  AC_SUBST([shlib_link_flags]) dnl
  AC_SUBST([shexe_rpath_flags]) dnl
  AC_SUBST([nosharedlibs]) dnl

])
# -- AC_SHARED_LIBS ---------------------------------------------------------


# -- AC_RUNTIME_PROFILING ---------------------------------------------------
#
# Checks to see if the RTPROFILING environment variable is set to non-zero
# and if it is then it forces various things to happen. Most notably is
# disabling the dynamic libraries and enabling the -g debugger switch.
#
# -- AC_RUNTIME_PROFILING ---------------------------------------------------
#
AC_DEFUN([AC_RUNTIME_PROFILING],[AC_MSG_CHECKING( for runtime profiling)

  rtprofiling=
  rtresults=no

  if test -n "$RTPROFILING"; then
    if test "$RTPROFILING" != "0"; then
      rtprofiling="1"
      rtresults=yes

      shlib_support=no
      shlib_compile_flags=
      shlib_link_flags=
      shexe_rpath_flags=
      nosharedlibs="1"

      AC_SUBST([shlib_compile_flags]) dnl
      AC_SUBST([shlib_link_flags]) dnl
      AC_SUBST([shexe_rpath_flags]) dnl
      AC_SUBST([nosharedlibs]) dnl
    fi
  fi

  AC_SUBST([rtprofiling])
  AC_MSG_RESULT($rtresults)
])
