CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LIBS = -lncurses
OBJS = main.o editor.o file_io.o input_handling.o rendering.o selection.o syntax.o
TARGET = editor



all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

main.o: main.c editor.h
	$(CC) $(CFLAGS) -c main.c

editor.o: editor.c editor.h
	$(CC) $(CFLAGS) -c editor.c

file_io.o: file_io.c editor.h
	$(CC) $(CFLAGS) -c file_io.c

input_handling.o: input_handling.c editor.h
	$(CC) $(CFLAGS) -c input_handling.c

rendering.o: rendering.c editor.h
	$(CC) $(CFLAGS) -c rendering.c

selection.o: selection.c editor.h
	$(CC) $(CFLAGS) -c selection.c

syntax.o: syntax.c editor.h
	$(CC) $(CFLAGS) -c syntax.c

clean:
	rm -f $(TARGET) $(OBJS) *.tmp *.bak *.backup *.autosave *.emergency /tmp/kilo_editor_clipboard.txt cursor_x cursor_y dirty line_count lines* select_* 2>/dev/null || true
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
