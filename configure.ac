#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([spada], [0.1], [deli.zhang@outlook.com])
AC_CONFIG_SRCDIR([src/config.h.in])
AC_CONFIG_HEADERS([src/config.h])
AC_CONFIG_AUX_DIR(bin)
AC_CONFIG_MACRO_DIR([acinclude])

AM_INIT_AUTOMAKE([-Wall -Wno-extra-portability -Werror foreign subdir-objects])
AC_SUBST([WARN_FLAGS], ['-Wall'])
CXXFLAGS='-g -O0 -fno-exceptions -fno-rtti'
#CXXFLAGS='-pg -O3 -fno-exceptions -fno-rtti'
#CXXFLAGS='-O3 -DNDEBUG -fno-exceptions -fno-rtti'
#AC_DEFINE(ENABLE_ASSERT, , 'Enable custom assert macros')
#AC_DEFINE(ENABLE_STATIC_ASSERT, , 'Enable custom static assert macros')
AC_DEFINE(ENABLE_PROFILER, 0, 'Enable non-blocking profiler')
GET_CACHE_LINESIZE=`getconf LEVEL1_DCACHE_LINESIZE`
AC_DEFINE_UNQUOTED([CACHE_LINE_SIZE], [$GET_CACHE_LINESIZE], ['Get D1 cache line size'])

LT_INIT

# Feature options

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_create], , [AC_MSG_FAILURE([
    ------------------------------------------
    pthread is required, stoping...
    ------------------------------------------])])

#AX_BOOST_BASE(1.37)
#if test "$HAVE_BOOST" = "no"; then
    #AC_MSG_FAILURE([
    #------------------------------------------
    #Boost 1.37 or above is required, stoping...
    #------------------------------------------])
#fi

#AC_CHECK_LIB([tbb], [main], , [AC_MSG_FAILURE([
    #------------------------------------------
    #Intel TBB is required, stoping...
    #------------------------------------------])])

#AC_CHECK_LIB([tcmalloc], [malloc], , [AC_MSG_FAILURE([
    #------------------------------------------
    #TCMalloc is required, stoping...
    #------------------------------------------])])

#AC_CHECK_LIB([m],[cos])
#AC_CHECK_LIB([gslcblas],[cblas_dgemm])
#AC_CHECK_LIB([gsl], [gsl_blas_dgemm], , [AC_MSG_FAILURE([
    #------------------------------------------
    #GSL is required, stoping...
    #------------------------------------------])])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([gettimeofday getrusage])
AC_CHECK_FUNCS(clock_gettime, [], [AC_CHECK_LIB(rt, clock_gettime, [AC_DEFINE([HAVE_CLOCK_GETTIME], [1], [Find clock_gettime]) LIBS="$LIBS -lrt"])])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/client/Makefile
                 src/common/Makefile
                 src/store/Makefile
                 src/server/Makefile])

AC_OUTPUT
