CFLAGS=-std=c++11 -g
LDFLAGS=-lpthread -ljsoncpp

SVRBIN=../bin/svr/ChatSvr
#SVRBIN=./Svr
CLIBIN=../bin/cli/Client

.PHONY:all
all:$(SVRBIN) $(CLIBIN)
$(SVRBIN):ChatServer.cpp
	g++ $^ -o $@ $(CFLAGS) $(LDFLAGS) 
$(CLIBIN):ChatClient.cpp
	g++ $^ -o $@ $(CFLAGS) $(LDFLAGS) -lncurses

.PHONY:clean
clean:
	rm -f $(SVRBIN) $(CLIBIN)
