CXX = g++
CPPFLAGS = -fPIC -g3 -O3 -D_M_X64 -std=c++11
INCDIR = ../../include
LIBDIR = ../../lib

INCLUDE = -I. -I$(INCDIR) -I$(INCDIR)/zz_tools/lwpr -I$(INCDIR)/cpptoml -I$(INCDIR)/curl

LIBRARY = -L$(LIBDIR) \
	-lzz_tools_lwpr -lcurl -lcares -lssl -lcrypto \
	-lpthread -lz -ldl -lrt -lm -lresolv

zz_collect:CollectMain.o GlobalRes.o CollectConfig.o \
		MobileVacantProber.o VacantProberTool.o HttpClient.o Cipher.o Base64.o
	$(CXX) $(CPPFLAGS) -o $@ \
		CollectMain.o GlobalRes.o CollectConfig.o \
		MobileVacantProber.o VacantProberTool.o HttpClient.o Cipher.o Base64.o $(LIB) $(LIBRARY)
	-mv zz_collect ../bin

migrate_checker:MigrateCheckerMain.o GlobalRes.o CollectConfig.o \
		UnicomProber.o ChinaTelecomTools.o ChinaUnicomTools.o LarkBotUtils.o HttpClient.o Cipher.o Base64.o
	$(CXX) $(CPPFLAGS) -o $@ \
		MigrateCheckerMain.o GlobalRes.o CollectConfig.o \
		UnicomProber.o ChinaTelecomTools.o ChinaUnicomTools.o LarkBotUtils.o HttpClient.o Cipher.o Base64.o $(LIB) $(LIBRARY)
	-mv migrate_checker ../bin

telecom_tool:TelecomToolTest.o GlobalRes.o CollectConfig.o \
		UnicomProber.o ChinaTelecomTools.o HttpClient.o Cipher.o Base64.o
	$(CXX) $(CPPFLAGS) -o $@ \
		TelecomToolTest.o GlobalRes.o CollectConfig.o \
		UnicomProber.o ChinaTelecomTools.o HttpClient.o Cipher.o Base64.o $(LIB) $(LIBRARY)
	-mv telecom_tool ../bin

.SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CPPFLAGS) -c $< -o $@ $(INCLUDE)

.PHONY: clean
clean:
	-rm -rf *.o
