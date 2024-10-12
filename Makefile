
TARGET = server
OBJS = aesdsocket.c

all: $(OBJS)
	$(CROSS_COMPILE)gcc -g -Wall $(OBJS) -o $(TARGET)


default: $(OBJS)
	$(CROSS_COMPILE)gcc -g -Wall $(OBJS) -o $(TARGET)

clean:
	rm -f *.o server

