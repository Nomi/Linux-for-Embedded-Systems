OBJS := whackamole.o 
whackamole: $(OBJS)
	$(CC) -o whackamole $(CFLAGS) $(LDFLAGS) $(OBJS) -lgpiod
$(OBJS) : %.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@