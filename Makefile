
LIB := -L/usr/local/lib 
INC := -I/usr/local/include -Ilib/inih/ -Ilib/inih/cpp/ -Ilib/ -Isrc/

all: moted

# %.o: %.cpp
# 	$(CXX) -Wall -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H $(INC) -c $< -o $@
%.o: src/%.cpp
	$(CXX) -Wall $(INC) -c $< -o obj/$@


moted: obj/moted.o lib/inih/ini.c lib/inih/cpp/INIReader.cpp
	$(CXX) $^ -o $@ $(LIB) -lv8


#example: echo_server_tls.o 
#	$(CXX) $^ -o $@ $(LIB) -lboost_thread -lboost_system -levent -lssl -lcrypto

clean:
	$(RM) obj/*.o moted
