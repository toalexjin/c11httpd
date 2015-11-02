CPPFLAGS_DEBUG=-Wall -I. -std=c++11 -g
CPPFLAGS_RELEASE=-Wall -I. -std=c++11 -DNDEBUG -O3
CPPFLAGS=$(CPPFLAGS_DEBUG)
LDFLAGS=-Wall -lrt -lpthread

C11HTTPD_LIB=obj/c11httpd.a
C11HTTPD_HEADERS=$(wildcard c11httpd/*.h)
C11HTTPD_SOURCES=$(wildcard c11httpd/*.cpp daemon/*.cpp)
C11HTTPD_OBJECTS=$(patsubst %.cpp,obj/%.o,$(C11HTTPD_SOURCES))

TESTTCP_EXE=exe/testtcp
TESTTCP_HEADERS=$(wildcard testtcp/*.h)
TESTTCP_SOURCES=$(wildcard testtcp/*.cpp)
TESTTCP_OBJECTS=$(patsubst %.cpp,obj/%.o,$(TESTTCP_SOURCES))

TESTHTTP_EXE=exe/testhttp
TESTHTTP_HEADERS=$(wildcard testhttp/*.h)
TESTHTTP_SOURCES=$(wildcard testhttp/*.cpp)
TESTHTTP_OBJECTS=$(patsubst %.cpp,obj/%.o,$(TESTHTTP_SOURCES))

all: c11httpd testtcp testhttp

c11httpd: c11httpd_dir $(C11HTTPD_OBJECTS)
	rm -f $(C11HTTPD_LIB)
	ar cr $(C11HTTPD_LIB) $(C11HTTPD_OBJECTS)

testtcp: c11httpd testtcp_dir $(TESTTCP_OBJECTS)
	rm -f $(TESTTCP_EXE)
	g++ $(LDFLAGS) -o $(TESTTCP_EXE) $(TESTTCP_OBJECTS) $(C11HTTPD_LIB)

testhttp: c11httpd testhttp_dir $(TESTHTTP_OBJECTS)
	rm -f $(TESTHTTP_EXE)
	g++ $(LDFLAGS) -o $(TESTHTTP_EXE) $(TESTHTTP_OBJECTS) $(C11HTTPD_LIB)

obj/c11httpd/%.o: c11httpd/%.cpp $(C11HTTPD_HEADERS)
	g++ $(CPPFLAGS) -c $< -o $@

obj/testtcp/%.o: testtcp/%.cpp $(C11HTTPD_HEADERS) $(TESTTCP_HEADERS)
	g++ $(CPPFLAGS) -c $< -o $@

obj/testhttp/%.o: testhttp/%.cpp $(C11HTTPD_HEADERS) $(TESTHTTP_HEADERS)
	g++ $(CPPFLAGS) -c $< -o $@

c11httpd_dir:
	mkdir -p obj/c11httpd

testtcp_dir:
	mkdir -p exe
	mkdir -p obj/testtcp

testhttp_dir:
	mkdir -p exe
	mkdir -p obj/testhttp

clean:
	rm -rf exe obj

echo:
	@echo "CPPFLAGS_DEBUG="$(CPPFLAGS_DEBUG)
	@echo "CPPFLAGS_RELEASE="$(CPPFLAGS_RELEASE)
	@echo "CPPFLAGS="$(CPPFLAGS)
	@echo "LDFLAGS="$(LDFLAGS)
	@echo "C11HTTPD_LIB="$(C11HTTPD_LIB)
	@echo "C11HTTPD_HEADERS="$(C11HTTPD_HEADERS)
	@echo "C11HTTPD_SOURCES="$(C11HTTPD_SOURCES)
	@echo "C11HTTPD_OBJECTS="$(C11HTTPD_OBJECTS)
	@echo "TESTTCP_EXE="$(TESTTCP_EXE)
	@echo "TESTTCP_HEADERS="$(TESTTCP_HEADERS)
	@echo "TESTTCP_SOURCES="$(TESTTCP_SOURCES)
	@echo "TESTTCP_OBJECTS="$(TESTTCP_OBJECTS)
	@echo "TESTHTTP_EXE="$(TESTHTTP_EXE)
	@echo "TESTHTTP_HEADERS="$(TESTHTTP_HEADERS)
	@echo "TESTHTTP_SOURCES="$(TESTHTTP_SOURCES)
	@echo "TESTHTTP_OBJECTS="$(TESTHTTP_OBJECTS)

