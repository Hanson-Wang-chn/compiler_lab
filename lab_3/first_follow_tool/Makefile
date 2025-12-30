# FIRST/FOLLOW集计算工具Makefile
# 用于编译和运行FIRST/FOLLOW集输出工具

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

TARGET = first_follow
SRC = first_follow.cpp

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
