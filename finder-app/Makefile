
all: writer.c
	#$(CROSS_COMPILE)gcc -g -Wall writer.c -o writer
   # $(CROSS_COMPILE)$(CC) -g -Wall writer.c -o writer
    $(CC) $(CFLAGS) $(INCLUDES) $(OBJS) -o $(TARGET) $(LDFLAGS)
#file writer.o > ../assignments/assignment2/fileresult.txt

clean:
	rm -f *.o writer

