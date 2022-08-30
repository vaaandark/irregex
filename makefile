CC=gcc
SANITIZE=-fsanitize=address
CFLAGS=-Wall -Wextra -Werror -Wshadow $(SANITIZE) -g -O0
SOURCEDIR=src
BUILDDIR=build
OBJECTS=$(BUILDDIR)/re.o $(BUILDDIR)/NFA.o

all: $(OBJECTS) $(BUILDDIR)/re2tree.o $(BUILDDIR)/re2graph.o
	$(CC) $(CFLAGS) $(OBJECTS) $(BUILDDIR)/re2tree.o -o $(BUILDDIR)/re2tree
	$(CC) $(CFLAGS) $(OBJECTS) $(BUILDDIR)/re2graph.o -o $(BUILDDIR)/re2graph

$(BUILDDIR)/re.o: $(SOURCEDIR)/def.h
	$(CC) $(CFLAGS) -c $(SOURCEDIR)/re.c -o $(BUILDDIR)/re.o

$(BUILDDIR)/NFA.o: $(SOURCEDIR)/re.h
	$(CC) $(CFLAGS) -c $(SOURCEDIR)/NFA.c -o $(BUILDDIR)/NFA.o

$(BUILDDIR)/re2tree.o: $(SOURCEDIR)/NFA.h
	$(CC) $(CFLAGS) -c $(SOURCEDIR)/re2tree.c -o $(BUILDDIR)/re2tree.o

$(BUILDDIR)/re2graph.o: $(SOURCEDIR)/NFA.h
	$(CC) $(CFLAGS) -c $(SOURCEDIR)/re2graph.c -o $(BUILDDIR)/re2graph.o

.PHONY: clean
clean:
	-rm -rf $(OBJECTS) $(BUILDDIR)re2graph.o $(BUILDDIR)/re2tree.o $(BUILDDIR)/re2graph $(BUILDDIR)/re2tree

