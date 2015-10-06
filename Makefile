CPPFLAGS=-Wall -DNDEBUG -O3 -I. -std=c++11
LDFLAGS=-Wall

HTTPD_NAME=exe/c11httpd
SOURCE_FILES=$(wildcard c11httpd/*.cpp server/*.cpp)
OBJECT_FILES=$(patsubst %.cpp,obj/%.o,$(SOURCE_FILES))


c11httpd: dirs $(OBJECT_FILES)
	g++ $(LDFLAGS) -o $(HTTPD_NAME) $(OBJECT_FILES)

obj/c11httpd/%.o: c11httpd/%.cpp
	g++ $(CPPFLAGS) -c $< -o $@

obj/server/%.o: server/%.cpp
	g++ $(CPPFLAGS) -c $< -o $@

dirs:
	mkdir -p exe
	mkdir -p obj/c11httpd
	mkdir -p obj/server

clean:
	rm -rf exe obj

echo:
	@echo "CPPFLAGS:      "$(CPPFLAGS)
	@echo "LDFLAGS:       "$(LDFLAGS)
	@echo "HTTPD_NAME:    "$(HTTPD_NAME)
	@echo "SOURCE_FILES:  "$(SOURCE_FILES)
	@echo "OBJECT_FILES:  "$(OBJECT_FILES)


