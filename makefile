CC=clang
#SANITIZE=-fsanitize=address
CFLAGS=-std=c99 -Wall -Wextra -Werror -Wshadow $(SANITIZE) -g -O2
SOURCEDIR=src
BUILDDIR=build
OBJECTS=$(BUILDDIR)/re.o $(BUILDDIR)/NFA.o $(BUILDDIR)/irregex.o

all: $(OBJECTS) $(BUILDDIR)/main.o $(BUILDDIR)/re2tree.o $(BUILDDIR)/re2graph.o $(BUILDDIR)/lexer.o
	$(CC) $(CFLAGS) $(OBJECTS) $(BUILDDIR)/main.o -o $(BUILDDIR)/irregex
	$(CC) $(CFLAGS) $(OBJECTS) $(BUILDDIR)/re2tree.o -o $(BUILDDIR)/re2tree
	$(CC) $(CFLAGS) $(OBJECTS) $(BUILDDIR)/re2graph.o -o $(BUILDDIR)/re2graph
	$(CC) $(CFLAGS) $(OBJECTS) $(BUILDDIR)/lexer.o -o $(BUILDDIR)/lexer

$(BUILDDIR)/re.o: $(SOURCEDIR)/re.c $(SOURCEDIR)/def.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/NFA.o: $(SOURCEDIR)/NFA.c $(SOURCEDIR)/re.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/irregex.o: $(SOURCEDIR)/irregex.c $(SOURCEDIR)/NFA.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/main.o: $(SOURCEDIR)/main.c $(SOURCEDIR)/irregex.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/re2tree.o: $(SOURCEDIR)/re2tree.c $(SOURCEDIR)/NFA.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/re2graph.o: $(SOURCEDIR)/re2graph.c $(SOURCEDIR)/NFA.h
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR)/lexer.o: $(SOURCEDIR)/lexer.c $(SOURCEDIR)/irregex.h
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	-rm -rf $(OBJECTS) \
		$(BUILDDIR)/re2graph.o $(BUILDDIR)/re2tree.o $(BUILDDIR)/irregex.o \
		$(BUILDDIR)/lexer.o $(BUILDDIR)/main.o $(BUILDDIR)/irregex $(BUILDDIR)/re2graph \
		$(BUILDDIR)/re2tree $(BUILDDIR)/lexer

