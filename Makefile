CPPFLAGS_DEBUG=-Wall -I. -std=c++11 -g
CPPFLAGS_RELEASE=-Wall -I. -std=c++11 -DNDEBUG -O3
CPPFLAGS=$(CPPFLAGS_DEBUG)
LDFLAGS=-Wall

HTTPD_NAME=exe/c11httpd
C11HTTPD_HEAD_FILES=$(wildcard c11httpd/*.h)
DAEMON_HEAD_FILES=$(wildcard main/*.h)
SOURCE_FILES=$(wildcard c11httpd/*.cpp daemon/*.cpp)
OBJECT_FILES=$(patsubst %.cpp,obj/%.o,$(SOURCE_FILES))

all: c11httpd

c11httpd: dirs $(OBJECT_FILES)
	g++ $(LDFLAGS) -o $(HTTPD_NAME) $(OBJECT_FILES)

obj/c11httpd/%.o: c11httpd/%.cpp $(C11HTTPD_HEAD_FILES)
	g++ $(CPPFLAGS) -c $< -o $@

obj/daemon/%.o: daemon/%.cpp $(C11HTTPD_HEAD_FILES) $(DAEMON_HEAD_FILES)
	g++ $(CPPFLAGS) -c $< -o $@

dirs:
	mkdir -p exe
	mkdir -p obj/c11httpd
	mkdir -p obj/daemon

clean:
	rm -rf exe obj

echo:
	@echo "CPPFLAGS:      "$(CPPFLAGS)
	@echo "LDFLAGS:       "$(LDFLAGS)
	@echo "HTTPD_NAME:    "$(HTTPD_NAME)
	@echo "SOURCE_FILES:  "$(SOURCE_FILES)
	@echo "OBJECT_FILES:  "$(OBJECT_FILES)


