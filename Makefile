CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -march=native -flto
LIBS = -lncurses -ldl
SRCDIR = src
BUILDDIR = build
OBJDIR = $(BUILDDIR)/obj
TARGET = $(BUILDDIR)/editor


SRCS = $(wildcard $(SRCDIR)/*.c)
OBJS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SRCS))


$(shell mkdir -p $(OBJDIR))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@


$(OBJDIR)/main.o: $(SRCDIR)/main.c $(SRCDIR)/editor.h $(SRCDIR)/plugin.h
$(OBJDIR)/editor.o: $(SRCDIR)/editor.c $(SRCDIR)/editor.h
$(OBJDIR)/file_io.o: $(SRCDIR)/file_io.c $(SRCDIR)/editor.h $(SRCDIR)/plugin.h
$(OBJDIR)/input_handling.o: $(SRCDIR)/input_handling.c $(SRCDIR)/editor.h
$(OBJDIR)/rendering.o: $(SRCDIR)/rendering.c $(SRCDIR)/editor.h
$(OBJDIR)/selection.o: $(SRCDIR)/selection.c $(SRCDIR)/editor.h
$(OBJDIR)/syntax.o: $(SRCDIR)/syntax.c $(SRCDIR)/editor.h
$(OBJDIR)/plugin.o: $(SRCDIR)/plugin.c $(SRCDIR)/plugin.h $(SRCDIR)/editor.h

clean:
	rm -rf $(BUILDDIR)
	rm -f *.tmp *.bak *.backup *.autosave *.emergency /tmp/kilo_editor_clipboard.txt cursor_x cursor_y dirty line_count lines* select_* 2>/dev/null || true
	find . -name "*.o" -delete
	find . -name "editor" -delete
	find . -name "*.tmp" -delete
	find . -name "*.bak" -delete
	find . -name "*.backup" -delete
	find . -name "*.autosave" -delete
	find . -name "*.emergency" -delete
	find . -name "cursor_x" -delete
	find . -name "cursor_y" -delete
	find . -name "dirty" -delete
	find . -name "line_count" -delete
	find . -name "lines*" -delete
	find . -name "select_*" -delete
	find . -name "*[*]*" -delete


.PHONY: all clean