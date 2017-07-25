SUBDIRS =

EXTRA_DIST = 

noinst_LIBRARIES = libcmn.a

libcmn_a_SOURCES = utils/assert.cc\
					utils/barrier.cc\
					utils/profiler.cc\
					utils/timer.cc

libcmn_a_CPPFLAGS = $(WARN_FLAGS) -std=c++11 -I$(top_builddir)/src -I$(top_srcdir)/src
