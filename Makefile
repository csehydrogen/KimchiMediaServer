INCLUDE_DIR = /usr/local/include
INCLUDES = -I$(INCLUDE_DIR)/UsageEnvironment -I$(INCLUDE_DIR)/groupsock -I$(INCLUDE_DIR)/liveMedia -I$(INCLUDE_DIR)/BasicUsageEnvironment
COMPILE_OPTS = -c $(INCLUDES)
COMPILER = c++
COMPILE = $(COMPILER) $(COMPILE_OPTS)

LIB_DIR = /usr/local/lib
LIBS = -lBasicUsageEnvironment -lUsageEnvironment -lgroupsock -lliveMedia
LINK = c++

KimchiMediaServer : KimchiMediaServer.o
	$(LINK) -L$(LIB_DIR) -o $@ $< $(LIBS)

%.o : %.cpp
	$(COMPILE) $<

clean :
	rm -rf *.o KimchiMediaServer
