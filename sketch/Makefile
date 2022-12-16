EXEC += appsketch
all: $(EXEC)

#编译工具链
CC	= gcc
CXX	= g++
LD	= $(CXX)

#编译选项
CFLAGS   = -Wall -std=c++11 -O3 $(incdir)	 
CXXFLAGS = -Wall -std=c++11 -O3 $(incdir)
LDFLAGS	 = -static

#定义源码目录
srcdir = ../libprotoident/lib/tcp/ \
	 ../libprotoident/lib/udp/ \
	 ../libprotoident/lib/ \
	 ../libprotoident/tools/ \
	 ./
srcdir_t = ../libprotoident/tools/protoident/

#定义包含目录
incdir = $(foreach dir,$(srcdir),-I$(dir))
incdir += $(foreach dir,$(srcdir_t),-I$(dir))

#定义附加依赖库
inclib = -lflowmanager -ltrace -lpthread -lwandder -lpcap -lwandio -llz4 -lzstd -llzma -llzo2 -lbz2 -lz

#遍历所有dir,并搜索该dir下面的所有.c文件
allsrc = $(foreach dir,$(srcdir),$(wildcard $(dir)/*.cc))
allsrc_t = $(foreach dir,$(srcdir_t),$(wildcard $(dir)/*.cc))
allsrc2 = $(foreach dir,$(srcdir),$(wildcard $(dir)/*.c))
allsrc3 = $(foreach dir,$(srcdir),$(wildcard $(dir)/*.cpp))

#把所有的.c替换为.o
allobj = $(allsrc:%.cc=%.o)
allobj_t = $(allsrc_t:%.cc=%.o)
allobj2 = $(allsrc2:%.c=%.o)
allobj3 = $(allsrc3:%.cpp=%.o)
	
#生成.o的规则
%.o: %.cc
	@echo Compiling $<...
	@-$(CXX) $(CXXFLAGS) -c $< -o $@
%.o: %.c
	@echo Compiling $<...
	@-$(CXX) $(CXXFLAGS) -c $< -o $@
%.o: %.cpp
	@echo Compiling $<...
	@-$(CXX) $(CXXFLAGS) -c $< -o $@

appsketch: $(allobj3) $(allobj2) $(allobj) $(allobj_t)
	@echo Linking target...
	@$(LD) $(CXXFLAGS) -o $@ $(allobj3) $(allobj2) $(allobj) $(allobj_t) $(inclib)
	@echo Link done!

#声明虚拟目标
.PHONY:	all clean

clean:
	@rm -rf $(EXEC) $(allobj3) $(allobj2) $(allobj_t)
	@echo clean done.
