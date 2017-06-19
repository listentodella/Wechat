Target=server client
all:$(Target)
server:s_fun.o server.o
	gcc -o $@ $^ -lpthread -Wall
client:c_fun.o client.o
	gcc -o $@ $^ -Wall
%.o:%.c
	gcc -c $< -o $@ -lpthread -Wall	
clean:
	rm *.o $(Target)
