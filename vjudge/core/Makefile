CC = gcc
CFLAGS = -Wall
SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)
CURL = -L/usr/lib/x86_64-linux-gnu -lcurl
EKHTML = /usr/local/lib/libekhtml.a
MYSQLCFLAGS = -I/usr/local/mysql/include/mysql -I/usr/include/mysql
MYSQLLDFLAGS = -L/usr/local/mysql/lib/mysql -L/usr/lib/mysql  -lmysqlclient
LDFLAGS = $(CURL) $(EKHTML) $(MYSQLLDFLAGS)
ALL = judge_client

.PHONY: all clean
all: $(ALL)

judge_client: $(OBJS) main.h
	$(CC) $(CFLAGS) -o judge_client $(OBJS) $(LDFLAGS)
$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $<
clean:
	$(RM) $(OBJS) $(ALL)
