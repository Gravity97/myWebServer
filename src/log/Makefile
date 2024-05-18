# 指定编译器
CXX = g++

# 指定编译选项
CXXFLAGS = -std=c++14 -O2 -Wall -g

# 指定源文件
SRC = blockdeque.cpp

# 指定目标文件
TARGET = BlockDequeTest

# 默认目标
all: $(TARGET)

# 编译规则
$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

# 清理规则
clean:
	rm -f $(TARGET)