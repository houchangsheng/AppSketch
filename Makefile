EXEC += appsketch
all: $(EXEC)

CC	= gcc
CXX	= g++
LD	= $(CXX)

CFLAGS   = -Wall -std=c++11 -O3 $(incdir)	 
CXXFLAGS = -Wall -std=c++11 -O3 $(incdir)
LDFLAGS	 = -static

srcdir = ./libprotoident/lib/tcp \
	 ./libprotoident/lib/udp \
	 ./libprotoident/lib \
	 ./libprotoident/tools \
	 ./libprotoident \
	 ../AppSketch

srcdir2 = ./libprotoident/lib/tcp \
	 ./libprotoident/lib/udp \
	 ./libprotoident/lib \
	 ./libprotoident/tools \
	 ./libprotoident \

incdir = $(foreach dir,$(srcdir),-I$(dir))

inclib = -lflowmanager -ltrace -lpthread -lwandder -lpcap -lwandio -llz4 -lzstd -llzma -llzo2 -lbz2 -lz

allsrc = $(foreach dir,$(srcdir),$(wildcard $(dir)/*.cc))
allsrc2 = $(foreach dir,$(srcdir),$(wildcard $(dir)/*.c))
allsrc3 = $(foreach dir,$(srcdir),$(wildcard $(dir)/*.cpp))

allobj = $(allsrc:%.cc=%.o)
allobj2 = $(allsrc2:%.c=%.o)
allobj3 = $(allsrc3:%.cpp=%.o)
allobj4 = $(foreach dir,$(srcdir2),$(wildcard $(dir)/*.o))

%.o: %.cc
	@echo Compiling $<...
	@-$(CXX) $(CXXFLAGS) -c $< -o $@
%.o: %.c
	@echo Compiling $<...
	@-$(CXX) $(CXXFLAGS) -c $< -o $@
%.o: %.cpp
	@echo Compiling $<...
	@-$(CXX) $(CXXFLAGS) -c $< -o $@

appsketch: $(allobj4) $(allobj3) $(allobj2) $(allobj)
	@echo Linking target...
	@$(LD) $(CXXFLAGS) -o $@ main.o sketch.o sketch2.o hash.o $(allobj4) $(inclib)
	@echo Link done!

.PHONY:	all clean

clean:
	@rm -rf $(EXEC) main.o sketch.o sketch2.o hash.o $(allobj3) $(allobj2)
	@echo clean done.
