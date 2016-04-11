#CXXFLAGS =	-O2 -g -Wall -fmessage-length=0 $(INCLUDE)
CXXFLAGS =	-g3 -Wall -fmessage-length=0 $(INCLUDE)
#CFLAGS = -O2 -g -Wall $(INCLUDE)
#CFLAGS = -g3 -Wall $(INCLUDE)

#INCLUDE = -I

OBJS =		ZeroOneGames_altConv.o
	
LIBS =

TARGET =	ZeroOneGames.exe

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS) 

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
