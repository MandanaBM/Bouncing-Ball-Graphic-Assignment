TARGET=bouncy

SOURCES=$(wildcard *.cpp)

OBJECTS=$(SOURCES:.cpp=.o)

CXX=g++
CXXFLAGS=-O3 -g -std=c++11 -I/usr/local/include `pkg-config --cflags glew`
LDFLAGS= `pkg-config --static --libs glew` -framework OpenGL -framework GLUT

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(LDFLAGS) -o $(TARGET) $^

%.o: %.c
	$(CXX) $(CXXFLAGS) -o $@ -c $<


.PHONY: clean all
clean:
	rm -f $(OBJECTS)
	rm $(TARGET)