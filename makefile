
OBJS = \
	list.o \
	shell.o

shell: $(OBJS)
	gcc -o shell $(OBJS)

%.o: %.c
	gcc -c -o $@ $<

.PHONY: clean
clean:
	rm -rf *.o
