#include "CBoardAnalyzer.h"

#include "poker_defs.h"
#include "enumdefs.h"

#include <memory.h>

////////////////////////////////////////////////////

// cf CBoardAnalyzer "enum { MONSTERS = 0, STRONG_DRAWS = 1, STRONG_MADE = 2, MEDIOCRE_DRAWS = 3, MEDIOCRE_MADE = 4, RUBBISH = 5 };"
const char* CBoardAnalyzer::_pcHandSummaryTable[] = { "Monsters", 
		"Strong Draws", "Strong Made Hands", "Mediocre Draws", "Mediocre Made Hands", "Weak Draws",
		"Rubbish"
};

unsigned int CBoardAnalyzer::_nEquityCache_level = 0; 
double CBoardAnalyzer::_dEquityCache_noSuit[225][225]; 
unsigned int CBoardAnalyzer::_nEquityCache_levelTable[225][225];

const int CBoardAnalyzer::_nHandCategoryTable[] = {
	CBoardAnalyzer::STATIC_HIGHCARD_BASE, 	// (4 values)
	CBoardAnalyzer::STATIC_PAIR_BASE,		// (6 values)
	CBoardAnalyzer::STATIC_2PAIR_BASE,		// (3 values)
	CBoardAnalyzer::STATIC_3KIND_BASE,		// (2 values)
	CBoardAnalyzer::STATIC_STR8_BASE ,		// (2 values)
	CBoardAnalyzer::STATIC_FLUSH_BASE,		// (3 values)
	CBoardAnalyzer::STATIC_FH_BASE,		// (2 values)
	CBoardAnalyzer::STATIC_QUADS_BASE,		// (1 value)
	CBoardAnalyzer::STATIC_STR8FL_BASE,	// (1 value)
	};
const char *CBoardAnalyzer::_szHandCategoryTable[] = {
	"High card:",
	"Pair:",
	"Two-pair:",
	"Trips/set:",
	"Straight:",
	"Flush:",
	"Full house:",
	"Quads:",
	"Straight flush:",
};

////////////////////////////////////////////////////

CBoardAnalyzer::CBoardAnalyzer()
{ 
	for(int i = 0; i < 5; ++i) _suitTable[i] = 0; 
	_nFeatures = 0;
}

////////////////////////////////////////////////////

std::string CBoardAnalyzer::getFeatures(bool bVerbose) const
{
	std::string sHand = CBoardAnalyzer::getHandFeatures(_nFeatures, bVerbose);
	return sHand;
}

//________________________________

std::string CBoardAnalyzer::getHandFeatures(int nFeatures, bool bVerbose)
{
	std::string sHand;
	// Note assume the features vector excludes all inconsitencies (eg str8+flush)
	
	// Regardless of verbosity:
	
	if (nFeatures & STR8FLUSH) {
		if (nFeatures & ONECARDSTR8) {
			sHand += "One card Bottom end Straight Flush; ";
		}
		else sHand += "Straight Flush; "; 
	}
	else if (nFeatures & PAIR) {
		
		if (nFeatures & OVERPAIR) {
			sHand += "Overpair; ";
		}
		else if (nFeatures & TOPPAIR) {
			sHand += "Top Pair";
		}
		else if (nFeatures & SECONDPAIR) {
			sHand += "2nd Pair";
		}
		else if (nFeatures & MIDDLEPAIR) {
			sHand += "Middle Pair";
		}
		else if (nFeatures & BOTTOMPAIR) {
			sHand += "Bottom Pair";
		}
		else if (nFeatures & FAKE) {
			sHand += "Fake Pair";
		}
		if (nFeatures & TOPKICKER) {
			sHand += " Top kicker; ";
		}
		else if (nFeatures & GOODKICKER) {
			sHand += " Good kicker; ";
		}
		else if (nFeatures & WEAKKICKER) {
			sHand += " Weak kicker; ";
		}
		else if (!(nFeatures & OVERPAIR)) sHand += "; ";
	}
	else if (nFeatures & TWOPAIR) {
		if (nFeatures & ONECARD) {
			sHand += "One card ";
		}
		if (nFeatures & FAKE) {
			sHand += "Fake 2 Pair";
		}
		if (nFeatures & TOP_2P) {
			sHand += "Top 2 Pair";
		}
		else if (nFeatures & TOPMIDDLE_2P) {
			sHand += "Top and middle 2 Pair";
		}
		else if (nFeatures& MIDDLE_2P) {
			sHand += "Middle 2 Pair";
		}
		else {
			sHand += "Weak 2 Pair";
		}
		if (nFeatures & TOPKICKER) {
			sHand += " Top kicker; ";
		}
		else if (nFeatures & GOODKICKER) {
			sHand += " Good kicker; ";
		}
		else if (nFeatures & WEAKKICKER) {
			sHand += " Weak kicker; ";
		}
		else sHand += "; ";
	}
	else if (nFeatures & SET) {
		sHand += "Set; ";
	}
	else if (nFeatures & TRIPS) {
		if (nFeatures & FAKE) {
			sHand += "Fake ";
		}
		sHand += "Trips";
		if (nFeatures & TOPKICKER) {
			sHand += " Top kicker; ";
		}
		else if (nFeatures & GOODKICKER) {
			sHand += " Good kicker; ";
		}
		else if (nFeatures & WEAKKICKER) {
			sHand += " Weak kicker; ";
		}
		else sHand += "; ";
	}
	if (nFeatures & ONEOVERCARD) {
		sHand += "One Overcard; ";
	}
	else if (nFeatures & TWOOVERCARDS) {
		sHand += "Two Overcards; ";
	}
		
	if (bVerbose) {

// Quads full house		
		if (nFeatures & QUADS) {
			
			if (nFeatures & FAKE) {
				sHand += "Fake Quads ";
				if (nFeatures & TOPKICKER) {
					sHand += "Top kicker; ";
				}
				else if (nFeatures & GOODKICKER) {
					sHand += "Good kicker; ";
				}
				else if (nFeatures & WEAKKICKER) {
					sHand += "Weak kicker; ";
				}
			}
			else sHand += "Quads;";
		}
		else if (nFeatures & FULLHOUSE) {
			if (nFeatures & UNDERFILL) {
				sHand += "Underfill Full house; ";
			}
			else sHand += "Full house; ";
		}
// Flushes etc
		else if (nFeatures & STRONG_FLUSH) {
			sHand += "Strong Flush";		
		}
		else if (nFeatures & MEDIUM_FLUSH) {
			sHand += "Medium Flush";		
		}
		else if (nFeatures & WEAK_FLUSH) {
			sHand += "Weak Flush";		
		}
		if (nFeatures & MADE_FLUSH) {
			sHand += "; ";
		}
		else if (nFeatures & FLUSH_DRAW) {
			sHand += " Draw; ";
		}		
// Straights etc
		if ((nFeatures & ANY_STR8_POSSIBILITIES) && !(nFeatures & STR8FLUSH)) {
			if (nFeatures & ONECARDSTR8) {
				sHand += "One card";
			}
			else if (nFeatures & TWOCARDSTR8) {
				sHand += "Two card";			
			}

			if (nFeatures & (ONECARDSTR8|TWOCARDSTR8)) {
				if (nFeatures & LOWENDSTR8) {
					sHand += " Bottom end";
				}
				
				if (nFeatures & OPENENDED) {
					sHand += " Open ended Straight Draw; ";
				}
				else if (nFeatures & GUTSHOT) {
					sHand += " Gutshot Straight Draw; ";				
				}
				else if (nFeatures & BELLYBUSTER) {
					sHand += " Bellybuster Straight Draw; ";
				}
				else if (nFeatures & STRAIGHT) {
					sHand += " Straight; ";
				}			
			}
		}
	}
	else {
		if (nFeatures & QUADS) {
			sHand += "Quads; ";
		}
		else if (nFeatures & FULLHOUSE) {
			sHand += "Full house; ";
		}
// Flushes etc
		else if (nFeatures & FLUSH_DRAW) {
			sHand += "Flush Draw; ";
		}
		else if (nFeatures & MADE_FLUSH) {
			sHand += "Flush; ";
		}
// Straights etc
		if (nFeatures & (ONECARDSTR8|TWOCARDSTR8)) {
			if (nFeatures & (OPENENDED|BELLYBUSTER)) {
				sHand += "Open ended Straight Draw; ";
			}
			else if (nFeatures & GUTSHOT) {
				sHand += "Gutshot Straight Draw; ";				
			}
		}
		if (nFeatures & STRAIGHT) {
			sHand += "Straight; ";
		}			
	}
	if (0 == nFeatures) sHand += "Air; ";
	
	return sHand;
}

//________________________________

bool CBoardAnalyzer::addCardToBoard(const CCard& newCard)
{
	if (_boardAndHand.size() == 6) return false; // only handling NLHE to turn

	// Save useful rank information

	int nRank = newCard.getRank(); 
	if (_boardAndHand.empty()) {
		_boardSummary.nHighCardRank = 1;			
		_boardSummary.nSecondCardRank = 1;			
		_boardSummary.nLowCardRank = 15;			
	}
	if (nRank >= _boardSummary.nHighCardRank) {
		_boardSummary.nHighCardRank = nRank;			
	}
	else if (nRank >= _boardSummary.nSecondCardRank) {
		_boardSummary.nSecondCardRank = nRank;						
	}
	if (nRank <= _boardSummary.nLowCardRank) {
		_boardSummary.nLowCardRank = nRank;
	}
	
	// Save card
	
	int *pn = &_boardAndHand[newCard];
		
	// Fill in suit information
	
	if (UNDEALT == *pn)
	{
		*pn = BOARD;
		_suitTable[newCard.getSuit()]++;
		return true;
	}
	else return false;
}

