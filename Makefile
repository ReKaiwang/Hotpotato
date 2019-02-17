TARGETS=player ringmaster
all: $(TARGETS)
clean:
	rm -f $(TARGETS)
player: player.cpp
	g++ -pedantic -Werror -Wall -std=gnu++98 -o $@ $<
ringmaster: ringmaster.cpp
	g++ -pedantic -Werror -Wall -std=gnu++98 -o $@ $<