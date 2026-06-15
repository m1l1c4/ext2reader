CXX = g++
CXXFLAGS = -Wall -g

TARGET = ext2_reader
SRCS = main.cpp superblock.cpp diskReader.cpp blockgroup.cpp inode.cpp directory.cpp
OBJS = $(SRCS:.cpp=.o)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)