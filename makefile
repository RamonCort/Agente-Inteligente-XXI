CXX := c++
EXE := bin/ProyectoBase
SRC := src/main.cpp

.PHONY: all clean run

all: run

run: $(EXE)
	./$(EXE)

$(EXE): $(SRC) | bin
	$(CXX) $(SRC) -o $(EXE) -std=c++17 -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

bin:
	mkdir bin

clean:
	del /Q bin\*.exe