//________________________________

bool CBoardAnalyzer::addHand(const CCard::Hole& newHand)
{
	if (_boardAndHand.size() == 6) return false; // only handling NLHE to turn

	int *pn = &_boardAndHand[newHand.first];
	
	if (UNDEALT == *pn)
	{
		*pn = HAND;
		_hand[0] = newHand.first;
		_suitTable[newHand.first.getSuit()]++;

		pn = &_boardAndHand[newHand.second];
		if (UNDEALT == *pn) {
			*pn = HAND;
			_hand[1] = newHand.second;
			_suitTable[newHand.second.getSuit()]++;
			return true;
		}		
		else return false;
	}
	else return false;
}

//________________________________

// Note this only handles up to the turn in NLHE (ie 6 cards total)

void CBoardAnalyzer::analyzeBoard()
{
	_nFeatures = 0;
	
// Flush draws and flushes

	int nSuit_flush = 0, nSuitSize;
	for (int i = CCard::HEARTS; i <= CCard::SPADES; ++i) {
		if ((nSuitSize = _suitTable[i]) >= 4) {
			// We have a flush/flush draw
			// Base strength on high card if it's of the right suit
			int nRank = 0;

			for (int j = 0; j < 2; ++j) {
				int nSuit = _hand[j].getSuit();
				if (i == nSuit) {
					nSuit_flush = i;
					int nNewRank = _hand[j].getRank(); 
					if (nNewRank > nRank) nRank = nNewRank;
				}
			}

			if (nRank > 0) { // otherwise it's just the board...
				if (nRank > 11) _nFeatures |= STRONG_FLUSH;
				else if (nRank < 7) _nFeatures |= WEAK_FLUSH;
				else _nFeatures |= MEDIUM_FLUSH;
				
				if (4 == nSuitSize) _nFeatures |= FLUSH_DRAW;
				else _nFeatures |= MADE_FLUSH;
			}
			break;
		}
	}
	// Some pre-calcs for str8 flushes:
	int nBoardAndHandSize = _boardAndHand.size();

	int nRank_str8fl = 0;
	if (_boardAndHand.begin()->first.getSuit() != nSuit_flush) 
		nRank_str8fl = _boardAndHand.begin()->first.getRank();
	else if (_boardAndHand.rbegin()->first.getSuit() != nSuit_flush) 
		nRank_str8fl = _boardAndHand.rbegin()->first.getRank();
	// else it's in the middle, so can't be a str8 flush
	
// Everything else...
		
	// Some initialization
	std::map<CCard, int>::iterator it = _boardAndHand.begin();	
	CCard highestCard = _boardAndHand.rbegin()->first;
	int nHighestInHole = _boardAndHand.rbegin()->second;
	int nHighestRank = highestCard.getRank();
	int nLastRank = 0, nLastHole = 0; 	
	
	// Straight state:
	int nLowEnd_str8 = 0, nTopEnd_str8 = 0;
	int nLowHoleCard_str8 = 0, nHighHoleCard_str8 = 0;
	int nHoleCardsIn_str8 = 0;
	unsigned char ucCluster_str8 = 0; 
		// (binary representation for an 8 card span starting at nLowEnd_str8, 1=card present, 0=not, LSb=low)
		// (we'll worry about the hole cards and special cases separately)

	// Pairs, 2-pairs, trips, sets, etc state
	int nRank_matches = 0;
	int nTable_matches[3]; // (<= 6 cards, so at most 3 matching ranks)
	int nHoleCard_matches[3];
	int nRankOf_matches[3];
	int nLowHoleCard_matches = 0, nHighHoleCard_matches = 0;
	int nTopBoardCard_matches = 0, n2ndBoardCard_matches = 0, nLowBoardCard_matches = 0;
	                
	// For straights, some handling of the wrap around A...
	if (14 == nHighestRank) {
		ucCluster_str8 = 0x1;
		nLowEnd_str8 = 1;
		nTopEnd_str8 = 1;
		if (HAND == nHighestInHole) {
			nLowHoleCard_str8 = 1;
			nHoleCardsIn_str8 = 1;
		}
	}
		
	for (; it != _boardAndHand.end(); ) {
		int nRank = it->first.getRank();
		int nHole = (HAND == it->second)?1:0;

// Straight draw		

		if (!(_nFeatures & ANY_STR8_POSSIBILITIES)) {
			
			if (nHole) { 
				if (nLowHoleCard_str8) nHighHoleCard_str8 = nRank;
				else nLowHoleCard_str8 = nRank;
			}				

			int nSpan_str8 = 0;

			if (0 == ucCluster_str8) { // Initialization
				ucCluster_str8 = 0x1;
				nLowEnd_str8 = nRank;
				nTopEnd_str8 = nRank;
				nHoleCardsIn_str8 = nHole;				
			}
			else if (nRank == nLastRank) { // this is only significant if the board pairs one of our hole cards 

				if (nHole || nLastHole) {
					if (nRank == nLowHoleCard_str8) nLowHoleCard_str8 = 0;
					else if (nRank == nHighHoleCard_str8) nHighHoleCard_str8 = 0;

					if (nLastHole && !nHole) nHoleCardsIn_str8--;  
				}
				if ((int)it->first == (int)highestCard) nSpan_str8 = -1;
			}
			else { //if (nRank > nLastRank) {

				nSpan_str8 = nRank - nLowEnd_str8;
				
				if (nSpan_str8 <= 6) { // Adjust the lookup word
					
					nTopEnd_str8 = nRank;
					nHoleCardsIn_str8 += nHole;				
					ucCluster_str8 |= (0x1 << nSpan_str8);

					if ((int)it->first == (int)highestCard) nSpan_str8 = -1; // final card, lookup
				}
				else if ((int)it->first == (int)highestCard) {
					// Cases where the final card is part of the str8 draw and results in a span>6, eg:
					// 1??0,111f, this is a straight draw				
					// 1001,111f, this is a straight
					// 1011,110f, a 1c straight could become a 2c straight... 				
					if (((nRank == (1+nLastRank)) && (7 == (ucCluster_str8>>4))) ||
							((nRank == (2+nLastRank)) && (7 == (ucCluster_str8>>3))))
					{
						
						// Duplicate the normal span update code at the bottom of the next switch statement
						while (nSpan_str8 > 6) {
							do {
								if (nLowHoleCard_str8 == nLowEnd_str8) nHoleCardsIn_str8--;
								if (nHighHoleCard_str8 == nLowEnd_str8) nHoleCardsIn_str8--;
								ucCluster_str8 >>= 1;
								nSpan_str8--;
								nLowEnd_str8++;
							} while (ucCluster_str8 && !(0x1 & ucCluster_str8));
	
							// Now update the str8 draw for the newest card
							nTopEnd_str8 = nRank;
							ucCluster_str8 |= (1 << nSpan_str8);
							nHoleCardsIn_str8 += nHole;				
						}					
						nSpan_str8 = -1; // final card, lookup
					}
				}								
			} // end rank > last rank
				
			if ((-1 == nSpan_str8)||(nSpan_str8 > 6)) { // (note only handling 6-card boards)
				// First lookup to see if we have something

				// We're going to look for LOW str8 draws where making your str8 makes
				// everyone else a 4-card str8 draw - this is where most of the complexity in the
				// following code is.

				//printf("LOOKUP %x %d %d(%d %d %d)\n", ucCluster_str8,nLowEnd_str8, nTopEnd_str8, nLowHoleCard_str8,nHighHoleCard_str8,nHoleCardsIn_str8);

				switch (ucCluster_str8) { // (NB never set MSb, nSpan_str8 <= 6) 
			// Made straights:
				case 0x1F: // LSb 1111100? MSb
				case 0x7D: // LSb 1011111? MSb [6 cards]
				{
					_nFeatures |= STRAIGHT;
					break;
				}
			// Special-case made straights:
				case 0x5F: // LSb 1111101? MSb [6 cards] NOT 1111110b?
				{
					_nFeatures |= STRAIGHT;
					if ((nLowHoleCard_str8 == nTopEnd_str8)||(nHighHoleCard_str8 == nTopEnd_str8))  {
						nHoleCardsIn_str8--; // (obviously the top card doesn't count!)
					}
					break;
				}
				case 0x3F: // LSb 1111110? MSb [6 cards] LOW bb11110?
				{
					_nFeatures |= STRAIGHT;
					if ((nLowHoleCard_str8 == nLowEnd_str8&&(nHighHoleCard_str8 == (nLowEnd_str8+1))))  {
						nHoleCardsIn_str8--;
						_nFeatures |= LOWENDSTR8;
					}
					break;
				}
			// 8-outers: 
				case 0x0F: // LSb 1111000? MSb 
				{
					if ((1 == nLowEnd_str8)||(14 == nTopEnd_str8)) _nFeatures |= GUTSHOT; // wrap draws
					else _nFeatures |= OPENENDED;
					break;
				}
				case 0x5D: // LSb 1011101? MSb
				{
					_nFeatures |= BELLYBUSTER;
					break;
				}
			// Special-case 8-outers:
				case 0x79: // LSb 1001111? MSb NOT b001111?
				{
					if (14 == nTopEnd_str8) _nFeatures |= GUTSHOT; // wrap draws
					else _nFeatures |= OPENENDED;
					
					if (1 == nHoleCardsIn_str8) { // 0x4F case
						if ((nLowHoleCard_str8 == nLowEnd_str8)||(nHighHoleCard_str8 == nLowEnd_str8)) { 
							nHoleCardsIn_str8 = 0; // (so will come out as a fake str8 draw)
						}			
					} 
					break;
				}
				case 0x4F: // LSb 1111001? MSb NOT 111100b? 
				{
					_nFeatures |= OPENENDED;

					if (1 == nHoleCardsIn_str8) { // 0x4F case
						if ((nLowHoleCard_str8 == (nLowEnd_str8+6))||(nHighHoleCard_str8 == (nLowEnd_str8+6))) { 
							nHoleCardsIn_str8 = 0; // (so will come out as a fake str8 draw)
						}								
					} 
					break;
				}
				case 0x6F: // LSb 1111011? MSb [6 cards] GUTSHOT 11110bb? LOW 1b11011? 
				case 0x2F: // LSb 1111010? MSb LOW 1b11010? GUTSHOT 11110b0
				{
					if (1 == nLowEnd_str8) _nFeatures |= GUTSHOT; // wrap draws
					else {
						if (1 == nHoleCardsIn_str8) { // 0x2F case
							if ((nLowHoleCard_str8 == (nLowEnd_str8+5))||(nHighHoleCard_str8 == (nLowEnd_str8+5))) { 
								_nFeatures |= GUTSHOT; // (g/s + ocs)		
							}								
						} 
						else if (2 == nHoleCardsIn_str8) {
							if (nLowHoleCard_str8 >= (nLowEnd_str8+5)) { // 0x6F case both hole cards high 
								_nFeatures |= GUTSHOT; // (g/s + ocs)		
							}
						}
					}
					if (!(_nFeatures & GUTSHOT)) _nFeatures |= OPENENDED;

					// Some extra low-end str8 cases to consider
					if (1 == nHoleCardsIn_str8) {
						if (nLowHoleCard_str8 == (nLowEnd_str8+1)) _nFeatures |= LOWENDSTR8;
						if (nHighHoleCard_str8 == (nLowEnd_str8+1)) _nFeatures |= LOWENDSTR8;
					}
					break;
				}
				case 0x3D: // LSb 1011110? Msb NOT b011110?
				{
					if (14 == nTopEnd_str8) _nFeatures |= GUTSHOT; // wrap draws
					else _nFeatures |= OPENENDED;
					
					if (1 == nHoleCardsIn_str8) { // 0x4F case
						if ((nLowHoleCard_str8 == nLowEnd_str8)||(nHighHoleCard_str8 == nLowEnd_str8)) { 
							nHoleCardsIn_str8 = 0; // (so will come out as a fake str8 draw)
						}			
					} 
					break;
				}
				case 0x7B: // LSb 1101111? MSb [6 cards] NOT 1b01111? NOT b101111? NOT bb01111?
				{
					if (14 == nTopEnd_str8) _nFeatures |= GUTSHOT; // wrap draws
					else _nFeatures |= OPENENDED;

					if (1 == nHoleCardsIn_str8) {
						if ((nLowHoleCard_str8 == nLowEnd_str8)||(nHighHoleCard_str8 == nLowEnd_str8)) {
							nHoleCardsIn_str8 = 0; // (so will come out as a fake str8 draw)
						}
						if ((nLowHoleCard_str8 == (nLowEnd_str8+1))||(nHighHoleCard_str8 == (nLowEnd_str8+1))) {
							nHoleCardsIn_str8 = 0; // (so will come out as a fake str8 draw)
						}
					}
					else if ((nLowHoleCard_str8 == nLowEnd_str8)&&(nHighHoleCard_str8 == (nLowEnd_str8+1))) {
						nHoleCardsIn_str8 = 0; // (so will come out as a fake str8 draw)							
					}
					break;						
				}
			// 4-outers:
				case 0x17: // LSb 1110100? Msb
				case 0x57: // LSb 1110101? Msb
				case 0x1B: // LSb 1101100? MSb
				case 0x5B: // LSb 1101101? MSb  
				case 0x1D: // LSb 1011100? Msb
				{
					_nFeatures |= GUTSHOT;
					break;
				}
			// Special-case 4-outers
				case 0x37: // LSb 1110110? Msb LOW 1b10110?
				case 0x3B: // LSb 1101110? MSb LOW 1b01110?
				{
					_nFeatures |= GUTSHOT;

					// Some extra low-end str8 cases to consider
					if (1 == nHoleCardsIn_str8) {
						if (nLowHoleCard_str8 == (nLowEnd_str8+1)) _nFeatures |= LOWENDSTR8;
						if (nHighHoleCard_str8 == (nLowEnd_str8+1)) _nFeatures |= LOWENDSTR8;
					}
					break;
				}
				case 0x75: // LSb 1010111? Msb NOT b010111? LOW 10b0111?
				case 0x77: // LSb 1110111? Msb [6 cards] NOT bb10111? 
				{
					_nFeatures |= GUTSHOT;

					// Some extra low-end str8 cases to consider
					if (1 == nHoleCardsIn_str8) {
						if (nLowHoleCard_str8 == (nLowEnd_str8+2)) _nFeatures |= LOWENDSTR8;
						else if (nHighHoleCard_str8 == (nLowEnd_str8+2)) _nFeatures |= LOWENDSTR8;
						else if ((nLowHoleCard_str8 == nLowEnd_str8)||(nHighHoleCard_str8 == nLowEnd_str8)) {
							nHoleCardsIn_str8 = 0; // (so will come out as a fake str8 draw)
						}
					}
					else if ((nLowHoleCard_str8 == nLowEnd_str8)&&(nHighHoleCard_str8 == (nLowEnd_str8 + 1))) {
						nHoleCardsIn_str8 = 0; // (so will come out as a fake str8 draw)
					}
					break;
				}
					
				default: {} // no str8 or str8 draw!
				}
				
				// Found something: there is a "general special" case for 1-card draws with the hole card at the bottom
				if (_nFeatures & ANY_STR8_POSSIBILITIES) {
					if (1 == nHoleCardsIn_str8) {
						_nFeatures |= ONECARDSTR8; // 1-card straights - worry about top/bottom end:
						if (nLowHoleCard_str8 == nLowEnd_str8) _nFeatures |= LOWENDSTR8;
						if (nHighHoleCard_str8 == nLowEnd_str8) _nFeatures |= LOWENDSTR8;
					}
					else if (2 == nHoleCardsIn_str8) _nFeatures |= TWOCARDSTR8;
					// (else fake)
				}
				else { // Second, if not, reset the cluster and continue
					// (note this is duplicated above, so be careful)
					while (nSpan_str8 > 6) {
						do {
							if (nLowHoleCard_str8 == nLowEnd_str8) nHoleCardsIn_str8--;
							if (nHighHoleCard_str8 == nLowEnd_str8) nHoleCardsIn_str8--;
							ucCluster_str8 >>= 1;
							nSpan_str8--;
							nLowEnd_str8++;
						} while (ucCluster_str8 && !(0x1 & ucCluster_str8));

						if (0 == ucCluster_str8) {
							nLowEnd_str8 = nRank;
							nHoleCardsIn_str8 = 0;
							nSpan_str8 = 0;
						}
						// Now update the str8 draw for the newest card
						nTopEnd_str8 = nRank;
						ucCluster_str8 |= (1 << nSpan_str8);
						nHoleCardsIn_str8 += nHole;				
					}
				}
			} // end span of str8 > 6 cards
			
		} // end if looking for str8

		// Pairs, 2-Pairs, Trips, Set
		
		if (nHole) { 
			if (nLowHoleCard_matches) nHighHoleCard_matches = nRank;
			else nLowHoleCard_matches = nRank;
		}
		if (nRank == nLastRank) { // We have a pair or more!

			if ((0 == nRank_matches) || (nRankOf_matches[nRank_matches-1] != nRank)) { // first time
				nRankOf_matches[nRank_matches] = nRank;
				nTable_matches[nRank_matches] = 0;
				nHoleCard_matches[nRank_matches] = 0;

				nRank_matches++;
				nRankOf_matches[nRank_matches] = 0;

				nHoleCard_matches[nRank_matches-1] += nLastHole;
			}
			nHoleCard_matches[nRank_matches-1] += nHole; //(ie 2==wired pair, 1==pair)
			nTable_matches[nRank_matches-1]++; // (1==pair,2==trips,3==quads)
		}

		// Board cards
		if (0 == nHole) {
			if (0 == nLowBoardCard_matches) {
				nLowBoardCard_matches = nRank;
				n2ndBoardCard_matches = nRank;
				nTopBoardCard_matches = nRank;
			}
			else if (nRank > nTopBoardCard_matches) {
				n2ndBoardCard_matches = nTopBoardCard_matches;
				nTopBoardCard_matches = nRank;
			}
		}
		
		// Save state
		nLastHole = nHole;
		nLastRank = nRank;		

		// (Reset state so that we can call again for different hole cards)
		if (1 == nHole) {
			_suitTable[it->first.getSuit()]--; // (reset for next call)
			_boardAndHand.erase(it++);			
		}
		else ++it;
		
	} // end loop over all cards

	// Let's do some analysis of the matches now
	// NOTE: can't use _suitTable or _boardAndHand as they've been reset

	if (!(_nFeatures & MADE_FLUSH) || !(_nFeatures & STRAIGHT)) {
		if (nLowHoleCard_matches > nTopBoardCard_matches) {
			_nFeatures |= TWOOVERCARDS;
		}
		else if (nHighHoleCard_matches > nTopBoardCard_matches) _nFeatures |= ONEOVERCARD;
	}
	// other "no overcard" cases handled below
	
	if (1 == nRank_matches) { // 1 match
		if (1 == nTable_matches[0]) { // pair
			_nFeatures |= PAIR;
			
			if (2 == nHoleCard_matches[0]) {
				
				_nFeatures &= NOOVERCARDS;

				if (nHighHoleCard_matches > nTopBoardCard_matches) {
					_nFeatures |= OVERPAIR;
				}
				else if (nHighHoleCard_matches > n2ndBoardCard_matches) {
					_nFeatures |= SECONDPAIR;					
				}
				else if (nHighHoleCard_matches < nLowBoardCard_matches) {
					_nFeatures |= BOTTOMPAIR;
				}
				else _nFeatures |= MIDDLEPAIR;				
			}
			else if (1 == nHoleCard_matches[0]) {

				_nFeatures &= NOOVERCARDS;

				int nPairOfRank = 0; int nKicker = 0;
				if (nRankOf_matches[0] == nHighHoleCard_matches) {
					nPairOfRank = nHighHoleCard_matches;
					nKicker = nLowHoleCard_matches;
				}
				else if (nRankOf_matches[0] == nLowHoleCard_matches) {
					nPairOfRank = nLowHoleCard_matches;
					nKicker = nHighHoleCard_matches;
				}
				
				if (nPairOfRank == nTopBoardCard_matches) {
					_nFeatures |= TOPPAIR;
				}
				else if (nPairOfRank == n2ndBoardCard_matches) {
					_nFeatures |= SECONDPAIR;					
				}
				else if (nPairOfRank == nLowBoardCard_matches) {
					_nFeatures |= BOTTOMPAIR;
				}
				else _nFeatures |= MIDDLEPAIR;				
				_nFeatures |= CBoardAnalyzer::getKicker(nKicker, nRankOf_matches[0]);
			}
			else if (0 == nHoleCard_matches[0]) {
				// There's no real advantage in treating a fake pair like a pair at all
				// (unlike trips/2-p because of counterfeiting, full-house draws etc), so:
				//_nFeatures |= FAKE;
				_nFeatures &= ~PAIR;
			}
		}
		else if (2 == nTable_matches[0]) {
			if (0 == nHoleCard_matches[0]) {
				_nFeatures |= FAKE|TRIPS;
			}
			else if (1 == nHoleCard_matches[0]) {
				_nFeatures |= TRIPS;
				_nFeatures &= NOOVERCARDS;
				if (nHighHoleCard_matches == nRankOf_matches[0]) {
					_nFeatures |= CBoardAnalyzer::getKicker(nLowHoleCard_matches, nRankOf_matches[0]);
				}
				else _nFeatures |= CBoardAnalyzer::getKicker(nHighHoleCard_matches, nRankOf_matches[0]);
			}
			else if (2 == nHoleCard_matches[0]) {
				_nFeatures |= SET;
				_nFeatures &= NOOVERCARDS;
			}
		}
		else if (3 == nTable_matches[0]) {
			_nFeatures |= QUADS;
			_nFeatures &= NOOVERCARDS;
			if (0 == nHoleCard_matches[0]) {
				_nFeatures |= FAKE;
				_nFeatures |= CBoardAnalyzer::getKicker(nHighHoleCard_matches, nRankOf_matches[0]);
			}
		}
	}
	else if (nRank_matches >= 2) {
		
		int nC1 = nRank_matches - 2, nC2 = nRank_matches - 1; // (ignore the lowest pair/whatever if there are 3) 

		if (3 == nTable_matches[nC1]) { // Quads again
			_nFeatures |= QUADS;
			_nFeatures &= NOOVERCARDS;
			if (0 == nHoleCard_matches[nC1]) {
				_nFeatures |= FAKE;
				_nFeatures |= CBoardAnalyzer::getKicker(nHighHoleCard_matches, nRankOf_matches[nC1]);
			}
		}
		else if (3 == nTable_matches[nC2]) { // Quads again
			_nFeatures |= QUADS;
			_nFeatures &= NOOVERCARDS;
			if (0 == nHoleCard_matches[nC2]) {
				_nFeatures |= FAKE;
				_nFeatures |= CBoardAnalyzer::getKicker(nHighHoleCard_matches, nRankOf_matches[nC1]);
			}
		}
		else if ((1 == nTable_matches[nC1])&&(1 == nTable_matches[nC2])) { // 2-pair
			_nFeatures |= TWOPAIR;

			if ((1 == nHoleCard_matches[nC1])&&(1 == nHoleCard_matches[nC2])) { // "classical"
				_nFeatures &= NOOVERCARDS;
				if ((nHighHoleCard_matches == nTopBoardCard_matches)&&(nLowHoleCard_matches == n2ndBoardCard_matches)) {
					_nFeatures |= TOP_2P;
				}
				else if (nHighHoleCard_matches == nTopBoardCard_matches) {
					_nFeatures |= TOPMIDDLE_2P;
				}
				else if (nLowHoleCard_matches != _boardSummary.nLowCardRank) _nFeatures |= MIDDLE_2P;
			}
			else if (2 == nHoleCard_matches[nC1]) { // there is a paired board above our pair
				_nFeatures |= PAIRED_BOARD;
				_nFeatures &= NOOVERCARDS;
				if (nRankOf_matches[nC2] == nTopBoardCard_matches) {
					if (nHighHoleCard_matches == n2ndBoardCard_matches) _nFeatures |= TOP_2P;
					else _nFeatures |= TOPMIDDLE_2P;
				}
				else {
					_nFeatures |= MIDDLE_2P;										
				}
			}
			else if (2 == nHoleCard_matches[nC2]) { // there is a paired board below our pair
				_nFeatures |= PAIRED_BOARD;
				_nFeatures &= NOOVERCARDS;
				if (nHighHoleCard_matches > nTopBoardCard_matches) {
					if (nRankOf_matches[nC1] == nTopBoardCard_matches) _nFeatures |= TOP_2P;
					else _nFeatures |= TOPMIDDLE_2P;
				}
				else _nFeatures |= MIDDLE_2P;
			}
			else if ((1 == nHoleCard_matches[nC1])||(1 == nHoleCard_matches[nC2])) { // 1 card 2-pair
				_nFeatures |= PAIRED_BOARD;
				_nFeatures &= NOOVERCARDS;
				_nFeatures |= ONECARD;
				if (nRankOf_matches[nC2] == nTopBoardCard_matches) {
					if (nRankOf_matches[nC1] == n2ndBoardCard_matches) _nFeatures |= TOP_2P;
					else _nFeatures |= TOPMIDDLE_2P;
				}
				else {
					_nFeatures |= MIDDLE_2P;										
				}
				_nFeatures |= CBoardAnalyzer::getKicker(nHighHoleCard_matches, nRankOf_matches[nC1]);
			}
			else { // no cards from the hand
				_nFeatures |= FAKE;
			}
		}
		else { // Must be a full house by elimination
			_nFeatures |= FULLHOUSE;
			_nFeatures &= NOOVERCARDS;

			if ((0 == nHoleCard_matches[nC1])&&(0 == nHoleCard_matches[nC2])) {
				_nFeatures |= FAKE;
			}
			else if ((2 == nTable_matches[nC2])&&(0 == nHoleCard_matches[nC2])) {
				_nFeatures |= UNDERFILL; // eg 999,7 we hold one or more 7s
			}			
		}
	}
	
	// Tidy up conflicting features
	
	if (_nFeatures & (FULLHOUSE|QUADS)) {
		_nFeatures &= ~ANY_DRAW; 
	}
	else if (_nFeatures & (MADE_FLUSH|STRAIGHT)) {
		if (_nFeatures & WEAKFEATURES) {
			_nFeatures &= ~WEAKFEATURES;
			_nFeatures &= ~FAKE; // (shared between 2-pair and fullhouse+)
		}
	}
	if (_nFeatures & MADE_FLUSH) {
		if (_nFeatures & STRAIGHT) {

			// Is is a straight flush? 
			bool bStr8Flush = false;			

			if ((5 == nBoardAndHandSize)||(nRank_matches > 0)) { // 5 effective cards => done
				bStr8Flush = true;
			}
			else { // eg 2c,3c,4c,5c,6d,Qc not a str8 flush...
				if (6 == nSuitSize) {
					bStr8Flush = true;
				}
				else { // 5 card flush ... there may be one odd man out
					
					if ((14 == nRank_str8fl) && (1 == nLowEnd_str8)) nRank_str8fl = 1; // use A as low

					if (0 != nRank_str8fl) {

						switch (ucCluster_str8) {
						case 0x1F: // LSB 1111100? MSb
						{
							if ((nRank_str8fl > nTopEnd_str8)||(nRank_str8fl < nLowEnd_str8)) bStr8Flush = true;
							break;
						}
						case 0x7D: // LSb 1011111? MSb [6 cards]
						{
							if (nRank_str8fl == nLowEnd_str8) bStr8Flush = true;
							break;
						}
						case 0x5F: // LSb 1111101? MSb [6 cards]
						{
							if (nRank_str8fl == nTopEnd_str8) bStr8Flush = true;
							break;
						}
						case 0x3F: // LSb 1111110? MSb [6 cards] 
						{
							if ((nRank_str8fl == nTopEnd_str8)||(nRank_str8fl == nLowEnd_str8)) bStr8Flush = true;
							break;
						}
						default:
						{
							printf("Internal logic error %x\n", ucCluster_str8); 
							exit (-1);
						}
						}
					}
				}
			}			
			if (bStr8Flush)
			{
				if ((_nFeatures & ONECARDSTR8)&&(_nFeatures & LOWENDSTR8)) {
					_nFeatures = STR8FLUSH|ONECARDSTR8|LOWENDSTR8;
				}
				else _nFeatures = STR8FLUSH; // (note overwrites all other features)
			}
			else _nFeatures &= ~ANY_STR8_POSSIBILITIES; 
		}
		else _nFeatures &= ~ANY_STR8_POSSIBILITIES;
	}
}

