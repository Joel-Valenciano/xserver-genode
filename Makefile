OBJS := core_handlers.o font_handlers.o xkb.o main.o

TARGET := xserver

$(TARGET): $(OBJS)
	$(CC) -Wall -g -o $(TARGET) $(OBJS) 

clean:
	rm -f *.o $(TARGET)