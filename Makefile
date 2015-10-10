CPPFLAGS_DEBUG=-Wall -I. -std=c++11 -g
CPPFLAGS_RELEASE=-Wall -I. -std=c++11 -DNDEBUG -O3
CPPFLAGS=$(CPPFLAGS_DEBUG)
LDFLAGS=-Wall

C11HTTPD_EXE=exe/c11httpd
C11HTTPD_HEADERS=$(wildcard c11httpd/*.h)
DAEMON_HEADERS=$(wildcard main/*.h)
SOURCE_FILES=$(wildcard c11httpd/*.cpp daemon/*.cpp)
OBJECT_FILES=$(patsubst %.cpp,obj/%.o,$(SOURCE_FILES))

all: c11httpd

c11httpd: dirs $(OBJECT_FILES)
	g++ $(LDFLAGS) -o $(C11HTTPD_EXE) $(OBJECT_FILES)

obj/c11httpd/%.o: c11httpd/%.cpp $(C11HTTPD_HEADERS)
	g++ $(CPPFLAGS) -c $< -o $@

obj/daemon/%.o: daemon/%.cpp $(C11HTTPD_HEADERS) $(DAEMON_HEADERS)
	g++ $(CPPFLAGS) -c $< -o $@

dirs:
	mkdir -p exe
	mkdir -p obj/c11httpd
	mkdir -p obj/daemon

clean:
	rm -rf exe obj

echo:
	@echo "CPPFLAGS_DEBUG="$(CPPFLAGS_DEBUG)
	@echo "CPPFLAGS_RELEASE="$(CPPFLAGS_RELEASE)
	@echo "CPPFLAGS="$(CPPFLAGS)
	@echo "LDFLAGS="$(LDFLAGS)
	@echo "C11HTTPD_EXE="$(C11HTTPD_EXE)
	@echo "C11HTTPD_HEADERS="$(C11HTTPD_HEADERS)
	@echo "DAEMON_HEADERS="$(DAEMON_HEADERS)
	@echo "SOURCE_FILES="$(SOURCE_FILES)
	@echo "OBJECT_FILES="$(OBJECT_FILES)


