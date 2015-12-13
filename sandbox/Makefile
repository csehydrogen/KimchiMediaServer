COMPILE_OPTS = -std=c++11 -c -g
COMPILER = c++
COMPILE = $(COMPILER) $(COMPILE_OPTS)

LIBS = RTSPServer.o RTSPParser.o RTSPRequest.o BufferedReader.o MPEG2TS.o
LINK = c++

KimchiMediaServer : main.o $(LIBS)
	$(LINK) -o $@ $^

%.o : %.cpp
	$(COMPILE) $<

clean :
	rm -rf *.o KimchiMediaServer