////////////////////////////////////////////////////

int CBoardAnalyzer::categorizeHand(int nFeatures)
{
	// Count monster hands - str8 flush, flush, straight, set, trips, "classic two pair"	
	if ((nFeatures & CBoardAnalyzer::STR8FLUSH)||(nFeatures & CBoardAnalyzer::QUADS)||(nFeatures & CBoardAnalyzer::FULLHOUSE)||
			(nFeatures & CBoardAnalyzer::MADE_FLUSH)||(nFeatures & CBoardAnalyzer::STRAIGHT)||(nFeatures & CBoardAnalyzer::SET)||
			(nFeatures & CBoardAnalyzer::TRIPS))
	{
		if (!(nFeatures & CBoardAnalyzer::FAKE)) {
			return MONSTERS;
		}
	}
	if ((nFeatures & CBoardAnalyzer::TWOPAIR)&&!(nFeatures & CBoardAnalyzer::PAIRED_BOARD)) {
		if (!(nFeatures & CBoardAnalyzer::FAKE)) {
			return MONSTERS;
		}
	}
	// Count strong draws - oesd+fl, g/s+fl, pair+fl, strong fl+1-2ocs, str+2ocs, pair+str8
	if ((nFeatures & CBoardAnalyzer::FLUSH_DRAW) && 
			((nFeatures & CBoardAnalyzer::PAIR)||(nFeatures & CBoardAnalyzer::TWOPAIR)))
	{
		if (!(nFeatures & CBoardAnalyzer::FAKE)) {
			return STRONG_DRAWS;
		}
	}
	if ((nFeatures & CBoardAnalyzer::FLUSH_DRAW) &&  
		((nFeatures & CBoardAnalyzer::OPENENDED)||(nFeatures & CBoardAnalyzer::GUTSHOT)||(nFeatures & CBoardAnalyzer::BELLYBUSTER)))
	{
		if (nFeatures & (CBoardAnalyzer::ONECARDSTR8|CBoardAnalyzer::TWOCARDSTR8)) {
			return STRONG_DRAWS;
		}
	}
	if ((nFeatures & CBoardAnalyzer::STRONG_FLUSH) && 
			((nFeatures & CBoardAnalyzer::ONEOVERCARD)||(nFeatures & CBoardAnalyzer::TWOOVERCARDS)))
	{
		return STRONG_DRAWS;
	}
	if (((nFeatures & CBoardAnalyzer::OPENENDED)||(nFeatures & CBoardAnalyzer::BELLYBUSTER)) &&
			((nFeatures & CBoardAnalyzer::TWOOVERCARDS)||
					((nFeatures & CBoardAnalyzer::PAIR)&&!(nFeatures & CBoardAnalyzer::FAKE))||
					((nFeatures & CBoardAnalyzer::TWOPAIR)&&!(nFeatures & CBoardAnalyzer::FAKE))))
	{
		if (nFeatures & (CBoardAnalyzer::ONECARDSTR8|CBoardAnalyzer::TWOCARDSTR8)) {
			return STRONG_DRAWS;
		}
	}
	// Count strong hands - overpair, top pair, second pair (pp or good/top kicker), some other 2 pairs
	if ((nFeatures & CBoardAnalyzer::OVERPAIR)||(nFeatures & CBoardAnalyzer::TOPPAIR)) {

		if (!(nFeatures & CBoardAnalyzer::FAKE)) {
			return STRONG_MADE;
		}
	}
	if ((nFeatures & CBoardAnalyzer::SECONDPAIR)&&!(nFeatures & CBoardAnalyzer::WEAKKICKER)) {
		if (!(nFeatures & CBoardAnalyzer::FAKE)) {
			return STRONG_MADE;
		}
	}
	if ((nFeatures & CBoardAnalyzer::TOP_2P)||(nFeatures & CBoardAnalyzer::TOPMIDDLE_2P)) {
		return STRONG_MADE;
	}
	// Count mediocre draws - fl, oesd, pair+g/s, 2ocs
	if ((nFeatures & CBoardAnalyzer::OPENENDED)||(nFeatures & CBoardAnalyzer::BELLYBUSTER)) {
		if (nFeatures & (CBoardAnalyzer::ONECARDSTR8|CBoardAnalyzer::TWOCARDSTR8)) {
			return MEDIOCRE_DRAWS;
		}
	}
	if (nFeatures & CBoardAnalyzer::FLUSH_DRAW) {
		return MEDIOCRE_DRAWS;
	}
	if ((nFeatures & CBoardAnalyzer::GUTSHOT)&&
			((nFeatures & CBoardAnalyzer::PAIR)||(nFeatures & CBoardAnalyzer::TWOOVERCARDS)))
	{
		if ((!(nFeatures & CBoardAnalyzer::FAKE)) 
				&& (nFeatures & (CBoardAnalyzer::ONECARDSTR8|CBoardAnalyzer::TWOCARDSTR8)))
		{
			return MEDIOCRE_DRAWS;
		}
	}
	if ((nFeatures & CBoardAnalyzer::GUTSHOT) && (nFeatures & CBoardAnalyzer::ONEOVERCARD))
	{
		return WEAK_DRAWS;		
	}
	if (nFeatures & CBoardAnalyzer::TWOOVERCARDS) {
		return WEAK_DRAWS;
	}
	// Count mediocre hands - second pair (med/weak kicker), middle pair, bottom pair
	if ((nFeatures & CBoardAnalyzer::PAIR)||(nFeatures & CBoardAnalyzer::TWOPAIR)) {
		if (!(nFeatures & CBoardAnalyzer::FAKE)) {
			return MEDIOCRE_MADE;
		}
	}
	// Count "air"
	return RUBBISH;
}

