run: build
	./run

build: folders main species
	g++ bin/*.o -lSDL2 -o run

main: folders
	g++ -lSDL2 -c src/main.cpp -o bin/main.o
species: folders
	g++ -lSDL2 -c src/species.cpp -o bin/species.o

folders:
	mkdir -p src bin