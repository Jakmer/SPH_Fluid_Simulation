# Makefile for your SFML C++ project

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -Wall

# SFML libraries
SFML_LIBS = -lsfml-graphics -lsfml-window -lsfml-system

# Source files and output executable
SRCS = main.cpp
TARGET = main

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(SFML_LIBS)

clean:
	rm -f $(TARGET)
