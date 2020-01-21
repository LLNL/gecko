include ../Config

BINDIR = ../bin
LIBDIR = ../lib
TARGET = $(BINDIR)/testgecko

all: $(TARGET)

clean:
	rm -f $(TARGET)

test: $(BINDIR)/testgecko
	$(BINDIR)/testgecko

$(TARGET): testgecko.cpp $(LIBDIR)/$(LIBGECKO)
	mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) testgecko.cpp -L$(LIBDIR) -lgecko -o $(TARGET)
