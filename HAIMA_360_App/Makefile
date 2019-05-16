OBJS = main.o imx6_com.o tsh_comm.o upgrade.o queue.o
TARGET = t6a_app
LIBS +=   -lpthread -L.  -lcommif   
LDFLAGS += -Wl,-rpath,.:/home/tsh/lib
$(TARGET): $(OBJS)
	$(CC)  $(CFLAGS) $(LDFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
clean:
	rm -f *.o *.out 