////////////////////////////////////////////////////

// Hand range:

static const char* pcPreflopHands_[] = {
		// Premium hands:
		"AA", "KK", "QQ", "AK", "JJ", "TT", "AQs", "99", "AJs", "AQo", // 5.4%
		// Medium pairs and decent suited aces (+KQs,AJo):
		"88", "77", "66", "ATs", "KQs", "AJo", "A9s", "A8s", "A7s", "A6s", // 9.5%
		// Other decent big hands, and the rest of the pairs:
		"KJs", "55", "QJs", "44", "KQo", "33", "ATo", "22", "KTs", "JTs", // 14.3% 
		// The rest of the suited aces:
		"A5s", "A4s", "A3s", "A2s", // 15.5%
		// The middle suited connectors:
		"QTs", "J9s", "T9s", "98s", "87s", "76s", "54s", "T8s", "97s", "86s", "75s", // 19.1%				
		// And some more big hands:
		"KJo", "A9o", "K9s", "Q9s", // 22.5%
		// Going down:
		"A8o", "KTo", "K8s", "QTo", "Q8s", "JTo", "J8s", "K9o", "Q9o", "A7o", "J9o", // 30.9%		
		// More vaguely connected stuff, and some weak Ks, and Ao:
		"A6o", "64s", "T9o", "98o", "A5o", "K7s", "K6s", "A4o", "A3o", "A2o", // 38.1%
		// Then everything else in some sort of order:
		"K5s", "K4s", "K3s", "K2s", "Q7s", "K8o", "K7o", "Q8o",  
		"96s", "T8o", "87o", "85s", "97o", "76o", "65o", "Q6s", "Q5s", "J6s", // ~49.6%
		"J8o", "86o", "75o", // 49.3% 
		// More suited!
		"Q4s", "Q3s", "Q2s", "J5s", "J4s", "J3s", "J2s", "T6s", "T5s", "T4s", "T3s", "T2s", // 54.2%
		// OK some more big hands:
		"K6o", "Q7o", "J7o", "T7o", // 58.9%
		// The rest of the playable suited:
		"95s", "84s", "74s", "63s", "53s", "43s", // 60.5%
		// And some last off suit trash:
		"K5o", "Q6o", "K4o", "96o", "K3o", "Q5o", // 65.9% enough!!
		// Other stuff until I get bored
		"K2o", "Q4o", "Q3o", "J6o", "J5o", "T6o", "T5o", // >71.4%
		"83s", "82s", "73s", "72s", "62s", "52s", "42s", "32s", 0 
};

