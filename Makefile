CC = gcc
CFLAGS = -Wall -Werror -g
INCLUDE = -Iinclude

SRCS = src/main.c src/transactions.c src/utils.c
OBJS = $(SRCS:.c=.o)
TARGET = bank

all: $(TARGET)

$(TARGET): $(OBJS)
    $(CC) $(CFLAGS) $(INCLUDE) -o $(TARGET) $(OBJS)

%.o: %.c
    $(CC) $(CFLAGS) $(INCLUDE) -c $< -o $@

clean:
    rm -f $(OBJS) $(TARGET)

.PHONY: all clean