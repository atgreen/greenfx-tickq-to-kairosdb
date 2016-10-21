bin_PROGRAMS = tickq-to-kairosdb

tickq_to_kairosdb_CPPFLAGS = $(AMQ_CFLAGS) -std=c++11

tickq_to_kairosdb_LDADD = -lcurl $(AMQ_LIBS) -ljson-c -lboost_date_time -lboost_system -lboost_thread -lpthread
tickq_to_kairosdb_SOURCES = main.cc
