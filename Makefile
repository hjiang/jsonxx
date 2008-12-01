CXXFLAGS=-Werror -Wall

jsonxx_test: jsonxx_test.cc jsonxx.hh

test: jsonxx_test
	./jsonxx_test
