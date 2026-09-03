CXX ?= clang++
CXXFLAGS ?= -std=c++20 -O2 -g -Wall -Wextra -Wno-unused-parameter
LDFLAGS ?= -lpthread

SRC := src
BUILD := build
TARGET := laml

SOURCES := $(wildcard $(SRC)/*.cpp)
OBJECTS := $(patsubst $(SRC)/%.cpp, $(BUILD)/%.o, $(SOURCES))

.PHONY: all clean run test bench bench-http bench-ws bench-churn bench-slow

all: $(TARGET)

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: $(SRC)/%.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET) run $(FILE)

bench-http: $(TARGET)
	BENCH_PORT=18090 ./$(TARGET) run bench/bench_http_server.lm & \
	SRV=$$!; sleep 2; \
	python3 bench/bench_http.py 18090 20 100; \
	kill $$SRV; wait $$SRV 2>/dev/null; true

bench-ws: $(TARGET)
	BENCH_PORT=18091 ./$(TARGET) run bench/bench_chat.lm & \
	SRV=$$!; sleep 2; \
	python3 bench/bench_ws.py 18091 50 3; \
	kill $$SRV; wait $$SRV 2>/dev/null; true

bench-churn: $(TARGET)
	BENCH_PORT=18092 ./$(TARGET) run bench/bench_chat.lm & \
	SRV=$$!; sleep 2; \
	python3 bench/bench_churn.py 18092 100 10 $$SRV; \
	kill $$SRV; wait $$SRV 2>/dev/null; true

bench-slow: $(TARGET)
	BENCH_PORT=18093 ./$(TARGET) run bench/bench_chat.lm & \
	SRV=$$!; sleep 2; \
	python3 bench/bench_slow.py 18093 10 200; \
	kill $$SRV; wait $$SRV 2>/dev/null; true

bench: bench-http bench-ws bench-churn bench-slow

clean:
	rm -rf $(BUILD) $(TARGET)
