CC = gcc
CXX = g++
CPPFLAGS = -fPIC -g3 -O3 -D_M_X64
#-g -D_MANAGER_SHARED
INCDIR = ../../include/zz_utils/micro_server
INCBASEDIR = ../../include
LIBDIR = ../../lib
INCLUDE = -I. -I$(INCBASEDIR) -I$(INCBASEDIR)/microhttpd \
	-I$(INCBASEDIR)/zz_utils/lwpr
LIBRARY = -L$(LIBDIR) -lbase64 -lmicrohttpd -llwpr -lmime -lpthread -lrt

OBJS = server.o config.o router.o types.o \
	cacheable_response.o response_renderer.o router_impl.o \
	safer_times.o server_impl.o service_framework.o
LIB = libmicroserver.a

$(LIB): $(OBJS)
	ar -r $(LIB) $(OBJS)
	ranlib $(LIB)
	-mv $(LIB) $(LIBDIR)
	-mkdir -p ../../include/zz_utils/micro_server
	-cp types.h $(INCDIR)
	-cp config.h $(INCDIR)
	-cp router.h $(INCDIR)
	-cp server.h $(INCDIR)
	-cp optional.hpp $(INCDIR)
	-cp service_framework.h $(INCDIR)
	-cp zz_server.h $(INCDIR)

.SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CPPFLAGS) $(INCLUDE) $(LIBRARY) -c $<

.PHONY: clean
clean:
	-rm *.o

