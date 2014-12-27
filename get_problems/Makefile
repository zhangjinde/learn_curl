CC = gcc
CFLAGS = -Wall
SRCS = $(wildcard *.c)
OBJS = $(SRCS:*.c=*.o)
BINS = $(basename $(SRCS))
CURL = -L/usr/lib/x86_64-linux-gnu -lcurl
EKHTML = /usr/local/lib/libekhtml.a
MYSQLCFLAGS = -I/usr/local/mysql/include/mysql -I/usr/include/mysql
MYSQLLDFLAGS = -L/usr/local/mysql/lib/mysql -L/usr/lib/mysql  -lmysqlclient
LDFLAGS = $(CURL) $(EKHTML) $(MYSQLCFLAGS)
ALL = get_problem

.PHONY: all clean
all: get_problem

get_problem: $(OBJS)
	$(CC) $(CFLAGS) -o get_problem $(OBJS)
$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $<
clean:
	$(RM) $(OBJS) $(ALL)
