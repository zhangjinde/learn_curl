all: judge_client judged

judge_client: judge_client.o copy_runtime.o
	g++ -Wall -o judge_client judge_client.o copy_runtime.o -L/usr/local/mysql/lib/mysql -L/usr/lib/mysql -lmysqlclient
judged: judged.o
	g++ -Wall -o judged judged.o -L/usr/local/mysql/lib/mysql -L/usr/lib/mysql  -lmysqlclient
judge_client.o: judge_client.cpp judge_client.h
	g++ -Wall -c -I/usr/local/mysql/include/mysql -I/usr/include/mysql judge_client.cpp
copy_runtime.o: copy_runtime.cpp
	g++ -Wall -c -I/usr/local/mysql/include/mysql -I/usr/include/mysql copy_runtime.cpp
judged.o:
	g++ -Wall -c -I/usr/local/mysql/include/mysql -I/usr/include/mysql  judged.cpp

clean:
	rm -rf judged judged.o judge_client.o judge_client

CC = gcc
CFLAGS = -Wall
SRCS = $(wildcard *.c)
OBJS = $(SRCS:%.c=%.o)
CURL = -L/usr/lib/x86_64-linux-gnu -lcurl
EKHTML = /usr/local/lib/libekhtml.a
MYSQLCFLAGS = -I/usr/local/mysql/include/mysql -I/usr/include/mysql
MYSQLLDFLAGS = -L/usr/local/mysql/lib/mysql -L/usr/lib/mysql  -lmysqlclient
LDFLAGS = $(CURL) $(EKHTML) $(MYSQLLDFLAGS)
ALL = get_problem

.PHONY: all clean
all: $(ALL)

get_problem: $(OBJS) main.h
	$(CC) $(CFLAGS) -o get_problem $(OBJS) $(LDFLAGS)
$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $<
clean:
	$(RM) $(OBJS) $(ALL)
