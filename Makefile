CFLAGS += -lpthread -Wall

OBJS = notifier.o test_main.o

TARGET = notifier_test

.PHONY:all

all:clean $(TARGET)

$(TARGET):$(OBJS)
	gcc $(CFLAGS) $(OBJS) -o $@

.PHONY:clean
clean:
	rm -rf *.o $(OBJS) $(TARGET)