// Some other hands ranges
//TODO: these don't seem right?

static const char* szPreflop6m_standard_open_UTG_ = "22+, AJ+, ATs-A9s, KTs+, QTs+, JTs, T9s, 98s, 87s, 76s, 65s";
static const char* szPreflop6m_standard_open_MP_ = "22+, AJ+, ATs-A7s, KQ, KJs-K9s, Q9s+, J9s+, T9s, 98s, 87s, 76s, 65s, 54s";
static const char* szPreflop6m_standard_open_CO_ = "22+, AT+, A9s-A2s, KT+, K9s-K6s, QJ, QTs-Q8s, J8s+, T8s+, 97s+, 86s+, 75s+, 64s+, 53s+, 43s";
static const char* szPreflop6m_standard_open_BTN_ = "22+, A2+, K9+, K8s-K2s, Q9+, Q8s-Q4s, J9+, J8s-J7s, T9, T8s-T7s, 98, 97s-96s, 87, 86s-85s, 74s+, 63s+, 53s+, 43s";
static const char* szPreflop6m_standard_open_SB_ = "22+, A2+, K9+, K8s-K2s, Q9+, Q8s-Q4s, J9+, J8s-J7s, T9, T8s-T7s, 98, 97s-96s, 87, 86s-85s, 74s+, 63s+, 53s+, 43s";

