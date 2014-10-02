
LIB := -L/usr/local/lib 
INC := -I/usr/local/include

all: test

# %.o: %.cpp
# 	$(CXX) -Wall -DHAVE_INTTYPES_H -DHAVE_NETINET_IN_H $(INC) -c $< -o $@
%.o: %.cpp $(DEP_HEAD)
	$(CXX) -Wall $(INC) -c $< -o $@


test: test.o 
	$(CXX) $^ -o $@ $(LIB) -lv8


example: echo_server_tls.o 
	$(CXX) $^ -o $@ $(LIB) -lboost_thread -lboost_system -levent -lssl -lcrypto

clean:
	$(RM) *.o test
