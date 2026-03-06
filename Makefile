CC = g++
FLAGS = -I ./
SRCS = kvstore.cpp epoll.cpp kvstore_array.cpp
TARGET = kvstore

$(TARGET): 
	$(CC) -o $(TARGET) $(SRCS) $(FLAGS)

clean:
	rm kvstore