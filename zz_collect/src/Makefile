CXX = g++
CPPFLAGS = -fPIC -g3 -O3 -D_M_X64 -std=c++11 #-std=c++17 
INCDIR = ../../include
LIBDIR = ../../lib

INCLUDE = -I. -I$(INCDIR) -I$(INCDIR)/zz_tools/lwpr -I$(INCDIR)/cpptoml -I$(INCDIR)/curl

LIBRARY = -L$(LIBDIR) \
	-lzz_tools_lwpr -lcurl -lcares -lssl -lcrypto \
	-lpthread -lz -ldl -lrt -lm -lresolv
#-lssl -lcrypto
zz_collect:CollectMain.o GlobalRes.o CollectConfig.o \
		MobileVacantProber.o VacantProberTool.o HttpClient.o Cipher.o Base64.o
	$(CXX) $(CPPFLAGS) -o $@ \
		CollectMain.o GlobalRes.o CollectConfig.o \
		MobileVacantProber.o VacantProberTool.o HttpClient.o Cipher.o Base64.o $(LIB) $(LIBRARY)
	-mv zz_collect ../bin/

.SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CPPFLAGS) -c $< -o $@ $(INCLUDE)

.PHONY: clean
clean:
	-rm -rf *.o
