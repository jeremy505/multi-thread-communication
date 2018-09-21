CC = gcc -std=c99 -lpthread
TARGET = pipe pipe_1 pipe_1_d socket_pair

all:$(TARGET)

%.o:%.c

.PHONY:clean

clean:
	find . -name "*.0" | xargs rm -rf
	find . -name "*~" | xargs rm -rf
	find . -name "*#" | xargs rm -rf
	rm -rf $(TARGET)