static const char* szPreflop6m_standard_ccall_vUTG_ = "88-22, 99:0.5, AJ, ATs-A9s, AQ:0.5, A8s:0.5, A7s:0.5, A6s:0.5, A5s:0.5, A4s:0.5, A3s:0.5, A2s:0.5, KTs+, QTs+, JTs, J9s:0.5, T9s:0.5, T8s:0.5, 98s:0.5, 97s:0.5, 87s:0.5, 86s:0.5, 76s:0.5, 75s:0.5, 65s:0.5";
static const char* szPreflop6m_standard_ccall_vMP_ = "88-22, 99:0.5, AJ, ATs-A7s, A6s:0.5, A5s:0.5, A4s:0.5, A3s:0.5, A2s:0.5, AQ:0.5, KQ, KJs-K9s, Q9s+, J9s+, T9s:0.5, T8s:0.5, 98s:0.5, 97s:0.5, 87s:0.5, 86s:0.5, 76s, 75s:0.5, 65s:0.5, 64s:0.5, 54s:0.5";
static const char* szPreflop6m_standard_ccall_vCO_ = "88-22, 99:0.5, AT, A9s-A2s, AQ:0.5, AJ:0.5, KT+, K9s-K6s, QJ, QTs-Q8s, J8s+, T8s+, 97s+, 86s+, 75s+, 64s+, 53s+, 43s";

static const char* szPreflopHU_standard_ccall_vSB_ = "88-22, A9-A2+, KJ-K9, K8s-K2s, Q9+, Q8s-Q4s, J9+, J8s-J7s, T9, T8s-T7s, 98, 97s-96s, 87, 86s-85s, 74s+, 63s+, 53s+, 43s";

static const char* szPreflopHU_standard_open_SB_ = "22+, A2+, K2+, Q4+, Q3s-Q2s, J7+, J6s-J2s, T7+, T6s-T2s, 96+, 95s-92s, 85+, 84s-82s, 75+, 74s-72s, 64+, 63s-62s, 53+, 52s, 43, 42s, 32s";

static const char* szPreflopAllHands = "22+,A2+,K2+,Q2+,J2+,T2+,92+,82+,72+,62+,52+,42+,32";

std::map<std::string, std::string> CBoardAnalyzer::_handRangeByName;

////////////////////////////////////////////////////

int CBoardAnalyzer::assignHandFromRange(const std::string& sName, std::string& sHandRange)
{
	// First time through, allocate some ranges:
	if (_handRangeByName.empty()) {
		_handRangeByName["6MUTGO"] = std::string(szPreflop6m_standard_open_UTG_);
		_handRangeByName["6MMPO"] = std::string(szPreflop6m_standard_open_MP_);
		_handRangeByName["6MCOO"] = std::string(szPreflop6m_standard_open_CO_);
		_handRangeByName["6MBO"] = std::string(szPreflop6m_standard_open_BTN_);
		_handRangeByName["6MSBO"] = std::string(szPreflop6m_standard_open_SB_);
		_handRangeByName["6MCvsUTG"] = std::string(szPreflop6m_standard_ccall_vUTG_);
		_handRangeByName["6MCvsMP"] = std::string(szPreflop6m_standard_ccall_vMP_);
		_handRangeByName["6MCvsCO"] = std::string(szPreflop6m_standard_ccall_vCO_);
		
		_handRangeByName["HUCvsSB"] = std::string(szPreflopHU_standard_ccall_vSB_);
		_handRangeByName["HUSBO"] = std::string(szPreflopHU_standard_open_SB_);
		_handRangeByName["ALL"] = std::string(szPreflopAllHands);
	}
	
	std::map<std::string, std::string>::iterator mit = _handRangeByName.find(sName);
	if (_handRangeByName.end() != mit) {
		sHandRange = mit->second;
	}	
	else return 0;
	
	return 0; // (work out what the % of hands is during the later CCard::addCard() calls)
}

////////////////////////////////////////////////////

std::string CBoardAnalyzer::DisplayPreFlopRanges()
{	
	std::string sDisp;
	CBoardAnalyzer::assignHandFromRange(std::string(""), sDisp);
	std::map<std::string, std::string>::iterator mit;
	
	for (mit = _handRangeByName.begin(); mit != _handRangeByName.end();  ++mit) {
		sDisp += mit->first;
		sDisp += ", ";
	}
	return sDisp;	
}
////////////////////////////////////////////////////

int CBoardAnalyzer::assignHandFromRange(int nPct, int nPct_max, std::deque<CCard::Hole>& lHandRange, std::string& sHandList)
{
	const char** psz = &pcPreflopHands_[0]; 
	int nTotalHands = 52*51/2; // (1125)
	nPct_max = (nPct_max*nTotalHands)/100; // (convert to a number of hands)
	nPct = (nPct*nTotalHands)/100 - nPct_max;
	for (; *psz && ((int)lHandRange.size() <= nPct); psz++) {
		CCard::addHand(*psz, lHandRange);
		if (nPct_max > 0) { // discard hands at the top of the range
			int nHandsToRemove = lHandRange.size();
			nPct_max -= nHandsToRemove;
			lHandRange.clear();
			if (nPct_max  < 0) { // if we've taken too much off, add back at the top
				nPct += nPct_max;
				nPct_max = 0;
			}
		}
		else {
			sHandList += *psz;
			sHandList += " ";
		}
	}
	return (100*lHandRange.size())/nTotalHands;
}

////////////////////////////////////////////////////

double CBoardAnalyzer::calculateEquity(const CCard::Hole& h1, const CCard::Hole& h2, const int nBoardTable[], int nCardsOnBoard)
{
	int npockets = 2, err, orderflag = 0;
	
	StdDeck_CardMask pockets[2];
	StdDeck_CardMask board;
	StdDeck_CardMask dead;
	enum_result_t result;

	StdDeck_CardMask_RESET(dead);
	StdDeck_CardMask_RESET(board);
	StdDeck_CardMask_RESET(pockets[0]);
	StdDeck_CardMask_RESET(pockets[1]);

	StdDeck_CardMask_SET(pockets[0], h1.first.convertToPokerEnum());
	StdDeck_CardMask_SET(pockets[0], h1.second.convertToPokerEnum());
	StdDeck_CardMask_SET(dead, h1.first.convertToPokerEnum());
	StdDeck_CardMask_SET(dead, h1.second.convertToPokerEnum());

	StdDeck_CardMask_SET(pockets[1], h2.first.convertToPokerEnum());
	StdDeck_CardMask_SET(pockets[1], h2.second.convertToPokerEnum());
	StdDeck_CardMask_SET(dead, h2.first.convertToPokerEnum());
	StdDeck_CardMask_SET(dead, h2.second.convertToPokerEnum());

	for (int i = 0; i < nCardsOnBoard; ++i) {
	
		CCard newCard(nBoardTable[i]);
		StdDeck_CardMask_SET(board, newCard.convertToPokerEnum());
		StdDeck_CardMask_SET(dead, newCard.convertToPokerEnum());
	}	
    enumResultClear(&result);
    err = enumExhaustive(game_holdem, pockets, board, dead, npockets, nCardsOnBoard, orderflag, &result);

    double dEq; 
    if (0 == err) {
    	dEq = result.ev[0]/result.nsamples;
    }
    else dEq = 0.0;
#if 0
    if (err) {
        printf("enumeration function failed err=%d\n", err);
    } else {
        enumResultPrint(&result, pockets, board);        
    }
#endif //0    
    return dEq;	
}

