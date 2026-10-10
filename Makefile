CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude -O2
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:.c=.o)
TARGET = hospital_er.exe

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f src/*.o $(TARGET) test_runner.exe

test:
	$(CC) $(CFLAGS) -Iinclude src/patient.c src/triage.c src/priority_queue.c src/bed_manager.c src/file_io.c tests/test_priority_queue.c -o test_runner.exe
	./test_runner.exe

.PHONY: all clean test
