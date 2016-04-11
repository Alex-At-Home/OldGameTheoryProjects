#ifndef CBOARDANALYZER_H_
#define CBOARDANALYZER_H_

////////////////////////////////////////////////////

#include <string>
#include <map>

#include "CCard.h"

////////////////////////////////////////////////////
////////////////////////////////////////////////////

class CBoardAnalyzer {
	
public:
	CBoardAnalyzer(); 

	// Add board card (flop or turn)
	bool addCardToBoard(const CCard& newCard);
	
	// Add hole cards
	bool addHand(const CCard::Hole& newHand);
	
	// Extract hand features
	void analyzeBoard();

	// Various feature extract formats
	int getFeatures() const {  return _nFeatures; }
	std::string getFeatures(bool bVerbose) const;
	static std::string getHandFeatures(int nFeatures, bool bVerbose);
	
	// Sequences enum
	enum { FLUSH_DRAW = 0x01, MADE_FLUSH = 0x02,
			STRONG_FLUSH = 0x4, MEDIUM_FLUSH = 0x8, WEAK_FLUSH = 0x10,

			OPENENDED = 0x20, GUTSHOT = 0x40, BELLYBUSTER = 0x80, STRAIGHT = 0x100,
			ONECARDSTR8 = 0x200, TWOCARDSTR8 = 0x400, 
			LOWENDSTR8 = 0x800, 
			STR8FLUSH = 0x1000
	};
	static const int ANY_STR8_POSSIBILITIES  = OPENENDED|GUTSHOT|BELLYBUSTER|STRAIGHT|ONECARDSTR8|TWOCARDSTR8|LOWENDSTR8;
	static const int ANY_DRAW = 0x1FFF; // (ie any of the bits from the enum above)

	// Matches enum
	enum {
		PAIR = 0x2000, TWOPAIR = 0x4000, TRIPS = 0x8000, SET = 0x10000, FULLHOUSE = 0x20000, QUADS = 0x40000,
		OVERPAIR = 0x80000, TOPPAIR = 0x100000, SECONDPAIR = 0x200000, MIDDLEPAIR = 0x400000, BOTTOMPAIR = 0x800000,
		TOP_2P = 0x80000, TOPMIDDLE_2P = 0x100000, MIDDLE_2P = 0x200000, ONECARD = 0x400000,
		UNDERFILL = 0x800000,
		TOPKICKER = 0x1000000, GOODKICKER = 0x2000000, WEAKKICKER = 0x4000000,
		ONEOVERCARD = 0x8000000, TWOOVERCARDS = 0x10000000,
		FAKE = 0x20000000, PAIRED_BOARD = 0x40000000
	};
	static const int NOOVERCARDS = ~(TWOOVERCARDS|ONEOVERCARD);
	static const int WEAKFEATURES = PAIR|TWOPAIR|TRIPS|SET|OVERPAIR|TOPPAIR|SECONDPAIR|MIDDLEPAIR|BOTTOMPAIR|
									TOP_2P|TOPMIDDLE_2P|MIDDLE_2P|ONECARD|TOPKICKER|GOODKICKER|WEAKKICKER|ONEOVERCARD|TWOOVERCARDS;
	enum { MONSTERS = 0, STRONG_DRAWS = 1, STRONG_MADE = 2, MEDIOCRE_DRAWS = 3, MEDIOCRE_MADE = 4, WEAK_DRAWS = 5, RUBBISH = 6 };				

	// Categorizes hands into the above enum given the feature set
	static int categorizeHand(int nFeatures);	

	enum { 
		STATIC_HIGHCARD_BASE = 0, 	// (4 values)
		STATIC_PAIR_BASE = 4,		// (6 values)
		STATIC_2PAIR_BASE = 10,		// (3 values)
		STATIC_3KIND_BASE = 13,		// (2 values)
		STATIC_STR8_BASE = 15,		// (2 values)
		STATIC_FLUSH_BASE = 17,		// (3 values)
		STATIC_FH_BASE = 20,		// (2 values)
		STATIC_QUADS_BASE = 22,		// (1 value)
		STATIC_STR8FL_BASE = 23,	// (1 value)
		STATIC_MAX = 24
	};
	static const int _nHandCategoryTable[];
	static const char *_szHandCategoryTable[];
	int calculateHandStrength(int nFeatures);
	
	// cf CBoardAnalyzer "enum { MONSTERS = 0, STRONG_DRAWS = 1, STRONG_MADE = 2, MEDIOCRE_DRAWS = 3, MEDIOCRE_MADE = 4, RUBBISH = 5 };"
	static const char* _pcHandSummaryTable[];

	// Pulls hands from input %s and a hand ordering
	static int assignHandFromRange(int nPct, int nPct_max, std::deque<CCard::Hole>& lHandRange, std::string& sHandList);
	static int assignHandFromRange(const std::string& sName, std::string& sHandRange);
	
	// Perfect equity calculation between 2 hands
	static double calculateEquity(const CCard::Hole& h1, const CCard::Hole& h2, const int nBoardTable[], int nCardsOnBoard);

	// Faster version that relies on previously categorized hands, not 100% accurate
	static double approximateEquity(const CBoardAnalyzer& h1Px, const CBoardAnalyzer& h2Px,
			const CCard::Hole& h1, const CCard::Hole& h2, const int nBoardTable[], int nCardsOnBoard);

	// Call this once per flop before calling approximateEquity
	static void resetEquityCache();

	// Some flop play information
	static double GetHandRangeWeighting(const std::string& sRangeName, int nHandCategory);
	
	static std::string DisplayPreFlopRanges();
	static std::string DisplayPostFlopRanges();
	
protected:
	enum { UNDEALT = 0, BOARD = 1, HAND = 2 };
	std::map<CCard, int> _boardAndHand;
	CCard _hand[2];
	int _suitTable[5]; // (1-4, see CCard enum)
	int _nFeatures;
	static double _dEquityCache_noSuit[225][225]; 
	static unsigned int _nEquityCache_levelTable[225][225];
	static unsigned int _nEquityCache_level;

	static std::map<std::string, std::string> _handRangeByName;
	
	// (Utility function for getting kickers from the hand)
	static int getKicker(int nRank, int nPair)
	{
		int nBestRank = (14 == nPair)?13:14;
		
		if (nBestRank == nRank) return TOPKICKER;
		else if (nRank >= (nBestRank - 4)) return GOODKICKER;
		else return WEAKKICKER;
	}
	
	// Some utility parameters for hand strength calculations:
	struct {
		int nHighCardRank;
		int nSecondCardRank;
		int nLowCardRank;	
	} _boardSummary;
	
	struct HandRangeCache {
		const std::map<int, double>* pmRange;
		const char *szRangeName;
	};
	static HandRangeCache _handRangeCache;
	static std::map<std::string, std::map<int, double> > _mHandRangeMap;

};

////////////////////////////////////////////////////
////////////////////////////////////////////////////

#endif /*CBOARDANALYZER_H_*/
