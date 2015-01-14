CC = gcc
CXX = g++
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:%.cpp=%.o)
CURL = -L/usr/lib/x86_64-linux-gnu -lcurl
EKHTML = /usr/local/lib/libekhtml.a
MYSQLCFLAGS = -I/usr/local/mysql/include/mysql -I/usr/include/mysql
MYSQLLDFLAGS = -L/usr/local/mysql/lib/mysql -L/usr/lib/mysql  -lmysqlclient
JSONCFLAGS = -I/usr/local/include/json
JSONLDFLAGS = -L/usr/local/lib -ljson
LDFLAGS = $(CURL) $(EKHTML) $(MYSQLLDFLAGS) $(JSONLDFLAGS)
CFLAGS = -Wall $(JSONCFLAGS)
ALL = judge_client

.PHONY: all clean
all: $(ALL)

judge_client: $(OBJS)
	$(CXX) $(CFLAGS) -o judge_client $(OBJS) $(LDFLAGS)
$(OBJS): %.o: %.cpp
	$(CXX) $(CFLAGS) -c $<
clean:
	$(RM) $(OBJS) $(ALL)
