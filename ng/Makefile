CXX ?= clang++
CXXFLAGS ?= -std=c++20 -O2 -g -Wall -Wextra -Wno-unused-parameter
LDFLAGS ?= -lpthread

SRC := src
BUILD := build
TARGET := laml

SOURCES := $(wildcard $(SRC)/*.cpp)
OBJECTS := $(patsubst $(SRC)/%.cpp, $(BUILD)/%.o, $(SOURCES))

.PHONY: all clean run test

all: $(TARGET)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: $(SRC)/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET) run $(FILE)

clean:
	rm -rf $(BUILD) $(TARGET)
