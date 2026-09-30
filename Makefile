EXTENSION = pg_pii_audit
DATA = sql/pg_pii_audit--1.0.sql
MODULE_big = pg_pii_audit
OBJS = src/pg_pii_audit.o
PG_CPPFLAGS = -Wall -Wextra -Wno-unused-parameter

PG_CONFIG ?= pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
