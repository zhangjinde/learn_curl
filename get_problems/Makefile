CC = gcc
SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)
CURL = -L/usr/lib/x86_64-linux-gnu -lcurl
EKHTML = /usr/local/lib/libekhtml.a
MYSQLCFLAGS = -I/usr/local/mysql/include/mysql -I/usr/include/mysql
MYSQLLDFLAGS = -L/usr/local/mysql/lib/mysql -L/usr/lib/mysql  -lmysqlclient
JSONCFLAGS = $(shell pkg-config --cflags json-c)
JSONLDFLAGS = $(shell pkg-config --libs json-c)
LDFLAGS = $(CURL) $(EKHTML) $(MYSQLLDFLAGS) $(JSONLDFLAGS)
CFLAGS = -Wall $(JSONCFLAGS)
ALL = get_problem

.PHONY: all clean
all: $(ALL)

get_problem: $(OBJS)
	$(CC) $(CFLAGS) -o get_problem $(OBJS) $(LDFLAGS)
$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $<
clean:
	$(RM) $(OBJS) $(ALL)
