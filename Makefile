KimchiMediaServer :
	cd src && \
	make && \
	mv KimchiMediaServer ..

clean :
	rm KimchiMediaServer && \
	cd src && \
	make clean
