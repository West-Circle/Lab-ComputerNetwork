CC = g++
LIB = -lpthread

EXE = webserver
OBJ = RequestHandler.o Network.o main.o csapp.o

$(EXE):$(OBJ)
	$(CC) -g -o $@ $(OBJ) $(LIB)

.SUFFIXES: .cpp .o .hpp

.cpp.o:
	$(CC) $(CC_FLAG) -g -c $*.cpp -o $*.o $(LIB)

.PRONY: clean

clean:
	@echo "Removing Link and Compiled Files"
	rm -f $(OBJ) $(EXE)
