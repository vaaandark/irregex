CC=gcc
#SANITIZE=-fsanitize=address
CFLAGS=-Wall -Wextra -Werror -Wshadow $(SANITIZE) -g -O0
SOURCEDIR=src
BUILDDIR=build
OBJECTS=$(BUILDDIR)/re.o $(BUILDDIR)/NFA.o $(BUILDDIR)/main.o

$(BUILDDIR)/regex: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(BUILDDIR)/regex

$(BUILDDIR)/re.o: $(SOURCEDIR)/def.h
	$(CC) -c $(SOURCEDIR)/re.c -o $(BUILDDIR)/re.o

$(BUILDDIR)/NFA.o: $(SOURCEDIR)/re.h
	$(CC) -c $(SOURCEDIR)/NFA.c -o $(BUILDDIR)/NFA.o

$(BUILDDIR)/main.o: $(SOURCEDIR)/NFA.h
	$(CC) -c $(SOURCEDIR)/main.c -o $(BUILDDIR)/main.o

.PHONY: clean
clean:
	-rm -rf $(BUILDDIR)/regex $(OBJECTS)

