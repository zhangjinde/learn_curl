all: judge_client judged

judge_client: judge_client.cc
	g++ -Wall -c -I/usr/local/mysql/include/mysql -I/usr/include/mysql judge_client.cpp
	g++ -Wall -o judge_client judge_client.o -L/usr/local/mysql/lib/mysql -L/usr/lib/mysql -lmysqlclient
judged: judged.cc
	g++ -Wall -c -I/usr/local/mysql/include/mysql -I/usr/include/mysql  judged.cpp
	g++ -Wall -o judged judged.o -L/usr/local/mysql/lib/mysql -L/usr/lib/mysql  -lmysqlclient

clean:
	rm -rf judged judged.o judge_client.o judge_client
