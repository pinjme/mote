SHELL := /bin/bash
LIB := -L/usr/local/lib 
INC := -Ilib/inih/ -Ilib/inih/cpp/ -Isrc/ -Ilib/v8/include -Ilib/v8

all: moted

debug: CXX += -DDEBUG -g -O0
#debug: LIB += -L /root/v8/out/x64.debug/lib.target
debug: moted
	gdb -e ./moted -x ./lib/debug.gdb

# %.o: %.cpp
# 	$(CXX) -Wall -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H $(INC) -c $< -o $@
obj/%.o: src/%.cpp
	$(CXX) -Wall $(INC) -c $< -o $@


moted: obj/moted.o obj/V8Engine.o lib/inih/ini.c lib/inih/cpp/INIReader.cpp
	$(CXX) $^ -o $@ $(LIB) -Wl,-Bstatic -lv8_base -Wl,-Bdynamic -lrt 

hello: obj/hello.o
	$(CXX) $^ -o $@ $(LIB) -Wl,-Bstatic -lv8_base -lv8_libbase -lv8_snapshot -lv8_libplatform  -licui18n -licuuc -licudata  -Wl,-Bdynamic -lrt 

test: test1

test1: test/test1.cpp obj/V8Engine.o
	$(CXX) $(INC) $^ -o $@ $(LIB) -Wl,--start-group lib/v8/x64/{libv8_base,libv8_libbase,libv8_snapshot,libv8_libplatform,libicuuc,libicui18n,libicudata}.a -Wl,--end-group -lrt
#example: echo_server_tls.o 
#	$(CXX) $^ -o $@ $(LIB) -lboost_thread -lboost_system -levent -lssl -lcrypto

clean:
	$(RM) obj/*.o moted test1
