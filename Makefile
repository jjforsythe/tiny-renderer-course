IDIR = include
CC = g++
CFLAGS = -I$(IDIR)

ODIR = obj
LDIR = lib
BDIR = bin

LIBS = -lm

_DEPS = tgaimage.h model.h geometry.h
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = tgaimage.o model.o main.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: src/%.cpp $(DEPS)
	@mkdir -p $(ODIR)
	$(CC) -c -o $@ $< $(CFLAGS)

main: $(OBJ)
	@mkdir -p $(BDIR)
	@mkdir -p output
	$(CC) -o $(BDIR)/$@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~
