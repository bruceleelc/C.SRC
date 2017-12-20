SRCDIR = .
SRCINC1 = ../comm
OBJDIR = .
BINDIR = .

CFLAGS:= -g -O0 -D_GNU_SOURCE -DTIXML_USE_STL -I. -fPIC
LDFLAGS:= -lrt -lpthread -L/usr/lib64/mysql/ -lmysqlclient -L. -lzlog -lrabbitmq -lzmq
TARGET:= ./bin/gpslocationdeal
CC=g++
SRC := $(wildcard $(SRCDIR)/*.cpp)
SRC1 := $(wildcard $(SRCINC1)/*.cpp)

OBJS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC))
OBJS1 := $(patsubst $(SRCINC1)/%.cpp, $(OBJDIR)/%.o, $(SRC1))

DEPS = $(SRC:.cpp=.d) 
.PHONY : all deps clean veryclean

all : $(TARGET)

deps : $(DEPS)

objs : $(OBJS) $(OBJS1)

$(OBJS) : $(OBJDIR)/%.o : $(SRCDIR)/%.cpp
	$(CC) -c $(CFLAGS) -I $(SRCINC1) $< -o $@
$(OBJS1) : $(OBJDIR)/%.o : $(SRCINC1)/%.cpp
	$(CC) -c $(CFLAGS) -I $(SRCINC1) $< -o $@


clean:
	rm -f $(OBJDIR)/*.o
	rm -f *.d
	rm -f $(BINDIR)/$(TARGET)
$(TARGET) : $(OBJS) $(OBJS1)
	$(CC) $(OBJS) $(OBJS1) -o $(BINDIR)/$(TARGET) $(LDFLAGS)


