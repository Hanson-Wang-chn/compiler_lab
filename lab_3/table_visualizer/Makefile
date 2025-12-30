# ACTION/GOTO表可视化工具Makefile
# 用于编译和运行分析表可视化工具

CXX = g++
CXXFLAGS = -std=c++11 -Wall -Wextra

TARGET = table_visualizer
SRC = table_visualizer.cpp

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
