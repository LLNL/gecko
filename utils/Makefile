include ../Config

TARGET = ../bin/gecko

all: $(TARGET)

$(TARGET): gecko.cpp ../lib/$(LIBGECKO)
	mkdir -p ../bin
	$(CXX) $(CXXFLAGS) gecko.cpp -L../lib -lgecko -o $(TARGET)

clean:
	rm -f $(TARGET)
