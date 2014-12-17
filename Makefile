export CC = gcc
export CXX = g++
export CFLAGS = -Wall
export CXXFLAGS = -Wall
DIRS = $(shell ls -F | grep /)
DIRSALL = $(DIRS:%/=%all)
DIRSCLEAN = $(DIRS:%/=%clean)
CSRCS = $(wildcard *.c)
COBJS = $(CSRCS:%.c=%.o)
CPPSRCS = $(wildcard *.cpp)
CPPOBJS = $(CPPSRCS:%.cpp=%.o)
CURL = -L/usr/lib/x86_64-linux-gnu -lcurl
LDFLAGS = $(CURL)
OBJS = $(COBJS) $(CPPOBJS)
SRCS = $(CSRCS) $(CPPSRCS)
ALL = $(basename $(SRCS))

.PHONY: all clean $(DIRSALL) $(DIRSCLEAN)
all: $(DIRSALL) $(ALL)
$(ALL): %: %.o
	$(CC) $(CFLAGS) -o $@ $@.o $(LDFLAGS)
$(OBJS): %.o: %.c
	$(CC) $(CFLAGS) -c $<
$(DIRSALL): %:
	@cd $(subst all,,$@); make all;
$(DIRSCLEAN): %:
	@cd $(subst clean,,$@); make clean;
clean: $(DIRSCLEAN)
	$(RM) $(OBJS) $(ALL)
