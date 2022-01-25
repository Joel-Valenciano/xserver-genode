OBJS := core_handlers.o font_handlers.o xkb.o main.o

TARGET := xserver
CFLAGS += -fPIE -pthread

$(TARGET): $(OBJS)
	$(CC) -pthread -Wall -g -o $(TARGET) $(OBJS)

clean:
	rm -f *.o $(TARGET)