////////////////////////////////////////////////////


//TODO: might want to make this faster with another table that gives the flop number

void CBoardAnalyzer::resetEquityCache()
{
	if (0 == _nEquityCache_level) {
		::memset((void*)&_nEquityCache_levelTable, 0x0, 225*225*sizeof(unsigned int));
	}
	_nEquityCache_level++;
}

////////////////////////////////////////////////////

double CBoardAnalyzer::approximateEquity(const CBoardAnalyzer& h1Px, const CBoardAnalyzer& h2Px,
		const CCard::Hole& h1, const CCard::Hole& h2, const int nBoardTable[], int nCardsOnBoard)
{
// Some interesting facts (I think)
// If you look at the ordered set of flops, eg AAA, AA2, AA3, ...A2A,...etc: there's 13*12*11=1716 variants
	// (not taking suits into account)
// Then apply suits: MT(+1), 2T(+3), RB(~+1) ... that's 5*1716=8630 flops only	
// Which suits doesn't matter at all, since all of our ranges are just "suited" or "unsuited" 
	// (so at least, when randomizing flops, make sure you pick different ranks or you've wasted a sample)

	//TODO: here's the plan ... write a version of this with a big look-up table for hole-cards,hole-cards *NOT* including suits
	// then for each hand, get the equity (calculating it with a suit-less version of calcuateEquity), then adjust
	// for flush draws, taking into account full-house+ possibilities (might have to not worry about str8 flushes!)
	// This should be ~ (4*4) * (10*10) == 1600 times faster?!
	// (and then x2 because the equity tables are symmetric)
	
	double dEq = 0.0;

	int nIndex1_h = 15*h1.first.getRank() + h1.second.getRank();
	int nIndex1_v = 15*h2.first.getRank() + h2.second.getRank();

//TODO ... convert suit to something other than hearts (which is what I'm forcing the final two cards to be)
// eg hole cards to club,club and board to diamond,diamond
	if (_nEquityCache_levelTable[nIndex1_h][nIndex1_v] != _nEquityCache_level) {

		// Call the "no suit" version of holdem I've added

		int npockets = 2, err, orderflag = 0;
		
		StdDeck_CardMask pockets[2];
		StdDeck_CardMask board;
		StdDeck_CardMask dead;
		enum_result_t result;

		StdDeck_CardMask_RESET(dead);
		StdDeck_CardMask_RESET(board);
		StdDeck_CardMask_RESET(pockets[0]);
		StdDeck_CardMask_RESET(pockets[1]);

		StdDeck_CardMask_SET(pockets[0], h1.first.convertToPokerEnum());
		StdDeck_CardMask_SET(pockets[0], h1.second.convertToPokerEnum());
//		StdDeck_CardMask_SET(dead, h1.first.convertToPokerEnum());
//		StdDeck_CardMask_SET(dead, h1.second.convertToPokerEnum());

		StdDeck_CardMask_SET(pockets[1], h2.first.convertToPokerEnum());
		StdDeck_CardMask_SET(pockets[1], h2.second.convertToPokerEnum());
//		StdDeck_CardMask_SET(dead, h2.first.convertToPokerEnum());
//		StdDeck_CardMask_SET(dead, h2.second.convertToPokerEnum());

		for (int i = 0; i < nCardsOnBoard; ++i) {
		
			CCard newCard(nBoardTable[i]);
			StdDeck_CardMask_SET(board, newCard.convertToPokerEnum());
//			StdDeck_CardMask_SET(dead, newCard.convertToPokerEnum());
		}		
	    enumResultClear(&result);
	    err = enumExhaustive(game_holdem_nosuits, pockets, board, dead, npockets, nCardsOnBoard, orderflag, &result);

	    if (0 == err) {
	    	dEq = result.ev[0]/result.nsamples;
	    	_dEquityCache_noSuit[nIndex1_h][nIndex1_v] = dEq;
	    	_dEquityCache_noSuit[nIndex1_v][nIndex1_h] = 1.0 - dEq;
	    	_nEquityCache_levelTable[nIndex1_h][nIndex1_v] = _nEquityCache_level;
	    	_nEquityCache_levelTable[nIndex1_v][nIndex1_h] = _nEquityCache_level;
	    }
	    else dEq = 0.0;

	    if (err) {
	        printf("enumeration function failed err=%d\n", err);
	    } 
	}
	else dEq = _dEquityCache_noSuit[nIndex1_h][nIndex1_v];
	
	return dEq;
}
/*
enum { 
	STATIC_HIGHCARD_BASE = 0, 	// (4 values)
	STATIC_PAIR_BASE = 4,		// (6 values)
	STATIC_2PAIR_BASE = 10,		// (3 values)
	STATIC_3KIND_BASE = 12,		// (2 values)
	STATIC_STR8_BASE = 15,		// (2 values)
	STATIC_FLUSH_BASE = 17,		// (3 values)
	STATIC_FH_BASE = 20,		// (2 values)
	STATIC_QUADS_BASE = 22,		// (1 value)
	STATIC_STR8FL_BASE = 23,	// (1 value)
	STATIC_MAX = 24
};
*/
//TODO: this should probably depend on the board texture, eg paired boards should be different, 3fl board should have more values
//TODO: also need to calculate the draws...

int CBoardAnalyzer::calculateHandStrength(int nFeatures)
{
	int nOffsetOnBase = 0;
	int nVal = -1;
	if (nFeatures & PAIR) {
		//TODO need to assign categorization based on frequency, ie size of pair + ways it can happen (ie based on board?)
		// With no board cards
		// (something like 22-AA: 12 cards each, 
		
		if (nFeatures & BOTTOMPAIR) {
			if (nFeatures & (TOPKICKER|GOODKICKER)) nOffsetOnBase = 1;
			nVal = STATIC_PAIR_BASE + nOffsetOnBase; // (0,1)
		}
		else if (nFeatures & MIDDLEPAIR) {
			if (nFeatures & TOPKICKER) nOffsetOnBase = 1;
			nVal = STATIC_PAIR_BASE + 2 + nOffsetOnBase; // (2,3)
		}
		else if (nFeatures & SECONDPAIR) {
			if (nFeatures & TOPKICKER) nOffsetOnBase = 1;
			nVal = STATIC_PAIR_BASE + 3 + nOffsetOnBase; // (3,4)
		}
		else if (nFeatures & TOPPAIR) {
			if (nFeatures & TOPKICKER) nOffsetOnBase = 1;
			nVal = (STATIC_2PAIR_BASE - 2 + nOffsetOnBase); // (4,5)
		}
		else if (nFeatures & OVERPAIR) {
			nVal = STATIC_2PAIR_BASE - 1; // (5)
		}
		//TODO drawing to 2P+ or set?
	}
	else if (nFeatures & TWOPAIR) {
		if (nFeatures & TOP_2P) {
			nVal = STATIC_2PAIR_BASE + 2; // (2)
		}
		else if (nFeatures & (TOPMIDDLE_2P|MIDDLE_2P)) {
			nVal = STATIC_2PAIR_BASE + 1; // (1)
		}
		else { // bottom?
			nVal = STATIC_2PAIR_BASE; // (0)
		}
	}
	else if (nFeatures & TRIPS) {
		if (nFeatures & TOPKICKER) nOffsetOnBase = 1;
		else if (nFeatures & GOODKICKER) nOffsetOnBase = 1;
		else nOffsetOnBase = 0;
		
		nVal = STATIC_3KIND_BASE + nOffsetOnBase;
	}
	else if (nFeatures & SET) {
		if ((_hand[0].getRank() == _boardSummary.nHighCardRank)||(_hand[0].getRank() == _boardSummary.nSecondCardRank))
		{
			nVal = STATIC_3KIND_BASE + 1;
		}
		else { 
			nVal = STATIC_3KIND_BASE + 0; 													
		}		
	}
	else if (nFeatures & STRAIGHT) {
		if ((nFeatures & ONECARDSTR8) || (nFeatures & LOWENDSTR8)) {
			nVal = STATIC_STR8_BASE;
		}
		else nVal = STATIC_STR8_BASE + 1;
	}
	else if (nFeatures & MADE_FLUSH) {
		if (nFeatures & WEAK_FLUSH) nVal = STATIC_FLUSH_BASE;
		else if (nFeatures & MEDIUM_FLUSH) nVal = STATIC_FLUSH_BASE + 1;
		else nVal = STATIC_FLUSH_BASE + 2;
	}
	else if (nFeatures & FULLHOUSE) {
		if (nFeatures & UNDERFILL) nVal = STATIC_FH_BASE;
		else nVal = STATIC_FH_BASE + 1;
	}
	else if (nFeatures & QUADS) {
		nVal = STATIC_QUADS_BASE;
	}
	else if (nFeatures & STR8FLUSH) {
		nVal = STATIC_STR8FL_BASE;		
	}
	else { // High card...
		if (14 == _boardSummary.nHighCardRank) {
			if (_hand[1].getRank() > _boardSummary.nSecondCardRank) {
				nVal = STATIC_HIGHCARD_BASE + 2;				
			}
			else if (_hand[0].getRank() > _boardSummary.nSecondCardRank) {
				nVal = STATIC_HIGHCARD_BASE + 1;								
			}
			else {
				nVal = STATIC_HIGHCARD_BASE;							
			}
		}
		else if (13 == _boardSummary.nHighCardRank) {
			if (14 == _hand[0].getRank()) {
				if (_hand[1].getRank() > _boardSummary.nSecondCardRank) {
					nVal = STATIC_HIGHCARD_BASE + 3;					
				}
				else {
					nVal = STATIC_HIGHCARD_BASE + 2;										
				}
			}
			else {
				if (_hand[0].getRank() > _boardSummary.nSecondCardRank) {
					nVal = STATIC_HIGHCARD_BASE + 1;					
				}
				else {
					nVal = STATIC_HIGHCARD_BASE + 0;										
				}				
			}			
		}
		else {
			if (nFeatures & TWOOVERCARDS) {
				nVal = STATIC_HIGHCARD_BASE + 3;
			}
			else if (nFeatures & ONEOVERCARD) {
				nVal = STATIC_HIGHCARD_BASE + 1;
				if (_hand[1].getRank() > _boardSummary.nSecondCardRank) {
					nVal++;
				}
			}
			else nVal = STATIC_HIGHCARD_BASE;
		}
		//TODO return some information for draws (drawing to TP or SP?)
	}
	return nVal;
}

