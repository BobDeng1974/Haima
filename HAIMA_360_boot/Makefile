OBJS = main.o queue.o  upgrade.o
TARGET = 360_boot
LIBS +=   -lpthread 
$(TARGET): $(OBJS)
	$(CC)  $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
clean:
	rm -f *.o *.out 360_boot

