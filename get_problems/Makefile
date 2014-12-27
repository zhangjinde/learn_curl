CC = gcc
CFLAGS = -Wall
SRCS = 
CURL = -L/usr/lib/x86_64-linux-gnu -lcurl
EKHTML = /usr/local/lib/libekhtml.a
MYSQLCFLAGS = -I/usr/local/mysql/include/mysql -I/usr/include/mysql
MYSQLLDFLAGS = -L/usr/local/mysql/lib/mysql -L/usr/lib/mysql  -lmysqlclient
LDFLAGS = $(CURL) $(EKHTML) $(MYSQLCFLAGS)
ALL = $(basename $(SRCS))

.PHONY: all clean $(DIRSALL) $(DIRSCLEAN)
all: $(DIRSALL) $(ALL)
ekhtml_tester ekhtml_parse_hdu: LDFLAGS = $(CURL) $(EKHTML)
$(CALL): %: %.o
	$(CC) $(CFLAGS) -o $@ $@.o $(LDFLAGS)
$(CPPALL): %: %.o
	$(CXX) $(CFLAGS) -o $@ $@.o $(LDFLAGS)
$(COBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $<
$(CPPOBJS): %.o: %.cpp
	$(CXX) $(CFLAGS) -c $<
$(DIRSALL): %:
	@cd $(subst all,,$@); make all;
$(DIRSCLEAN): %:
	@cd $(subst clean,,$@); make clean;
clean: $(DIRSCLEAN)
	$(RM) $(OBJS) $(ALL)
