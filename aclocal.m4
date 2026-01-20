# generated automatically by aclocal 1.10.2 -*- Autoconf -*-

# Copyright (C) 1996, 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004,
# 2005, 2006, 2007, 2008  Free Software Foundation, Inc.
# This file is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

# ===========================================================================
#             http://autoconf-archive.cryp.to/lib_socket_nsl.html
# ===========================================================================
#
# SYNOPSIS
#
#   LIB_SOCKET_NSL
#
# DESCRIPTION
#
#   This macro figures out what libraries are required on this platform to
#   link sockets programs.
#
#   The common cases are not to need any extra libraries, or to need
#   -lsocket and -lnsl. We need to avoid linking with libnsl unless we need
#   it, though, since on some OSes where it isn't necessary it will totally
#   break networking. Unisys also includes gethostbyname() in libsocket but
#   needs libnsl for socket().
#
# LAST MODIFICATION
#
#   2008-04-12
#
# COPYLEFT
#
#   Copyright (c) 2008 Russ Allbery <rra@stanford.edu>
#   Copyright (c) 2008 Stepan Kasal <kasal@ucw.cz>
#   Copyright (c) 2008 Warren Young <warren@etr-usa.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved.

AC_DEFUN([LIB_SOCKET_NSL],
[
	AC_SEARCH_LIBS([gethostbyname], [nsl])
	AC_SEARCH_LIBS([socket], [socket], [], [
		AC_CHECK_LIB([socket], [socket], [LIBS="-lsocket -lnsl $LIBS"],
		[], [-lnsl])])
])

# ===========================================================================
#             http://autoconf-archive.cryp.to/type_socklen_t.html
# ===========================================================================
#
# SYNOPSIS
#
#   TYPE_SOCKLEN_T
#
# DESCRIPTION
#
#   Check whether sys/socket.h defines type socklen_t. Please note that some
#   systems require sys/types.h to be included before sys/socket.h can be
#   compiled.
#
# LAST MODIFICATION
#
#   2008-04-12
#
# COPYLEFT
#
#   Copyright (c) 2008 Lars Brinkhoff <lars@nocrew.org>
#
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation; either version 2 of the License, or (at your
#   option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#   Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program. If not, see <http://www.gnu.org/licenses/>.
#
#   As a special exception, the respective Autoconf Macro's copyright owner
#   gives unlimited permission to copy, distribute and modify the configure
#   scripts that are the output of Autoconf when processing the Macro. You
#   need not follow the terms of the GNU General Public License when using
#   or distributing such scripts, even though portions of the text of the
#   Macro appear in them. The GNU General Public License (GPL) does govern
#   all other use of the material that constitutes the Autoconf Macro.
#
#   This special exception to the GPL applies to versions of the Autoconf
#   Macro released by the Autoconf Macro Archive. When you make and
#   distribute a modified version of the Autoconf Macro, you may extend this
#   special exception to the GPL to apply to your modified version as well.

AC_DEFUN([TYPE_SOCKLEN_T],
[AC_CACHE_CHECK([for socklen_t], ac_cv_type_socklen_t,
[
  AC_TRY_COMPILE(
  [#include <sys/types.h>
   #include <sys/socket.h>],
  [socklen_t len = 42; return 0;],
  ac_cv_type_socklen_t=yes,
  ac_cv_type_socklen_t=no)
])
  if test $ac_cv_type_socklen_t != yes; then
    AC_DEFINE(socklen_t, int, [Substitute for socklen_t])
  fi
])

