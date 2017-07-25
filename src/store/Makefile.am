SUBDIRS =

EXTRA_DIST = 

lib_LTLIBRARIES = libtxd.la

pkginclude_HEADERS = container/txlist.h\
					 container/txlist.hh\
					 transaction/descriptor.h\
					 transaction/set.h

libtxd_la_SOURCES = transaction/descriptor.cc

libtxd_la_CPPFLAGS = $(WARN_FLAGS) -std=c++11 -I$(top_builddir)/src -I$(top_srcdir)/src

libtxd_la_LIBADD = ../common/libcmn.a