////////////////////////////////////////////////////

// Some post-flop ranges vs activities

// 1. Model PF caller:
// 1.1. Play draws / strong hands; [eg raising range]
// 1.2. Play medium hands / overcards (complement of above, minus "rubbish") [eg calling range]
// 1.3. Play any sort of hand (union of above two categories)
// 1.4. Balanced raiser: (see below)
// 1.5. Balanced caller: (see below)

// 2. Model PF raiser vs check-raise:
// 2.1. Play strong hands + strong draws [eg 3-bet]
// 2.2. Play good hands + decent draws [eg call]
// 2.3. Balanced 3-bettor: (see below)
// 2.4. Balanced caller: (see below)

//TODO different ranges depending on fl draw possibilities

static const int	_pnDrawOrStrongMade_caller[] = 		{ CBoardAnalyzer::WEAK_DRAWS, CBoardAnalyzer::MEDIOCRE_DRAWS, CBoardAnalyzer::MONSTERS, CBoardAnalyzer::STRONG_MADE, CBoardAnalyzer::STRONG_DRAWS, -1 }; 
static const double _pdDrawOrStrongMade_caller[] = 		{ 0.2, 1.0, 1.0, 0.75, 0.5, 0.0 };
static const int	_pnMediumMadeHands_caller[] = 		{ CBoardAnalyzer::MEDIOCRE_MADE, CBoardAnalyzer::STRONG_MADE, -1 }; 
static const double	_pdMediumMadeHands_caller[] = 		{ 1.0, 0.25, 0.0 }; 

static const int 	_pnStrongDrawOrMadeHands_opener[] = { CBoardAnalyzer::MEDIOCRE_DRAWS, CBoardAnalyzer::MONSTERS, CBoardAnalyzer::STRONG_DRAWS, -1 };
static const double _pdStrongDrawOrMadeHands_opener[] = { 1.0, 1.0, 1.0, 0.0 };
static const int	_pnMediumHands_opener[] = 			{ CBoardAnalyzer::MEDIOCRE_MADE, CBoardAnalyzer::MEDIOCRE_DRAWS, -1 }; 
static const double _pdMediumHands_opener[] = 			{ 1.0, 1.0, 0.0 }; 

static const int	_pnAllHands[] = 			{ CBoardAnalyzer::MONSTERS , CBoardAnalyzer::STRONG_DRAWS , CBoardAnalyzer::STRONG_MADE , CBoardAnalyzer::MEDIOCRE_DRAWS , CBoardAnalyzer::MEDIOCRE_MADE , CBoardAnalyzer::RUBBISH , -1 }; 
static const double _pdAllHands[] = 			{ 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0 }; 
static const int	_pnNotAirHands[] = 			{ CBoardAnalyzer::MONSTERS , CBoardAnalyzer::STRONG_DRAWS , CBoardAnalyzer::STRONG_MADE , CBoardAnalyzer::MEDIOCRE_DRAWS , CBoardAnalyzer::MEDIOCRE_MADE , -1 }; 
static const double _pdNotAirHands[] = 			{ 1.0, 1.0, 1.0, 1.0, 1.0, 0.0 };

static const int	_pnBalanced6MCheckRaise_caller[] = 	{ CBoardAnalyzer::MONSTERS, CBoardAnalyzer::STRONG_DRAWS, CBoardAnalyzer::STRONG_MADE, CBoardAnalyzer::MEDIOCRE_MADE, CBoardAnalyzer::MEDIOCRE_DRAWS, CBoardAnalyzer::WEAK_DRAWS, -1 }; 
static const double _pdBalanced6MCheckRaise_caller[] = 	{ 0.75, 0.5, 0.5, 0.0, 0.75, 0.2, 0.0 };
static const int	_pnBalanced6MCheckCall_caller[] = 	{ CBoardAnalyzer::MONSTERS, CBoardAnalyzer::STRONG_DRAWS, CBoardAnalyzer::STRONG_MADE, CBoardAnalyzer::MEDIOCRE_MADE, CBoardAnalyzer::MEDIOCRE_DRAWS, CBoardAnalyzer::WEAK_DRAWS, -1 }; 
static const double _pdBalanced6MCheckCall_caller[] = 	{ 0.25, 0.5, 0.5, 1.0, 0.25, 0.1, 0.0 };

static const int	_pnBalanced6MBetCall_opener[] = 	{ CBoardAnalyzer::MONSTERS, CBoardAnalyzer::STRONG_DRAWS, CBoardAnalyzer::STRONG_MADE, CBoardAnalyzer::MEDIOCRE_MADE, CBoardAnalyzer::MEDIOCRE_DRAWS, CBoardAnalyzer::WEAK_DRAWS, -1 }; 
static const double _pdBalanced6MBetCall_opener[] = 	{ 0.25, 0.5, 0.5, 0.0, 0.75, 0.1, 0.0 };

////////////////////////////////////////////////////

CBoardAnalyzer::HandRangeCache CBoardAnalyzer::_handRangeCache;
std::map<std::string, std::map<int, double> > CBoardAnalyzer::_mHandRangeMap;

static void loadRange(const int *pnTable, const double *pdTable, std::map<int, double>& mRange)
{
	for (; *pnTable != -1; pnTable++) {
		mRange[*pnTable] = *pdTable++;
	}
}

double CBoardAnalyzer::GetHandRangeWeighting(const std::string& sRangeName, int nHandCategory)
{
	// First time through, load hands
	if (_mHandRangeMap.empty()) {
		::loadRange(_pnDrawOrStrongMade_caller, _pdDrawOrStrongMade_caller, _mHandRangeMap["DR_STR_C"]);
		::loadRange(_pnMediumMadeHands_caller, _pdMediumMadeHands_caller, _mHandRangeMap["MED_C"]);

		::loadRange(_pnStrongDrawOrMadeHands_opener, _pdStrongDrawOrMadeHands_opener, _mHandRangeMap["DR_STR_O"]);
		::loadRange(_pnMediumHands_opener, _pdMediumHands_opener, _mHandRangeMap["MED_O"]);
		
		::loadRange(_pnBalanced6MCheckRaise_caller, _pdBalanced6MCheckRaise_caller, _mHandRangeMap["BAL6M_KR"]);
		::loadRange(_pnBalanced6MCheckCall_caller, _pdBalanced6MCheckCall_caller, _mHandRangeMap["BAL6M_KC"]);
		::loadRange(_pnBalanced6MBetCall_opener, _pdBalanced6MBetCall_opener, _mHandRangeMap["BAL6M_BC"]);

		::loadRange(_pnAllHands, _pdAllHands, _mHandRangeMap["ALL"]);
		::loadRange(_pnNotAirHands, _pdNotAirHands, _mHandRangeMap["NOTAIR"]);
	}
	// Check cache
	const std::map<int, double>* pmRange;
	if (sRangeName.c_str() == _handRangeCache.szRangeName) {
		pmRange = _handRangeCache.pmRange;
	}
	else {
		pmRange = &_mHandRangeMap[sRangeName];		
		if (pmRange->empty()) {
			_mHandRangeMap.erase(sRangeName);
			return -1.0;
		}
		_handRangeCache.szRangeName = sRangeName.c_str();
		_handRangeCache.pmRange = pmRange;
	}
	std::map<int, double>::const_iterator mit;
	
	if (pmRange->end() != (mit = pmRange->find(nHandCategory))) {
		return mit->second;
	}
	return -1.0;
}

std::string CBoardAnalyzer::DisplayPostFlopRanges()
{
	CBoardAnalyzer::GetHandRangeWeighting(std::string(""), 0);
	std::map<std::string, std::map<int, double> >::iterator mit;
	
	std::string sDisp;
	for (mit = _mHandRangeMap.begin(); mit != _mHandRangeMap.end();  ++mit) {
		sDisp += mit->first;
		sDisp += ", ";
	}
	return sDisp;
}
