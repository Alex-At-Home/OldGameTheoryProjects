CXXFLAGS =	-O2 -g -Wall -fmessage-length=0 $(INCLUDE)
#CXXFLAGS =	-g -Wall -fmessage-length=0 $(INCLUDE)
CFLAGS = -O2 -g -Wall $(INCLUDE)

INCLUDE = -Ipokerenum/include

OBJS =		BoardAnalyzer.o CCard.o CBoard.o CBoardAnalyzer.o UnitTest.o\
			GTO_demo_1.o GTO_demo_2.o GTO_demo_preflop.o \
	pokerenum/deck.o\
	pokerenum/deck_std.o\
	pokerenum/rules_std.o\
	pokerenum/evx.o\
	pokerenum/enumerate.o\
	pokerenum/enumord.o\
	pokerenum/combinations.o\
	pokerenum/t_cardmasks.o\
	pokerenum/t_maskrank.o\
	pokerenum/t_nbits.o\
	pokerenum/t_straight.o\
	pokerenum/t_topbit.o\
	pokerenum/t_topcard.o\
	pokerenum/t_topfivebits.o\
	pokerenum/t_topfivecards.o\
	pokerenum/t_toptwobits.o\
	pokerenum/t_botcard.o\
	pokerenum/t_botfivecards.o\
	pokerenum/t_botfivecardsj.o
	

LIBS =

TARGET =	BoardAnalyzer.exe

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS) 

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

cleanish:
	rm -f $(OBJS) 
	