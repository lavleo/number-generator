CXX      = x86_64-w64-mingw32-g++
CXXFLAGS = -Wall -Wextra -std=c++11 -mwindows -DUNICODE -D_UNICODE
LDFLAGS  = -static -static-libgcc -static-libstdc++ -lcomdlg32
TARGET   = number-generator.exe
SOURCES  = main.cpp
OBJECTS  = $(SOURCES:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJECTS) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean