CXX = g++
CPPFLAGS = -fPIC -g3 -O3 -D_M_X64 -std=c++11
INCDIR = ../../include
LIBDIR = ../../lib

INCLUDE = -I. -I$(INCDIR) -I$(INCDIR)/zz_tools/lwpr -I$(INCDIR)/cpptoml -I$(INCDIR)/curl

LIBRARY = -L$(LIBDIR) \
	-lzz_tools_lwpr -lcurl -lcares -lssl -lcrypto \
	-lpthread -lz -ldl -lrt -lm -lresolv

mobile_vacant_prober:CollectMain.o GlobalRes.o CollectConfig.o \
		MobileVacantProber.o VacantProberTool.o HttpClient.o Cipher.o Base64.o
	$(CXX) $(CPPFLAGS) -o $@ \
		CollectMain.o GlobalRes.o CollectConfig.o \
		MobileVacantProber.o VacantProberTool.o HttpClient.o Cipher.o Base64.o $(LIB) $(LIBRARY)
	-mv mobile_vacant_prober ../bin

vacant_unicom_checker:VacantUnicomChecker.o GlobalRes.o CollectConfig.o \
		UnicomProber.o ChinaUnicomTools.o LarkBotUtils.o HttpClient.o Cipher.o Base64.o
	$(CXX) $(CPPFLAGS) -o $@ \
		VacantUnicomChecker.o GlobalRes.o CollectConfig.o \
		UnicomProber.o ChinaUnicomTools.o LarkBotUtils.o HttpClient.o Cipher.o Base64.o $(LIB) $(LIBRARY)
	-mv vacant_unicom_checker ../bin

uni_attr_chk:UnicomAttrChecker.o GlobalRes.o CollectConfig.o \
		UnicomAttrProber.o ChinaUnicomTools.o LarkBotUtils.o HttpClient.o Cipher.o Base64.o
	$(CXX) $(CPPFLAGS) -o $@ \
		UnicomAttrChecker.o GlobalRes.o CollectConfig.o \
		UnicomAttrProber.o ChinaUnicomTools.o LarkBotUtils.o HttpClient.o Cipher.o Base64.o $(LIB) $(LIBRARY)
	-mv uni_attr_chk ../bin

telecom_tool:TelecomToolTest.o GlobalRes.o CollectConfig.o \
		UnicomProber.o ChinaTelecomTools.o HttpClient.o Cipher.o Base64.o
	$(CXX) $(CPPFLAGS) -o $@ \
		TelecomToolTest.o GlobalRes.o CollectConfig.o \
		UnicomProber.o ChinaTelecomTools.o HttpClient.o Cipher.o Base64.o $(LIB) $(LIBRARY)
	-mv telecom_tool ../bin

rsa_tool:RsaToolTest.o Cipher.o
	$(CXX) $(CPPFLAGS) -o $@ RsaToolTest.o Cipher.o $(LIB) $(LIBRARY)
	-mv rsa_tool ../bin

.SUFFIXES: .cpp .o
.cpp.o:
	$(CXX) $(CPPFLAGS) -c $< -o $@ $(INCLUDE)

.PHONY: clean
clean:
	-rm -rf *.o
