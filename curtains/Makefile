CXX = g++
CXXFLAGS = -std=c++11 -Wall
LDFLAGS = -lGL -lGLEW -lglfw -lm

TARGET = curtain
SOURCES = curtain.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run
