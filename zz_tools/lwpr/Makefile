CXX = g++
CPPFLAGS = -fPIC -g3 -O3 -DLINUX -D_M_X64 # -D_MANAGER_SHARED
INCDIR = .
INCBASEDIR = ../../include
LIBDIR = .
INSDIR = ../..
INCLUDE = -I$(INCDIR) -I$(INCBASEDIR) -I$(INCBASEDIR)/base64
LIBRARY = -L$(LIBDIR) -lbase64 -lpthread -lrt -lresolv -ldl

OBJS = AtomicBoolean.o AtomicInteger.o AtomicInt64.o Buffer.o InputStream.o IPCMSQ.o IPCSEM.o IPCSHM.o Logger.o \
	Object.o OutputStream.o Socket.o StringUtil.o Synchronized.o Utility.o CFile.o FileUtil.o \
	StringBuffer.o ProgramOption.o ProcessUtil.o Thread.o RWMutex.o Mutex.o Semph.o EventMsqDispatch.o \
	SimpleProcessPool.o MTNetServer.o STNetServer.o NetServer.o Resource.o ThreadPool.o TimerThread.o ConfigProperty.o \
	ThreadLocal.o ProcessInfo.o cJSON.o UUID.o
LIB = libzz_tools_lwpr.a

$(LIB) : $(OBJS)
	ar -r $(LIB) $(OBJS) $@
	-mv $(LIB) $(INSDIR)/lib
	-mkdir -p $(INSDIR)/include/zz_tools/lwpr
	-cp *.h $(INSDIR)/include/zz_tools/lwpr

.SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CPPFLAGS) $(INCLUDE) $(LIBRARY) -c $<

.PHONY: clean
clean:
	-rm *.o
