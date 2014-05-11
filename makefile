# compiler
CC = $(CXX)

# compile-time flags
# CFLAGS = -Wall -g
CFLAGS = -g -std=c++0x
PTHREAD = -pthread -lrt

# path to compiled boost library (necessary only if it isn't in compilers include path)
#BOOST_PATH = /usr/local/include/boost
#BOOST_HEADERS = $(BOOST_PATH)/include
#BOOST_LIBS = $(BOOST_PATH)/../../lib
BOOST_HEADERS = /usr/local/include/boost
BOOST_LIBS = /usr/local/lib

BOOST_INCLUDE = -I $(BOOST_HEADERS) -L $(BOOST_LIBS)

BOOST_SYSTEM = $(BOOST_LIBS)/libboost_system.a
BOOST_SERIALIZATION = $(BOOST_LIBS)/libboost_serialization.a
BOOST_THREAD = $(BOOST_LIBS)/libboost_thread.a

BOOST_ARCHIVE = $(BOOST_LIBS)/archive


# match all *.cc files and make *.o files
OBJS = $(patsubst %.cc, %.o, $(wildcard *.cc))

# program name
TARGET = main

bin: $(TARGET)

$(OBJS): %.o: %.cc
	$(CC) -c $< $(CFLAGS) $(BOOST_INCLUDE) -o $@

main: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(BOOST_INCLUDE) $(BOOST_SYSTEM) $(BOOST_SERIALIZATION) $(BOOST_THREAD) $(PTHREAD) -o $@

clean:
	$(RM) $(OBJS) $(OBJSC) $(TARGET)

