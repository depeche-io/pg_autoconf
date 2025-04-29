# contrib/auth_delay/Makefile

MODULES = autoconf

EXTENSION = autoconf

DATA = autoconf--1.0.sql

PGFILEDESC = "autoconf - automatic config tuning"

PG_CPPFLAGS = -I$(libpq_srcdir)
PG_LIBS_INTERNAL = $(libpq_pgport)
SHLIB_LINK_INTERNAL = $(libpq)

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
SHLIB_PREREQS = submake-libpq
subdir = contrib/autoconf
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk

override CPPFLAGS := -I$(libpq_srcdir) $(CPPFLAGS)
LDFLAGS_INTERNAL += $(libpq_pgport)

endif