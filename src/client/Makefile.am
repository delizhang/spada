SUBDIRS =

EXTRA_DIST = 

lib_LTLIBRARIES = libspada-client.la

pkginclude_HEADERS =

libspada_client_la_SOURCES = connector.cc

libspada_client_la_CPPFLAGS = $(WARN_FLAGS) -std=c++11 -I$(top_builddir)/src -I$(top_srcdir)/src

libspada_client_la_LIBADD = ../common/libcmn.a


bin_PROGRAMS = spada-client-cli

spada_client_cli_SOURCES = cli/main.cc

spada_client_cli_CPPFLAGS = $(WARN_FLAGS) -std=c++11 -pthread -I$(top_builddir)/src -I$(top_srcdir)/src

spada_client_cli_LDADD = libspada-client.la
