# Makefile for Mandelbrot Viewer (OneDrive SFML build)
# Uses a visible recipe prefix to avoid tab problems

.RECIPEPREFIX := >
CXX := g++
SFML_ROOT := C:/Users/jordy/OneDrive/SFML-2.4.0-built

CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -I"$(SFML_ROOT)/include"
LDFLAGS  := -L"$(SFML_ROOT)/lib" -lsfml-graphics -lsfml-window -lsfml-system

TARGET := mandelbrot
SRC    := mandelbrot.cpp

.PHONY: all run clean dlls

all: $(TARGET)

$(TARGET): $(SRC)
> $(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

dlls:
> cp "$(SFML_ROOT)/bin/sfml-graphics-2.dll" .
> cp "$(SFML_ROOT)/bin/sfml-window-2.dll" .
> cp "$(SFML_ROOT)/bin/sfml-system-2.dll" .

run: all dlls
> ./$(TARGET)

clean:
> rm -f $(TARGET) sfml-graphics-2.dll sfml-window-2.dll sfml-system-2.dll


