CC= gcc
IDIR = bin
CFLAGS= -std=c++11 -Wall -I$(IDIR)
ODIR= bin

DEPS= serwerHTTP.h StreamRecord.h BeginRecord.h ConnectionManager.h\
Parser.h HTTPManager.h FCGIManager.h

OBJ= serwerHTTP.o StreamRecord.o BeginRecord.o ConnectionManager.o\
Parser.o HTTPManager.o FCGIManager.o

$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

serwerHTTP: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)
