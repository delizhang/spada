SUBDIRS =

EXTRA_DIST = 

bin_PROGRAMS = spada

spada_SOURCES = clienthandler.cc\
				connector.cc\
				main.cc

spada_CPPFLAGS = $(WARN_FLAGS) -std=c++11 -pthread -I$(top_builddir)/src -I$(top_srcdir)/src

spada_LDADD = ../store/libtxd.la
