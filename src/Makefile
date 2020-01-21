include ../Config

LIBDIR = ../lib
TARGETS = $(LIBDIR)/libgecko.a $(LIBDIR)/libgecko.so
OBJECTS = drawing.o graph.o subgraph.o version.o

static: $(LIBDIR)/libgecko.a

shared: $(LIBDIR)/libgecko.so

clean:
	rm -f $(TARGETS) $(OBJECTS)

$(LIBDIR)/libgecko.a: $(OBJECTS)
	mkdir -p $(LIBDIR)
	rm -f $@
	ar rc $@ $^

$(LIBDIR)/libgecko.so: $(OBJECTS)
	mkdir -p $(LIBDIR)
	$(CXX) $(CXXFLAGS) -shared $^ -o $@

.cpp.o:
	$(CXX) $(CXXFLAGS) -c $<
