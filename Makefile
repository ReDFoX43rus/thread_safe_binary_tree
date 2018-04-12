CC=g++
CFLAGS=-c -Wall -std=c++11 -g
LDFLAGS=
LIBS=-lpthread

SRCDIR=src
OBJDIR=obj

SOURCES=$(wildcard $(SRCDIR)/*.cpp)
OBJECTS=$(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
EXEC=binary_tree

.SILENT:

all: $(SOURCES) $(EXEC)

$(EXEC): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@ $(LIBS)
	@echo LINK $@

$(OBJECTS): $(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -o $@ $^
	@echo C $^

clean:
	rm -rf $(OBJDIR)
	rm $(EXEC)
	@echo Cleaned

memcheck: $(ECEC)
	valgrind --leak-check=full --show-leak-kinds=all ./$(EXEC)
