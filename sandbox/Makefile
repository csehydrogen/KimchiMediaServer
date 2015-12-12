COMPILE_OPTS = -c
COMPILER = c++
COMPILE = $(COMPILER) $(COMPILE_OPTS)

LIBS = RTSPServer.o RTSPParser.o
LINK = c++

KimchiMediaServer : main.o $(LIBS)
	$(LINK) -o $@ $^

%.o : %.cpp
	$(COMPILE) $<

clean :
	rm -rf *.o KimchiMediaServer
