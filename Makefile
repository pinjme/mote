
LIB := -L/usr/local/lib 
INC := -I/usr/local/include -Iinih/ -Iinih/cpp/

all: moted

# %.o: %.cpp
# 	$(CXX) -Wall -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H $(INC) -c $< -o $@
%.o: %.cpp
	$(CXX) -Wall $(INC) -c $< -o $@


moted: moted.o inih/ini.c inih/cpp/INIReader.cpp
	$(CXX) $^ -o $@ $(LIB) -lv8


#example: echo_server_tls.o 
#	$(CXX) $^ -o $@ $(LIB) -lboost_thread -lboost_system -levent -lssl -lcrypto

clean:
	$(RM) *.o moted
