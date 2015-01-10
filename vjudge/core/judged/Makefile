all: judged
judged:
	gcc -Wall -c -I/usr/local/mysql/include/mysql -I/usr/include/mysql  judged.c
	gcc -Wall -o judged judged.o -L/usr/local/mysql/lib/mysql -L/usr/lib/mysql  -lmysqlclient
clean:
	rm judged.o judged
