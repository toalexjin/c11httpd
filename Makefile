CPPFLAGS=-Wall -DNDEBUG -O3 -I. -std=c++11
LDFLAGS=-Wall

HTTPD_NAME=exe/c11httpd
SOURCE_FILES=$(wildcard c11httpd/*.cpp server/*.cpp)
OBJECT_FILES=$(patsubst %.cpp,obj/%.o,$(SOURCE_FILES))


c11httpd: $(OBJECT_FILES)
	mkdir -p exe
	g++ $(LDFLAGS) -o $(HTTPD_NAME) $^

obj/c11httpd/%.o: c11httpd/%.cpp
	mkdir -p obj/c11httpd
	g++ $(CPPFLAGS) -c $< -o $@

obj/server/%.o: server/%.cpp
	mkdir -p obj/server
	g++ $(CPPFLAGS) -c $< -o $@

clean:
	rm -rf exe obj

echo:
	@echo "CPPFLAGS:      "$(CPPFLAGS)
	@echo "LDFLAGS:       "$(LDFLAGS)
	@echo "HTTPD_NAME:    "$(HTTPD_NAME)
	@echo "SOURCE_FILES:  "$(SOURCE_FILES)
	@echo "OBJECT_FILES:  "$(OBJECT_FILES)


