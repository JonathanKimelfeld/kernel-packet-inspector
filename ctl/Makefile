CC = gcc
CFLAGS = -Wall -I../include
TARGET = packet_ctl

all: $(TARGET)

$(TARGET): packet_ctl.c
	$(CC) $(CFLAGS) -o $(TARGET) packet_ctl.c

clean:
	rm -f $(TARGET)
