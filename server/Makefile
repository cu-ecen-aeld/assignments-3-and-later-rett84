
TARGET = aesdsocket
OBJS = aesdsocket.c
CFLAGS = -lpthread

all: $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(TARGET) $(LDFLAGS)


default: $(OBJS)
	$(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f *.o aesdsocket

