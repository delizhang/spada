SUBDIRS =

EXTRA_DIST = 

bin_PROGRAMS = spada

spada_SOURCES = main.cc \
				server.cc

spada_CPPFLAGS = $(WARN_FLAGS) -std=c++11 -pthread -I$(top_builddir)/src -I$(top_srcdir)/src

spada_LDADD = ../store/libtxd.la
