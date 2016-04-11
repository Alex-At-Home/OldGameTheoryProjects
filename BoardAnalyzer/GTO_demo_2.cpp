///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
//
// GTO_demo_2
//
///////////////////////////////////////////////////////////////////////////////////////////
//
// Some Game Theory Optimal solutions
//
///////////////////////////////////////////////////////////////////////////////////////////
//
//TODO: Explain why in the following trial:
//"BoardAnalyzer.exe --vsrange 6MCOO --myrange 6MCvsCO --GTO turn --faction ALL,BAL6M_KC --flop Qh Ts 7h 5c"
// Caller ("6MCCvsCO") KCs "90%: 9s7s, 7s6s, ;" but "0%: [...]8h8s, 8s8c, 9h9s, 9s9c,[...] 9s8s," (and similar) 
//
//TODO: It's also not clear in the above example why Bettor ("6MCOO") does so much better (10.2 v 4.9, bet flop 63%) when 
// their ranges seem pretty similar (B is a bit stronger; C has more straight draws)?
// Eg w/ the same (static) distribution, [0,1] gives (0.66 v 0.45, bet flop 28%)
//
//TODO: The "faction" doesn't get applied to the hand ranges
//
//TODO: Do all this in category form, eg "50% TPTK+" either for display (and!/)or processing reasons
//
//TODO: see if the "perturbed" version of the game is any different?
//
// Another example I've been playing with: 
//BoardAnalyzer.exe --GTO turn --flop Qh 7s 3h 3d --myrange 6MCvsUTG --vsrange 6MUTGO|more	
//
///////////////////////////////////////////////////////////////////////////////////////////

#include <deque>
#include <string>
#include <set>
#include <list>
#include <map>

#include <string.h>

#include "poker_defs.h"
#include "enumdefs.h"

#include "CBoard.h"
#include "CBoardAnalyzer.h"

#define __DISP_RIVERS
//#define __DISPLAY_HAND_DETAILS

static int _nRiverOnly = -1; // (otherwise the compressed value of the river)

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// In this model, the following scenario occurs:

// 1. Caller (C==H) is either in position and can only check/call/fold; or out of position and can only check+call/fold
// 2. The starting pot is $15.
// 3. Bettor (B==V) can only check or bet the pot.
// 4. All 4 streets play as normal

static const int NSUIT = 4;
static const int NRANK = 13;
static const int NDECK = 52;

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

//TODO: rewrite the design information here
// Translate Bettor and Caller ranges as follows:
	// (1)  unused
	// (2)	If there are 2 or 3 of a suit, make that suit hearts
	// (3)	In case (2), the other suit will be diamonds
	// (4)	In case (2), 2-t, then we break suited cards into HH, DD, and CC [<- with 2x freq]
	// (5)	In case (2), 1-t, then we break suited cards into HH, and CC [<- 3x]
	// (6)	In case (2), then we break unsuited cards unpaired into HC, CH [<- 3x] and DC [<- x6]
	// (7)	In case (2), then we break unsuited cards paired into HC [<- 3x] and DC [<- x3]
	// (8)	If the suit is rainbow, make the suits H,C,D
	// (9) 	In case (8), suited cards are left alone
	// (10) In case (8), unsuited unpaired cards are TBD (ugh card replacement?)
	// (11) In case (8), unsuited paired cards are TBD (ugh card replacement?)
// (Wherever it says C, substitue for S for the second player.)

////////////////////////////////////////////////////////////////////////////////////////////////

// Some relevant structures:

// Frequencies of compressed hands based on the board/other hands

// Hand frequency based on dead cards:
// H H C D (possible flushes=1)
// Xh *c ... [0]3; [1]2; [2]1; [3+]0 ... ie "F-f0"
// Xc Xs ... [0]3; [1]1; [2+]0 ... ie "0.5*(F - f0)(F - f0 - 1)" 
// Xc Ys ... [0,0]6.0; [1,0]4.0; [1,1]2.6; [2,1]1.3; [2,2]0.66; [2,0]2.0 ... ie "[F-f0]*[F-f1]*[F-1]/F" 
// Xc Yc ... [0,0]3; [1,0]2; [1,1]1.3; [2,0]1; [2,1]0.67; [2,2]0.33; [3+,0]0 ... ie "[F-f0]*[F-f1]/F"
	// (eg for [1,1]: 1/3 chance it's the same suit => 2 combos, 2/3 chance it's different => 1 combo etc etc)
	// I've deleted the working for some reason for the rest, but it runs along the following lines:
	// look at { (F-f0)/F*Xa } for possible "a"s and combine with allowed { (F-f1)/F*Yb } for possible "b"s
//
// Then we're adding +1 to each f0/f1 because the frequency input in will have been decreased for the hand itself
// eg If we have 23 and no other 2s are used, then the freq(2) input will be 2, not 3. Pairs are special and are +2, obviously.

class FreqCalculator {
public:	
	virtual double calculate(int nFreq1, int nFreq2) const = 0;
	virtual double calculate() const { return this->calculate(_nBaseFreq - 1, _nBaseFreq - 1); } // for testing
protected:
	int _nBaseFreq;
};
class FreqCalculator_UnCompressibleHand: public FreqCalculator {
public:	
	FreqCalculator_UnCompressibleHand(int nBaseFreq) { _nBaseFreq = nBaseFreq; }
	double calculate(int nFreq1, int nFreq2) const { return 1.0; } 
};
class FreqCalculator_SemiCompressibleHand : public FreqCalculator {
public:	
	FreqCalculator_SemiCompressibleHand(int nBaseFreq) { _nBaseFreq = nBaseFreq; }
	double calculate(int nFreq1, int nFreq2) const { return 1 + nFreq2; } 
};
class FreqCalculator_SemiCompressibleHand_Inverted : public FreqCalculator { // (here card 2 is the compressible one)
public:	
	FreqCalculator_SemiCompressibleHand_Inverted(int nBaseFreq) { _nBaseFreq = nBaseFreq; }
	double calculate(int nFreq1, int nFreq2) const { return 1 + nFreq1; } 
};
class FreqCalculator_FullyCompressible_Pair : public FreqCalculator {
public:	
	FreqCalculator_FullyCompressible_Pair(int nBaseFreq) { _nBaseFreq = nBaseFreq; }
	double calculate(int nFreq1, int nFreq2) const { return 0.5*(1 + nFreq1)*(2 + nFreq1); } 
		// (horrible hack, add +1 to both terms to account for the fact the hand itself will always contain two of the rank)
	virtual double calculate() const { return this->calculate(_nBaseFreq - 2, 0); } // for testing, see above
};

class FreqCalculator_FullyCompressible_SuitedNonpair : public FreqCalculator {
public:	
	FreqCalculator_FullyCompressible_SuitedNonpair(int nBaseFreq) { _nBaseFreq = nBaseFreq; }
	double calculate(int nFreq1, int nFreq2) const { return (1 + nFreq1)*(1 + nFreq2)/(double)_nBaseFreq; } 
};
//TODO: ^^^^^ errr no this is wrong, I actually need to know which compressible cards have gone on flop/turn
// so I can discount suited two pair combos (probably true also for non-suited non-pair?)... 
// ...because flop/turn combos are always a different suit (or they wouldn't be compressible)
// eg:
//BOARD = Ad 2h 7h, 6c
// >>>38/30, 45/32 = 1 1 1 1 / 1.33 3.00 and 1.00 1.00
// R=2d(13 26=3): Ac6c v 8s8c EF=0.00 F=12.00
// Here there's actually only 1 compressible A6s but we count it as 1.33 because we take into account the possibility that the
// flop Ad and turn 6c are the same suit!

class FreqCalculator_FullyCompressible_NonsuitedNonpair : public FreqCalculator {
public:	
	FreqCalculator_FullyCompressible_NonsuitedNonpair(int nBaseFreq) { _nBaseFreq = nBaseFreq; }
	double calculate(int nFreq1, int nFreq2) const { return (1 + nFreq1)*(1 + nFreq2)*(_nBaseFreq - 1)/(double)_nBaseFreq; }
		// (the -1 added to the _nBaseFreq in this equation is because you have to ignore the card of the same suit as the other holecard)
};
//^^^ (similar comment applies? The case we're "ignoring" - see the comment above - might not actually apply depending on the flop/turn)

////////////////////////////////////////////////////////////////////////////////////////////////

// Compressed hand with pokerenum hand ranking (and pre-compression frequency, ie based on flop) 

struct TranslatedHandInfo {
	// Initialize
	int nHole1;
	int nHole2;			
	double dFreq; // (pre-flop)
	
	const FreqCalculator *pFreq; // (based on hand type)
	
	double dCacheFreq; // (dFreq*pFreq->calculate(board,hand info)
	
	double *dRiverCacheFreq;
	double *dCacheFreqOnRiver;
	void initRiverCache(int nRivers) { 
		dRiverCacheFreq = new double[nRivers]; for (int i = 0; i < nRivers; ++i) dRiverCacheFreq[i] = 1.0; 
		dCacheFreqOnRiver = new double[nRivers]; for (int i = 0; i < nRivers; ++i) dCacheFreqOnRiver[i] = 1.0; 
	}
	
	TranslatedHandInfo() { dRiverCacheFreq = 0; dCacheFreqOnRiver = 0; }
	~TranslatedHandInfo() { if (dRiverCacheFreq) delete [] dRiverCacheFreq; if (dCacheFreqOnRiver) delete [] dCacheFreqOnRiver; }
};

////////////////////////////////////////////////////////////////////////////////////////////////

// Frequency/compression information

struct TransformParameters {
	int nFlushableNum;
	int nCollapsibleNum;
	int nCollapsibleFreq0;
	int nTransformTable[CCard::SUITS];
	int nReverseTransformTable[CCard::SUITS];

	void set(int nFlushesPossible)
	{
		nFlushableNum = ::NRANK*nFlushesPossible;
		nCollapsibleNum = nFlushableNum + ::NRANK;
		nCollapsibleFreq0 = ::NSUIT - nFlushesPossible;
	}
};

////////////////////////////////////////////////////////////////////////////////////////////////

// Encapsulates equity*(compression frequency) and compression frequency

class EquityFreqTable {
public:
	EquityFreqTable(int nP1_hands, int nP2_hands, int nRivers) 
		{ _nP1_hands = nP1_hands; _nP2_hands = nP2_hands; _nRivers = nRivers; _pTable = new Element[nP1_hands*nP2_hands*_nRivers]; }
	~EquityFreqTable() { delete [] _pTable; }
	
	struct Element {
		double dEquityFreq;
		double dFreq;
	};
	
	Element* at(int nP1, int nP2, int nR) { return &_pTable[(nP1*_nRivers + nR)*_nP2_hands + nP2]; }
	const Element* at(int nP1, int nP2, int nR) const { return &_pTable[(nP1*_nRivers + nR)*_nP2_hands + nP2]; }
	
protected:
	Element *_pTable;
	int _nP1_hands;
	int _nP2_hands;
	int _nRivers;
};

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// Translation/compression transformation functions

static void translateTurnBoard(
						int *nTurnTable, 
						struct TransformParameters* pTransformParams)
{
	// Initialize transform tables
	for (int i = CCard::HEARTS; i < CCard::SUITS; ++i) {
		pTransformParams->nTransformTable[i] = -1;
		pTransformParams->nReverseTransformTable[i] = -1;
	}			
	int nFlushesPossible = 0;
	CCard c1(nTurnTable[0]), c2(nTurnTable[1]), c3(nTurnTable[2]);
	CCard t(nTurnTable[3]);
	
	//printf("Map\n%s %s %s, %s -> \n", c1.s().c_str(), c2.s().c_str(), c3.s().c_str(), t.s().c_str());	
	
	if (c1.getSuit() == c2.getSuit())
		if (c2.getSuit() == c3.getSuit())
		{
			pTransformParams->nTransformTable[CCard::HEARTS] = c1.getSuit();
			
			if (t.getSuit() == c1.getSuit()) { // HHHH
				nFlushesPossible = 1;
				nTurnTable[3] = t.changeSuit(CCard::HEARTS);
			}
			else { // HHHD
				nFlushesPossible = 1;
				pTransformParams->nTransformTable[CCard::DIAMONDS] = t.getSuit();
				nTurnTable[3] = t.changeSuit(CCard::DIAMONDS);
			}
			nTurnTable[0] = c1.changeSuit(CCard::HEARTS);
			nTurnTable[1] = c2.changeSuit(CCard::HEARTS);
			nTurnTable[2] = c3.changeSuit(CCard::HEARTS);
		}
		else // c3 different to c1/c2
		{
			pTransformParams->nTransformTable[CCard::HEARTS] = c1.getSuit();
			pTransformParams->nTransformTable[CCard::DIAMONDS] = c3.getSuit();
			
			if (t.getSuit() == c1.getSuit()) { // HHDH
				nFlushesPossible = 1;
				nTurnTable[3] = t.changeSuit(CCard::HEARTS);
			}
			else if (t.getSuit() == c3.getSuit()) { // HHDD
				nFlushesPossible = 2;
				pTransformParams->nTransformTable[CCard::DIAMONDS] = t.getSuit();
				nTurnTable[3] = t.changeSuit(CCard::DIAMONDS);				
			}
			else { // HHDC
				nFlushesPossible = 1;
				pTransformParams->nTransformTable[CCard::CLUBS] = t.getSuit();
				nTurnTable[3] = t.changeSuit(CCard::CLUBS);
			}
			nTurnTable[0] = c1.changeSuit(CCard::HEARTS);
			nTurnTable[1] = c2.changeSuit(CCard::HEARTS);
			nTurnTable[2] = c3.changeSuit(CCard::DIAMONDS);
		}
	else if (c1.getSuit() == c3.getSuit())
		if (c2.getSuit() == c3.getSuit())
		{
			// Internal logic error, c1=c2=c3 already done
			exit(-1);
		}
		else // c2 different to c1/c3
		{
			pTransformParams->nTransformTable[CCard::HEARTS] = c1.getSuit();
			pTransformParams->nTransformTable[CCard::DIAMONDS] = c2.getSuit();

			if (t.getSuit() == c1.getSuit()) { // HDHH
				nFlushesPossible = 1;
				nTurnTable[3] = t.changeSuit(CCard::HEARTS);
			}
			else if (t.getSuit() == c2.getSuit()) { // HDHD
				nFlushesPossible = 2;
				pTransformParams->nTransformTable[CCard::DIAMONDS] = t.getSuit();
				nTurnTable[3] = t.changeSuit(CCard::DIAMONDS);				
			}
			else { // HDHC
				nFlushesPossible = 1;
				pTransformParams->nTransformTable[CCard::CLUBS] = t.getSuit();
				nTurnTable[3] = t.changeSuit(CCard::CLUBS);
			}
			nTurnTable[0] = c1.changeSuit(CCard::HEARTS);
			nTurnTable[1] = c2.changeSuit(CCard::DIAMONDS);
			nTurnTable[2] = c3.changeSuit(CCard::HEARTS);
		}
	else if (c2.getSuit() == c3.getSuit())
		if (c2.getSuit() == c1.getSuit())
		{
			// Internal logic error, c1=c2=c3 already done
			exit(-1);
		}
		else // c1 different to c2/c3
		{
			pTransformParams->nTransformTable[CCard::HEARTS] = c2.getSuit();
			pTransformParams->nTransformTable[CCard::DIAMONDS] = c1.getSuit();

			if (t.getSuit() == c2.getSuit()) { // DHHH
				nFlushesPossible = 1;
				nTurnTable[3] = t.changeSuit(CCard::HEARTS);
			}
			else if (t.getSuit() == c1.getSuit()) { // DHHD
				nFlushesPossible = 2;
				pTransformParams->nTransformTable[CCard::DIAMONDS] = t.getSuit();
				nTurnTable[3] = t.changeSuit(CCard::DIAMONDS);				
			}
			else { // DHHC
				nFlushesPossible = 1;
				pTransformParams->nTransformTable[CCard::CLUBS] = t.getSuit();
				nTurnTable[3] = t.changeSuit(CCard::CLUBS);
			}
			nTurnTable[0] = c1.changeSuit(CCard::DIAMONDS);
			nTurnTable[1] = c2.changeSuit(CCard::HEARTS);
			nTurnTable[2] = c3.changeSuit(CCard::HEARTS);
		}
	else { 
		// Rainbow flop, this one's a bit more complicated because we need to choose H based on turn
		
		if (t.getSuit() == c1.getSuit()) { // HDCH
			nFlushesPossible = 1;
			pTransformParams->nTransformTable[CCard::HEARTS] = c1.getSuit();
			pTransformParams->nTransformTable[CCard::DIAMONDS] = c2.getSuit();
			pTransformParams->nTransformTable[CCard::CLUBS] = c3.getSuit();
			nTurnTable[0] = c1.changeSuit(CCard::HEARTS);
			nTurnTable[1] = c2.changeSuit(CCard::DIAMONDS);
			nTurnTable[2] = c3.changeSuit(CCard::CLUBS);
			nTurnTable[3] = t.changeSuit(CCard::HEARTS);
		}
		else if (t.getSuit() == c2.getSuit()) { // DHCH
			nFlushesPossible = 1;
			pTransformParams->nTransformTable[CCard::DIAMONDS] = c1.getSuit();
			pTransformParams->nTransformTable[CCard::HEARTS] = c2.getSuit();
			pTransformParams->nTransformTable[CCard::CLUBS] = c3.getSuit();
			nTurnTable[0] = c1.changeSuit(CCard::DIAMONDS);
			nTurnTable[1] = c2.changeSuit(CCard::HEARTS);
			nTurnTable[2] = c3.changeSuit(CCard::CLUBS);
			nTurnTable[3] = t.changeSuit(CCard::HEARTS);
		}
		else if (t.getSuit() == c3.getSuit()) { // DCHH
			nFlushesPossible = 1;
			pTransformParams->nTransformTable[CCard::DIAMONDS] = c1.getSuit();
			pTransformParams->nTransformTable[CCard::CLUBS] = c2.getSuit();
			pTransformParams->nTransformTable[CCard::HEARTS] = c3.getSuit();
			nTurnTable[0] = c1.changeSuit(CCard::DIAMONDS);
			nTurnTable[1] = c2.changeSuit(CCard::CLUBS);
			nTurnTable[2] = c3.changeSuit(CCard::HEARTS);
			nTurnTable[3] = t.changeSuit(CCard::HEARTS);
		}
		else { // HDSC
			nFlushesPossible = 0;
			pTransformParams->nTransformTable[CCard::SPADES] = t.getSuit();
			nTurnTable[0] = c1.changeSuit(CCard::HEARTS);
			nTurnTable[1] = c2.changeSuit(CCard::DIAMONDS);
			nTurnTable[2] = c3.changeSuit(CCard::CLUBS);
			nTurnTable[3] = t.changeSuit(CCard::SPADES);
		}
	}
	pTransformParams->set(nFlushesPossible);

	//printf("%s %s %s, %s (%d %d %d)\n", 
			//CCard(nTurnTable[0]).s().c_str(), CCard(nTurnTable[1]).s().c_str(), CCard(nTurnTable[2]).s().c_str(), CCard(nTurnTable[3]).s().c_str(),
			//nFlushesPossible, pTransformParams->nCollapsibleFreq0, pTransformParams->nFlushableNum);	

	// Do the reverse table as best we can to start with
	std::set<int> unusedSuits;
	for (int i = CCard::HEARTS; i < CCard::SUITS; ++i) {
		if (-1 != pTransformParams->nTransformTable[i]) {
			pTransformParams->nReverseTransformTable[pTransformParams->nTransformTable[i]] = i;
		}
		else {
			unusedSuits.insert(i);			
		}
	} // (end first pass)
	
	// Fill in remainder of transform table... most simply done with two passes 
	for (int i = CCard::HEARTS; i < CCard::SUITS; ++i) {		
		if (-1 == pTransformParams->nTransformTable[i]) {
			// Can I just map straight across?
			if (-1 == pTransformParams->nReverseTransformTable[i]) {
				pTransformParams->nTransformTable[i] = i;
				pTransformParams->nReverseTransformTable[i] = i;
				unusedSuits.erase(i);
			}
		}
	} // (end second pass)
	// Final pass, mop up remaining suits 
	for (int i = CCard::HEARTS; i < CCard::SUITS; ++i) {		
		if (-1 == pTransformParams->nReverseTransformTable[i]) {
			pTransformParams->nReverseTransformTable[i] = *unusedSuits.begin();
			pTransformParams->nTransformTable[pTransformParams->nReverseTransformTable[i]] = i;
			unusedSuits.erase(unusedSuits.begin());
		}		
	} // (end third pass)
	
	// for (int i = CCard::HEARTS; i < CCard::SUITS; ++i) printf("[1] %d -> %d | %d <- %d\n", i, pTransformParams->nTransformTable[i], i, pTransformParams->nReverseTransformTable[i]);		
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// Once the board is done, need to worry about the hands

///////////////////////////////////////////////////////////////////////////////////////////

// Utility functions to ensure there are no clashes with the flop,turn cards 

static bool getTranslatedSuits_suited(
		struct TranslatedHandInfo *pHand, int nRank1, int nRank2,
		const StdDeck_CardMask& board, const struct TransformParameters* pTransformParams)
{
	int nActualCard1 = StdDeck_MAKE_CARD(nRank1, StdDeck_Suit_SPADES);
	int nActualCard2 = StdDeck_MAKE_CARD(nRank2, StdDeck_Suit_SPADES);
	// (start at the top, work down ... opposite direction to the board suits)
	
	// Easy, just move the start suit until there are no collisions
	while ((StdDeck_CardMask_CARD_IS_SET(board, nActualCard1) || (StdDeck_CardMask_CARD_IS_SET(board, nActualCard2)))
			&& (nActualCard1 >= pTransformParams->nFlushableNum))
	{
		nActualCard1 -= ::NRANK;
		nActualCard2 -= ::NRANK;
	}
	if (nActualCard1 >= pTransformParams->nFlushableNum) {
		pHand->nHole1 = nActualCard1;
		pHand->nHole2 = nActualCard2;
		return true;
	}
	return false;
}

////////////////////////////////////////

static bool getTranslatedSuits_unsuited(
		struct TranslatedHandInfo *pHand, int nRank1, int nRank2,
		const StdDeck_CardMask& board, const struct TransformParameters* pTransformParams)
{
	int nActualCard1 = StdDeck_MAKE_CARD(nRank1, StdDeck_Suit_SPADES);
	int nActualCard2 = StdDeck_MAKE_CARD(nRank2, StdDeck_Suit_SPADES); // (they won't end up the same suit)
	// (start at the top, work down ... opposite direction to the board suits)
	
	// Less-easy-ish, move the suits around making sure they don't end up the same
	int nSuit1 = StdDeck_Suit_SPADES;
	while (StdDeck_CardMask_CARD_IS_SET(board, nActualCard1) && (nActualCard1 >= pTransformParams->nFlushableNum))
	{
		nActualCard1 -= ::NRANK;
		nSuit1--;
	}
	if (nActualCard1 >= pTransformParams->nFlushableNum) {
		int nSuit2 = StdDeck_Suit_SPADES;
		while (((StdDeck_CardMask_CARD_IS_SET(board, nActualCard2) && (nActualCard2 >= pTransformParams->nFlushableNum))
				|| (nSuit1 == nSuit2)))
		{
			nActualCard2 -= ::NRANK;
			nSuit2--;
		}
		if (nActualCard2 >= pTransformParams->nFlushableNum) {
			pHand->nHole1 = nActualCard1;
			pHand->nHole2 = nActualCard2;
			return true;		
		}
	}
	return false;
}

////////////////////////////////////////

static bool getTranslatedSuits_semiCompressible(
		int& nActualCard, int nRank1, 
		const StdDeck_CardMask& board, const struct TransformParameters* pTransformParams)
{
	nActualCard = StdDeck_MAKE_CARD(nRank1, StdDeck_Suit_SPADES);
	// (start at the top, work down ... opposite direction to the board suits)
	
	// Easy, just move the start suit until there are no collisions
	while (StdDeck_CardMask_CARD_IS_SET(board, nActualCard) && (nActualCard >= pTransformParams->nFlushableNum))
	{
		nActualCard -= ::NRANK;
	}
	if (nActualCard >= pTransformParams->nFlushableNum) {
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////

// Main hand translation loop

static struct TranslatedHandInfo *getTranslatedHandTable(
									const std::deque<CCard::Hole>& lHandRange, 
									const struct TransformParameters* pTransformParams,	
									const StdDeck_CardMask& board, const StdDeck_CardMask& dead, 
  									int *pnSize, double *pdTotalHandFreq)
{
	std::deque<CCard::Hole>::const_iterator itH;
	int nH = 0;
	int nSize = 0;
	TranslatedHandInfo *pHandTable = new TranslatedHandInfo[lHandRange.size()]; // (maximum size possible)

	// Will calculate frequencies for each hand type (used in equity pre-calculations):
	const FreqCalculator* pFreq_UnComp = new FreqCalculator_UnCompressibleHand(pTransformParams->nCollapsibleFreq0);
	const FreqCalculator* pFreq_SemiComp = new FreqCalculator_SemiCompressibleHand(pTransformParams->nCollapsibleFreq0);
	const FreqCalculator* pFreq_SemiComp_Inv = new FreqCalculator_SemiCompressibleHand_Inverted(pTransformParams->nCollapsibleFreq0);
	const FreqCalculator* pFreq_FullyComp_Pair = new FreqCalculator_FullyCompressible_Pair(pTransformParams->nCollapsibleFreq0);
	const FreqCalculator* pFreq_FullyComp_Suited = new FreqCalculator_FullyCompressible_SuitedNonpair(pTransformParams->nCollapsibleFreq0);
	const FreqCalculator* pFreq_FullyComp_Nonpair = new FreqCalculator_FullyCompressible_NonsuitedNonpair(pTransformParams->nCollapsibleFreq0);
	
	// Some verification (ie that we haven't lost any hands!)
	double dCount1 = 0.0, dCount2 = 0.0;
	
	for (itH = lHandRange.begin(); itH != lHandRange.end(); ++itH, nH++) {
		int n1 = itH->first.convertToPokerEnum(), n2 = itH->second.convertToPokerEnum();
		dCount1 += itH->dFreq;
		
		int nRank1 = StdDeck_RANK(n1);
		int nRank2 = StdDeck_RANK(n2);
		int nSuit1 = pTransformParams->nReverseTransformTable[StdDeck_SUIT(n1) + 1] - 1; // (only care about suit, not rank)
		int nSuit2 = pTransformParams->nReverseTransformTable[StdDeck_SUIT(n2) + 1] - 1; // (note this is CCard format, not pokenum format)

		//printf("%s/%s -> %s/%s\t", itH->first.s().c_str(), itH->second.s().c_str(), CCard(StdDeck_MAKE_CARD(nRank1, nSuit1),0).s().c_str(), CCard(StdDeck_MAKE_CARD(nRank2, nSuit2),0).s().c_str());
		
		int n1_trans = StdDeck_MAKE_CARD(nRank1, nSuit1);
		int n2_trans = StdDeck_MAKE_CARD(nRank2, nSuit2);

		if (StdDeck_CardMask_CARD_IS_SET(dead, n1_trans) || StdDeck_CardMask_CARD_IS_SET(dead, n2_trans)) 
		{
			// Check vs non-compressible board cards, as an easy way of reducing the number of hands
			//printf("0- Dedup %s/%s (%f)\n", CCard(pHandTable[nSize].nHole1,0).s().c_str(), CCard(pHandTable[nSize].nHole2,0).s().c_str(), pHandTable[nSize].dFreq); 				
			dCount2 += itH->dFreq;
		}		
		else if (nSuit1 == nSuit2) { // Suited cards
			if (n1_trans < pTransformParams->nFlushableNum) { // H or possibly D
				// Add as is
				pHandTable[nSize].nHole1 = n1_trans;
				pHandTable[nSize].nHole2 = n2_trans;
				pHandTable[nSize].dFreq = itH->dFreq;
				pHandTable[nSize].pFreq = pFreq_UnComp;
				pHandTable[nSize].initRiverCache(pTransformParams->nCollapsibleNum);

				//printf("1= Map to %s/%s (%f)\n", CCard(pHandTable[nSize].nHole1,0).s().c_str(), CCard(pHandTable[nSize].nHole2,0).s().c_str(), pHandTable[nSize].dFreq); 				
				dCount2 += pHandTable[nSize].dFreq*pHandTable[nSize].pFreq->calculate();
				nSize++;
			}
			else if (StdDeck_Suit_CLUBS == nSuit1) { // 0=H, 1=D, 2=S, 3=C ... use C	
				// Compress C, S, possibly D into C
				if (::getTranslatedSuits_suited(&pHandTable[nSize], nRank1, nRank2, board, pTransformParams))
				{
					pHandTable[nSize].dFreq = itH->dFreq;
					pHandTable[nSize].pFreq = pFreq_FullyComp_Suited;
					pHandTable[nSize].initRiverCache(pTransformParams->nCollapsibleNum);
					
					//printf("2+ Map to %s/%s (%f)\n", CCard(pHandTable[nSize].nHole1,0).s().c_str(), CCard(pHandTable[nSize].nHole2,0).s().c_str(), pHandTable[nSize].dFreq*pHandTable[nSize].pFreq->calculate()); 				
					dCount2 += pHandTable[nSize].dFreq*pHandTable[nSize].pFreq->calculate();
					nSize++;
				}
			}
			else { // (else ignore - compressed by one of the above cases)
				//printf("3- Ignore\n"); 					
			}			
		} // (end if suited)
		else { // Unsuited cards, there's various scenarios
			if (n1_trans < pTransformParams->nFlushableNum) { // top card not compressible, H or possibly D
				
				if (n2_trans < pTransformParams->nFlushableNum) { // D or possibly H				
					// Add as is
					pHandTable[nSize].nHole1 = n1_trans; 
					pHandTable[nSize].nHole2 = n2_trans;
					pHandTable[nSize].dFreq = itH->dFreq;
					pHandTable[nSize].pFreq = pFreq_UnComp;
					pHandTable[nSize].initRiverCache(pTransformParams->nCollapsibleNum);
					
					//printf("4= Map to %s/%s (%f)\n", CCard(pHandTable[nSize].nHole1,0).s().c_str(), CCard(pHandTable[nSize].nHole2,0).s().c_str(), pHandTable[nSize].dFreq); 				
					dCount2 += pHandTable[nSize].dFreq*pHandTable[nSize].pFreq->calculate();
					nSize++;
				}
				else if (StdDeck_Suit_CLUBS == nSuit2) { // 0=H, 1=D, 2=S, 3=C ... use C
					// Compress C, S, possibly D into C
					if (::getTranslatedSuits_semiCompressible(pHandTable[nSize].nHole2, nRank2, board, pTransformParams))
					{
						pHandTable[nSize].nHole1 = n1_trans;
						pHandTable[nSize].dFreq = itH->dFreq;
						pHandTable[nSize].pFreq = pFreq_SemiComp;
						pHandTable[nSize].initRiverCache(pTransformParams->nCollapsibleNum);
						
						//printf("5+ Map to %s/%s (%f)\n", CCard(pHandTable[nSize].nHole1,0).s().c_str(), CCard(pHandTable[nSize].nHole2,0).s().c_str(), pHandTable[nSize].dFreq*pHandTable[nSize].pFreq->calculate()); 				
						dCount2 += pHandTable[nSize].dFreq*pHandTable[nSize].pFreq->calculate();
						nSize++;					
					}
				}
				else { // (else ignore - compressed by one of the above cases)
					//printf("6- Ignore\n"); 					
				}			
			}
			else if (StdDeck_Suit_CLUBS == nSuit1) { // top ranked card compressible (bottom card might not be)				
				if (n2_trans < pTransformParams->nFlushableNum) { // D or possibly H				
					// Compress C, S, possibly D into C
					if (::getTranslatedSuits_semiCompressible(pHandTable[nSize].nHole1, nRank1, board, pTransformParams))
					{
						pHandTable[nSize].nHole2 = n2_trans; 
						pHandTable[nSize].dFreq = itH->dFreq;
						pHandTable[nSize].pFreq = pFreq_SemiComp_Inv;
						pHandTable[nSize].initRiverCache(pTransformParams->nCollapsibleNum);
						
						//printf("7+ Map to %s/%s (%f)\n", CCard(pHandTable[nSize].nHole1,0).s().c_str(), CCard(pHandTable[nSize].nHole2,0).s().c_str(), pHandTable[nSize].dFreq*pHandTable[nSize].pFreq->calculate()); 				
						dCount2 += pHandTable[nSize].dFreq*pHandTable[nSize].pFreq->calculate();
						nSize++;					
					}
				}
				else if (StdDeck_Suit_SPADES == nSuit2) { 
					// 0=H, 1=D, 2=S, 3=C ... use C and S
					// Compress C, S, possibly D into C and S
					if (::getTranslatedSuits_unsuited(&pHandTable[nSize], nRank1, nRank2, board, pTransformParams))
					{
						pHandTable[nSize].dFreq = itH->dFreq;
							// (second factor is "- 1" because the cards can't be suited ... note freq > 0 otherwise can't be here)
						if (nRank1 == nRank2) {
							pHandTable[nSize].pFreq = pFreq_FullyComp_Pair;
						}					
						else {
							pHandTable[nSize].pFreq = pFreq_FullyComp_Nonpair;
						}
						pHandTable[nSize].initRiverCache(pTransformParams->nCollapsibleNum);
						//printf("8* Map to %s/%s (%f)\n", CCard(pHandTable[nSize].nHole1,0).s().c_str(), CCard(pHandTable[nSize].nHole2,0).s().c_str(), pHandTable[nSize].dFreq*pHandTable[nSize].pFreq->calculate());
						dCount2 += pHandTable[nSize].dFreq*pHandTable[nSize].pFreq->calculate();
						nSize++;
					}
				}
				else { // (else ignore - compressed by one of the above cases)
					//printf("9- Ignore\n"); 					
				}			
			}
			else if ((nRank1 == nRank2) && (StdDeck_Suit_CLUBS == nSuit2) && (StdDeck_Suit_SPADES == nSuit1)) {
				// For pairs you might get XsXc not XcXs
				if (::getTranslatedSuits_unsuited(&pHandTable[nSize], nRank1, nRank2, board, pTransformParams))
				{
					pHandTable[nSize].dFreq = itH->dFreq;
					pHandTable[nSize].pFreq = pFreq_FullyComp_Pair;
					pHandTable[nSize].initRiverCache(pTransformParams->nCollapsibleNum);
	
					//printf("A* Map to %s/%s (%f)\n", CCard(pHandTable[nSize].nHole1,0).s().c_str(), CCard(pHandTable[nSize].nHole2,0).s().c_str(), pHandTable[nSize].dFreq*pHandTable[nSize].pFreq->calculate()); 				
					dCount2 += pHandTable[nSize].dFreq*pHandTable[nSize].pFreq->calculate();
					nSize++;					
				}
			}
			else { // (else ignore - compressed by one of the above cases)
				//printf("B- Ignore\n"); 					
			}			
		} // (end if card suited-ness)				
	} // end loop over hand range

	//printf("_____________________________________________\n");
	//printf("Compressed %d (freq %f) into %d (freq %f)\n", lHandRange.size(), dCount1, nSize, dCount2);

	*pdTotalHandFreq = dCount1;
	
	*pnSize = nSize;
	return pHandTable;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// Player strategies (TBD need to rewrite this to allow more complex strategies)

struct Strategy {

	enum { BETTOR_TURN_CHECK = 0, BETTOR_TURN_POT, BETTOR_TURN_NUMACTIONS };
	enum { BETTOR_RIVER_CHECK = 0, BETTOR_RIVER_POT, BETTOR_RIVER_NUMACTIONS };
	//TODO: currently assuming C has only action, "call/check-behind" ... in practice, all of the following
	//parameters need to be indexed by both C and B possible strategies, eg to allow "C" to bet/raise  

	// Identical enums but worded differently
	enum { CALLER_TURN_FOLD = 0, CALLER_TURN_CALL, CALLER_TURN_NUMACTIONS };
	enum { CALLER_RIVER_FOLD = 0, CALLER_RIVER_CALL, CALLER_RIVER_NUMACTIONS };

	double *(_dTurnAction[BETTOR_TURN_NUMACTIONS]); // indexed by poss flop action, hand, no other information at this point
	double _dTurnActionOverRange[BETTOR_TURN_NUMACTIONS]; // (average over range)

	double *(_dRiverAction[BETTOR_TURN_NUMACTIONS][::NDECK][BETTOR_RIVER_NUMACTIONS]); // indexed by flop action, turn, poss turn action, and hand 
	double _dRiverActionOverRange[BETTOR_TURN_NUMACTIONS][::NDECK][BETTOR_RIVER_NUMACTIONS]; // (average over range)

	int _nHandsInRange; // (size of range)
	int _nRivers;
	
// Utility
	
	Strategy(int nHandsInRange, int nRivers)
	{
		_nHandsInRange = nHandsInRange;
		_nRivers = nRivers;
		
		for (int i = 0; i < BETTOR_TURN_NUMACTIONS; ++i) {
			_dTurnAction[i] = new double[nHandsInRange];
			for (int ii = 0; ii < _nRivers; ++ii) {					
				for (int j = 0; j < BETTOR_RIVER_NUMACTIONS; ++j) {
					_dRiverAction[i][ii][j] = new double[nHandsInRange];					
				}
			}
		}
	}
	~Strategy()
	{
		for (int i = 0; i < BETTOR_TURN_NUMACTIONS; ++i) {
			delete [] _dTurnAction[i];
			for (int ii = 0; ii < _nRivers; ++ii) {					
				for (int j = 0; j < BETTOR_RIVER_NUMACTIONS; ++j) {
					delete [] _dRiverAction[i][ii][j];					
				}
			}
		}
	}		
	void initializeRange()
	{
		for (int i = 0; i < BETTOR_TURN_NUMACTIONS; ++i) {
			_dTurnActionOverRange[i] = 0;
#if 1
//TODO: deciding whether to do this for now
			for (int ii = 0; ii < _nRivers; ++ii) {					
				for (int j = 0; j < BETTOR_RIVER_NUMACTIONS; ++j) {
					_dRiverActionOverRange[i][ii][j] = 0;
				}
			}
#endif 
		}			
	}

	// (Somewhat) Detailed results
	void displayProbs(const std::string& sBoard, Strategy *pValue, struct TranslatedHandInfo* pHands, char cDisp) const
	{
		const char* szDispKF = ('C' == cDisp)?"ffff":"....";
		const char* szDispBC = ('C' == cDisp)?"CCCC":"1111";
		
		if ('C' == cDisp) {
			printf("Turn action summary: KK=%.2f (ev=$%3.0f) BC=%.2f (ev=$%3.0f)\n\n", 
				_dTurnActionOverRange[0], pValue->_dTurnActionOverRange[0], _dTurnActionOverRange[1], pValue->_dTurnActionOverRange[1]);
		}
		else {
			printf("Turn action summary: KK=%.2f (ev=$%3.0f) Bx=%.2f (ev=$%3.0f)\n\n", 
				_dTurnActionOverRange[0], pValue->_dTurnActionOverRange[0], _dTurnActionOverRange[1], pValue->_dTurnActionOverRange[1]);			
		}
		
		for (int n = _nHandsInRange - 1; n >= 0; --n) {
			CCard h1, h2; 
			h1.setFromPokerEnum(pHands[n].nHole1); h2.setFromPokerEnum(pHands[n].nHole2);

			if (_dTurnAction[0][n] < 0.0) continue; // Hand conflicts with turn
			
			printf("HAND ** %s%s (f=%f) **\n",
					h1.convertToString().c_str(), h2.convertToString().c_str(), pHands[n].dFreq);
					
			printf("BOARD (%s): ", sBoard.c_str());
			for (int i = 0; i < BETTOR_TURN_NUMACTIONS; ++i) {
				if ((0 == i) && ('C' == cDisp)) {
					printf("[%d: p=---- ev=$%3.0f] ", i,  pValue->_dTurnAction[i][n]);
				}
				else {
					if (_dTurnAction[i][n] < 0.005) {
						printf("[%d: p=%s ev=$%3.0f] ", i, szDispKF, pValue->_dTurnAction[i][n]);
					}
					else if (_dTurnAction[i][n] > 0.995) {
						printf("[%d: p=%s ev=$%3.0f] ", i, szDispBC, pValue->_dTurnAction[i][n]);
					}
					else {
						printf("[%d: p=%.2f ev=$%3.0f] ", i,  _dTurnAction[i][n], pValue->_dTurnAction[i][n]);
					}
				}
			}
			printf("\n");				
#ifdef __DISP_RIVERS

			for (int ii = 0; ii < _nRivers; ++ii) {				
				if (_dRiverAction[0][ii][0][n] < 0.0) {
					continue; // Hand conflicts with river
				}				
				if (_nRiverOnly != ii) { // (river only play)
					continue;
				}
				CCard cR; 
				cR.setFromPokerEnum(ii);
				printf("RIVER (%s)%s: ", sBoard.c_str(), cR.convertToString().c_str());
				for (int i = 0; i < BETTOR_TURN_NUMACTIONS; ++i) {
					for (int j = 0; j < BETTOR_RIVER_NUMACTIONS; ++j) {
						if ((0 == j) && ('C' == cDisp)) {
							printf("[%d/%d: p=---- ev=$%3.0f] ", i, j, pValue->_dRiverAction[i][ii][j][n]);
						}
						else {
							if (_dRiverAction[i][ii][j][n] < 0.005) {
								printf("[%d/%d: p=%s ev=$%3.0f] ", i, j, szDispKF, pValue->_dRiverAction[i][ii][j][n]);										
							}
							else if (_dRiverAction[i][ii][j][n] > 0.995) {
								printf("[%d/%d: p=%s ev=$%3.0f] ", i, j, szDispBC, pValue->_dRiverAction[i][ii][j][n]);										
							}
							else {
								printf("[%d/%d: p=%.2f ev=$%3.0f] ", i, j, _dRiverAction[i][ii][j][n], pValue->_dRiverAction[i][ii][j][n]);																	
							}
						}
					}
				}					
				printf("\n");
			} // end loop over rivers
			printf("\n");
#endif //__DISP_RIVERS
		}
		printf("\n");				
	}
	
	void summarizeProbs(const std::string& sBoard, Strategy *pValue, struct TranslatedHandInfo* pHands, char cDisp) const
	{
		if ('C' == cDisp) {
			printf("CALLER STRATEGY:\n");
			printf("Turn action summary: KK=%.2f (ev=$%3.0f) BC=%.2f (ev=$%3.0f)\n", 
				_dTurnActionOverRange[0], pValue->_dTurnActionOverRange[0], _dTurnActionOverRange[1], pValue->_dTurnActionOverRange[1]);
		}
		else {
			printf("BETTOR STRATEGY:\n");
			printf("Turn action summary: KK=%.2f (ev=$%3.0f) Bx=%.2f (ev=$%3.0f)\n", 
				_dTurnActionOverRange[0], pValue->_dTurnActionOverRange[0], _dTurnActionOverRange[1], pValue->_dTurnActionOverRange[1]);			
		}
		// Turn:
		// First group hands according to prob(action), rounded to nearest 10%
		std::map<int, std::list<int> > mHandByProbAction;
		for (int n = 0; n < _nHandsInRange; ++n) {
			int nProbAction = (int)((_dTurnAction[1][n] + 0.05)*10.0)*10; 
			if (nProbAction >= 0)
			{
				mHandByProbAction[nProbAction].push_back(n);
			}
		}
		std::map<int, std::list<int> >::reverse_iterator rit;
		std::list<int>::iterator lit;
		for (rit = mHandByProbAction.rbegin(); rit != mHandByProbAction.rend(); ++rit)
		{
			printf("\t%3d%%: ", rit->first);
			for (lit = rit->second.begin(); lit != rit->second.end(); ++lit) {
				printf("%s%s, ", CCard(pHands[*lit].nHole1,0).s().c_str(), CCard(pHands[*lit].nHole2,0).s().c_str());
			}
			printf(";\n");
		}
		printf("\n");
		for (int nTA = BETTOR_TURN_CHECK; nTA < BETTOR_TURN_NUMACTIONS; ++nTA) {
			std::map<std::string, std::list<int> > mRiverActions;
			printf("River action summary (turn %s):\n", nTA?"BC":"KK");
#ifdef __DISP_RIVERS
			for (int nR = 0; nR < _nRivers; ++nR) {
				if (_nRiverOnly != nR) { // (river only play)
					continue;
				}				
				
				mHandByProbAction.clear();
				for (int n = 0; n < _nHandsInRange; ++n) {					
					if (_dRiverAction[0][nR][0][n] < 0.0) {
						continue; // Hand conflicts with river
					}					
//TODO pHands[n].dFreq doesn't take into account river?					
					int nTProbAction = (int)((pHands[n].dFreq*_dTurnAction[nTA][n] + 0.05)*10.0)*10; 
					int nRProbAction = (int)((_dRiverAction[nTA][nR][1][n] + 0.05)*10.0)*10; 
					if ((nRProbAction > 0) && (nTProbAction > 0)) {
						mHandByProbAction[nRProbAction].push_back(n);
					}
				} // end loop over "my" hands
				
				if (mHandByProbAction.size() > 0) {
					static char szLine[16384];
					int nWrote = 0;
					for (rit = mHandByProbAction.rbegin(); rit != mHandByProbAction.rend(); ++rit)
					{
						sprintf(szLine + nWrote, "\t%3d%%: ", rit->first);
						nWrote += 7;
						for (lit = rit->second.begin(); lit != rit->second.end(); ++lit) {
							sprintf(szLine + nWrote, "%s%s", CCard(pHands[*lit].nHole1,0).s().c_str(), CCard(pHands[*lit].nHole2,0).s().c_str());
							nWrote += 4;
							double d;
							if ((d = (pHands[*lit].dFreq*_dTurnAction[nTA][*lit])) < 0.95) { // (ie won't round up to 100%)
								sprintf(szLine + nWrote, " (%2d%%), ", (int)((d + 0.025)*20)*5);
								nWrote += 8;
							}
							else {
								sprintf(szLine + nWrote, ", ");
								nWrote += 2;
							}
						}
						sprintf(szLine + nWrote, ";\n");
						nWrote += strlen(";\n"); // (CRLF = 2 chars in windows, 1 in linux)
					}
					if (nWrote > 0) {
						std::string s = std::string(szLine);
						mRiverActions[s].push_back(nR);
					}
				} // end if hand %s
			} // end loop over rivers

			std::map<std::string, std::list<int> >::iterator mit;
			for (mit = mRiverActions.begin(); mit != mRiverActions.end(); ++mit) {
				for (lit = mit->second.begin(); lit != mit->second.end(); ++lit) {
					printf("%s [p=%.2f ev=$%3.0f] ", CCard(*lit,0).s().c_str(),
						_dRiverActionOverRange[nTA][*lit][1], pValue->_dRiverActionOverRange[nTA][*lit][1]);
				}
				printf(":\n");
				printf("%s", mit->first.c_str());
			}
			printf("\n");
#endif //__DISP_RIVERS
		}		
		printf("\n");
	}
};

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// Some more display functions

static void display_equity_distribution(
		std::multimap<double, int>* pmmCallerEquityOrders,
		Strategy* pCallerStrategy, struct TranslatedHandInfo* pCallerHands,
		std::multimap<double, int>* pmmBettorEquityOrders,
		Strategy* pBettorStrategy, struct TranslatedHandInfo* pBettorHands,
		const struct TransformParameters &transParams
		)
{
	static int nStepSize_ = 10;
	double dCallerProbActTable[1 + 100/nStepSize_];
	double dBettorProbActTable[1 + 100/nStepSize_];
	for (int nR = 0; nR < transParams.nCollapsibleNum; ++nR)
	{
		if (_nRiverOnly != nR) { // river only play
			continue;
		}		
		
		std::multimap<double, int>*pmmCEO = pmmCallerEquityOrders + nR;
		if (pmmCEO->empty()) continue;
		std::multimap<double, int>*pmmBEO = pmmBettorEquityOrders + nR;

		printf("********** River = %s:\n\t", CCard().setFromPokerEnum(nR).s().c_str());

		// Scale
		for (int n = 0; n < 100; n += nStepSize_)
		{
			printf("% 3d%%+\t", n);
		}
		printf("\nC:\t");
		std::multimap<double, int>::iterator mit;
		int nDivEq = 0; int nLastIndex = pmmCEO->begin()->second;
		for (mit = pmmCEO->begin(); mit != pmmCEO->end(); ++mit)
		{
			int nCurrDivEq = (((int)(mit->first*100.0))/nStepSize_)*nStepSize_;
			if ((nCurrDivEq != nDivEq) && (nDivEq < 90)) {
				
				printf("%s%s-\t", CCard().setFromPokerEnum(pCallerHands[nLastIndex].nHole1).s().c_str(), CCard().setFromPokerEnum(pCallerHands[nLastIndex].nHole2).s().c_str());
				for (nDivEq += nStepSize_; nDivEq < nCurrDivEq; nDivEq += nStepSize_)
				{
					printf(".....   ");
				}
			}
			nLastIndex = mit->second;
		}
		printf("%s%s-\t", CCard().setFromPokerEnum(pCallerHands[nLastIndex].nHole1).s().c_str(), CCard().setFromPokerEnum(pCallerHands[nLastIndex].nHole2).s().c_str());
		for (nDivEq += nStepSize_; nDivEq < 100; nDivEq += nStepSize_)
		{
			printf(".....   ");
		}			
		printf("\nB:\t");
		nDivEq = 0; nLastIndex = pmmBEO->begin()->second;
		for (mit = pmmBEO->begin(); mit != pmmBEO->end(); ++mit)
		{			
			int nCurrDivEq = (((int)(mit->first*100.0))/nStepSize_)*nStepSize_;

//printf("%d %d %d ... %s%s\n", nCurrDivEq, nDivEq, nLastIndex,CCard().setFromPokerEnum(pBettorHands[mit->second].nHole1).s().c_str(), CCard().setFromPokerEnum(pBettorHands[mit->second].nHole2).s().c_str());
			
			if ((nCurrDivEq != nDivEq) && (nDivEq < 90)) {
				printf("%s%s-\t", CCard().setFromPokerEnum(pBettorHands[nLastIndex].nHole1).s().c_str(), CCard().setFromPokerEnum(pBettorHands[nLastIndex].nHole2).s().c_str());
				for (nDivEq += nStepSize_; nDivEq < nCurrDivEq; nDivEq += nStepSize_)
				{
					printf(".....   ");
				}
			}
			nLastIndex = mit->second;
		}
		printf("%s%s-\t", CCard().setFromPokerEnum(pBettorHands[nLastIndex].nHole1).s().c_str(), CCard().setFromPokerEnum(pBettorHands[nLastIndex].nHole2).s().c_str());
		for (nDivEq += nStepSize_; nDivEq < 100; nDivEq += nStepSize_)
		{
			printf(".....   ");
		}			
		printf("\n--------");
		for (int n = 0; n < 100; n += nStepSize_)
		{
			printf("--------");
		}
		printf("\n");
		
		for (int nTA = Strategy::BETTOR_TURN_CHECK; nTA < Strategy::BETTOR_TURN_NUMACTIONS; ++nTA) {
			if ((Strategy::BETTOR_TURN_POT == nTA) && (-1 != _nRiverOnly)) {
				continue; // river only == turn k-k
			}
			printf("(Turn action %s):\nC:\t", nTA?"BC":"KK");

			for (int i = 0; i < 1 + 100/nStepSize_; ++i) {
				dCallerProbActTable[i] = 0.0;
				dBettorProbActTable[i] = 0.0;
			}
			
			double dCallerTotalFreq = 0.0;
			for (mit = pmmCEO->begin(); mit != pmmCEO->end(); ++mit)
			{
				dCallerTotalFreq += pCallerHands[mit->second].dCacheFreqOnRiver[nR]*pCallerStrategy->_dTurnAction[nTA][mit->second];				
			}
			
			nDivEq = 0;
			double dCumFreq = 0.0;		
			
			// Caller distribution
			int nProbActIndex = 0;
			for (mit = pmmCEO->begin(); mit != pmmCEO->end(); ++mit)
			{
				int nCurrDivEq = (((int)(mit->first*100.0))/nStepSize_)*nStepSize_;

#if 0
printf("hand %s%s ... %.1f (%.1f) ... %.1f %d %d ... SUM = %.1f ... prob act = %.1f\n",
		CCard().setFromPokerEnum(pCallerHands[mit->second].nHole1).s().c_str(),CCard().setFromPokerEnum(pCallerHands[mit->second].nHole2).s().c_str(),
		pCallerHands[mit->second].dCacheFreqOnRiver[nR], dCallerTotalFreq, mit->first*100.0, nCurrDivEq, nDivEq, dCumFreq, 100.0*pCallerStrategy->_dRiverAction[nTA][nR][1][mit->second]);
#endif
				if ((nCurrDivEq == nDivEq)||(nDivEq == (100 - nStepSize_))) {
					double dFreq = pCallerHands[mit->second].dCacheFreqOnRiver[nR]*pCallerStrategy->_dTurnAction[nTA][mit->second]; 
					dCumFreq += dFreq;
					dCallerProbActTable[nProbActIndex] += dFreq*pCallerStrategy->_dRiverAction[nTA][nR][1][mit->second]; 
				}
				else { // new bin
					if (dCumFreq > 0.0) {
						dCallerProbActTable[nProbActIndex] /= dCumFreq;
					}
					else {
						dCallerProbActTable[nProbActIndex] = -1.0;						
					}
					printf("% 3d%%\t", (int)((100.0*dCumFreq)/dCallerTotalFreq + 0.5));
					for (nDivEq += nStepSize_, ++nProbActIndex; nDivEq < nCurrDivEq; nDivEq += nStepSize_, ++nProbActIndex)
					{
						printf("  0%%\t");
						dCallerProbActTable[nProbActIndex] = -1.0;
					}
					dCumFreq = pCallerHands[mit->second].dCacheFreqOnRiver[nR]*pCallerStrategy->_dTurnAction[nTA][mit->second];
					dCallerProbActTable[nProbActIndex] = dCumFreq*pBettorStrategy->_dRiverAction[nTA][nR][1][mit->second]; 
				} // (end if new bin)
			} // end loop over caller hands
			if (dCumFreq > 0.0) {
				dCallerProbActTable[nProbActIndex] /= dCumFreq;
			}
			else {
				dCallerProbActTable[nProbActIndex] = -1.0;						
			}
			printf("% 3d%%\t", (int)((100.0*dCumFreq)/dCallerTotalFreq + 0.5));
			for (nDivEq += nStepSize_, ++nProbActIndex; nDivEq < 100; nDivEq += nStepSize_, ++nProbActIndex)
			{
				printf("  0%%\t");
				dCallerProbActTable[nProbActIndex] = -1.0;
			}			
			printf("\n\t");
			for (int i = 0; i < 100/nStepSize_; ++i) {
				if (dCallerProbActTable[i] >= 0.0) {
					printf("[% 3d%%]\t", (int)(100.0*dCallerProbActTable[i]));
				}
				else {
					printf("[N/A]\t");
				}
			}
			printf("\nB:\t");
			
			nDivEq = 0;
			dCumFreq = 0.0;		
			nProbActIndex = 0;
			
			double dBettorTotalFreq = 0.0;
			for (mit = pmmBEO->begin(); mit != pmmBEO->end(); ++mit)
			{
				dBettorTotalFreq += pBettorHands[mit->second].dCacheFreqOnRiver[nR]*pBettorStrategy->_dTurnAction[nTA][mit->second];				
			}

			// Bettor distribution
			for (mit = pmmBEO->begin(); mit != pmmBEO->end(); ++mit)
			{
				int nCurrDivEq = (((int)(mit->first*100.0))/nStepSize_)*nStepSize_;
				if ((nCurrDivEq == nDivEq)||(nDivEq == (100 - nStepSize_))) { // (100 gets lumped in with 90+)
					double dFreq = pBettorHands[mit->second].dCacheFreqOnRiver[nR]*pBettorStrategy->_dTurnAction[nTA][mit->second];
					dCumFreq += dFreq;
					dBettorProbActTable[nProbActIndex] += dFreq*pBettorStrategy->_dRiverAction[nTA][nR][1][mit->second]; 
				}
				else { // new bin
					if (dCumFreq > 0.0) {
						dBettorProbActTable[nProbActIndex] /= dCumFreq;
					}
					else {
						dBettorProbActTable[nProbActIndex] = -1.0;						
					}
					printf("% 3d%%\t", (int)((100.0*dCumFreq)/dBettorTotalFreq + 0.5));
					for (nDivEq += nStepSize_, ++nProbActIndex; nDivEq < nCurrDivEq; nDivEq += nStepSize_, ++nProbActIndex)
					{
						printf("  0%%\t");
						dBettorProbActTable[nProbActIndex] = -1.0;
					}			
					dCumFreq = pBettorHands[mit->second].dCacheFreqOnRiver[nR]*pBettorStrategy->_dTurnAction[nTA][mit->second];
					dBettorProbActTable[nProbActIndex] = dCumFreq*pBettorStrategy->_dRiverAction[nTA][nR][1][mit->second]; 
				} // (end if new bin)
			} // end loop over bettor hands
			if (dCumFreq > 0.0) {
				dBettorProbActTable[nProbActIndex] /= dCumFreq;
			}
			else {
				dBettorProbActTable[nProbActIndex] = -1.0;						
			}
			printf("% 3d%%\t", (int)((100.0*dCumFreq)/dBettorTotalFreq + 0.5));
			for (nDivEq  += nStepSize_; nDivEq < 100; nDivEq += nStepSize_)
			{
				printf("  0%%\t");
			}			
			for (nDivEq += nStepSize_, ++nProbActIndex; nDivEq < 100; nDivEq += nStepSize_, ++nProbActIndex)
			{
				printf("  0%%\t");
				dBettorProbActTable[nProbActIndex] = -1.0;
			}			
			printf("\n\t");
			for (int i = 0; i < 100/nStepSize_; ++i) {
				if (dBettorProbActTable[i] >= 0.0) {
					printf("[% 3d%%]\t", (int)(100.0*dBettorProbActTable[i]));
				}
				else {
					printf("[N/A]\t");
				}
			}
			printf("\n");
			
		} // end loop over turn actions
		printf("\n");
	} // end loop over rivers
}


///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// Strategy calculation functions

// Pre-calculates weightings and equities of compressed hand ranges given {flop,turn}

	// Places per-"equity hand" frequency (given flop/turn) into TranslatedHandInfo::dCacheFreq (NB one way transform)
	// Places river frequency (given flop/turn, "equity hand") into TranslatedHandInfo::dRiverCacheFreq
	// Places per-"oppo hand" frequency (given flop/turn, "equity hand", river) into EquityFreqTable::dFreq

// (In a somewhat unclean bit of coding - not the only place! - this also inserts some frequencies into the 
//  translated hand info)

static void calculateEquityTable(EquityFreqTable& eqFreqTable,
		struct TranslatedHandInfo *pEquityRange, int nEquityRange, 
		const struct TranslatedHandInfo *pOppoRange, int nOppoRange,
		StdDeck_CardMask board, StdDeck_CardMask dead, 
		int nBoardFreqTable[], const struct TransformParameters &transParams,
		std::multimap<double, int>* pmmEquityOrders
		)
{			
	// (Initialize equity calculator)
	int err;	
	StdDeck_CardMask pockets[2];
	enum_result_t result;
	StdDeck_CardMask_RESET(pockets[0]);
	StdDeck_CardMask_RESET(pockets[1]);

	EquityFreqTable::Element *pEl = eqFreqTable.at(0, 0, 0);
	for (int nE = 0; nE < nEquityRange; ++nE) {
		int n1_E = pEquityRange[nE].nHole1, n2_E = pEquityRange[nE].nHole2;
		int nR1_E = StdDeck_RANK(n1_E); int nR2_E = StdDeck_RANK(n2_E);

		int nFE1 = (n1_E < transParams.nFlushableNum) ? 1 : nBoardFreqTable[nR1_E];
		int nFE2 = (n2_E < transParams.nFlushableNum) ? 1 : nBoardFreqTable[nR2_E];
			// (note here and below, this miscounts pairs for these checks, but the only effect is wasted CPU)
		
		if ((StdDeck_CardMask_CARD_IS_SET(dead, n1_E) || StdDeck_CardMask_CARD_IS_SET(dead, n2_E))
				||				//^^^ (note I only set the dead card mask for flushable cards)
			((1 > nFE1))||(1 > nFE2))
		{
			pEl->dEquityFreq = -4.0;
			pEl->dFreq = -4.0;
			pEl = eqFreqTable.at(nE + 1, 0, 0);
			continue; 			
		}
		StdDeck_CardMask_SET(pockets[0], n1_E);
		StdDeck_CardMask_SET(pockets[0], n2_E);
		if (n1_E < transParams.nFlushableNum) {
			StdDeck_CardMask_SET(dead, n1_E); 
		}
		else {
			nBoardFreqTable[nR1_E]--;
		}
		if (n2_E < transParams.nFlushableNum) {
			StdDeck_CardMask_SET(dead, n2_E); 
		}
		else {
			nBoardFreqTable[nR2_E]--;
		}
		nFE1 = (n1_E < transParams.nFlushableNum) ? 1 : nBoardFreqTable[nR1_E];
		nFE2 = (n2_E < transParams.nFlushableNum) ? 1 : nBoardFreqTable[nR2_E];
		
		pEquityRange[nE].dCacheFreq = pEquityRange[nE].dFreq*pEquityRange[nE].pFreq->calculate(nFE1, nFE2);
		if (pEquityRange[nE].dCacheFreq < 0.0) {
			pEquityRange[nE].dCacheFreq = 0.0;
		}
		
		double dSumEquityFreq = 0.0, dSumFreq = 0.0; 		
		
		for (int nR = 0; nR < transParams.nCollapsibleNum; ++nR) {				
			int nRR = StdDeck_RANK(nR);
			std::multimap<double, int>* pmmEquityOrder = pmmEquityOrders + nR; 

			int nFR = (nR < transParams.nFlushableNum) ? 1 : nBoardFreqTable[nRR];
			if ((StdDeck_CardMask_CARD_IS_SET(dead, nR))||(1 > nFR))
			{
				pEl->dEquityFreq = -3.0;
				pEl->dFreq = -3.0;
				pEl = eqFreqTable.at(nE, 0, nR + 1);
				continue;
			}
			int nR_adj = nR;
			if (nR < transParams.nFlushableNum) {
				StdDeck_CardMask_SET(dead, nR); 
				// (pEquityRange[nE].dRiverCacheFreq initialized to 1, so nothing to do here)
			}
			else {
				nBoardFreqTable[nRR]--;
				while (StdDeck_CardMask_CARD_IS_SET(board, nR_adj)) {
					// (this loop handles making compressible rivers different to compressible E hold cards) 
					nR_adj += ::NRANK;
				}
				pEquityRange[nE].dRiverCacheFreq[nR] = nFR;
				// (if all the cards are in use nFR would be 0, so we'll always "how to exit the loop")
			}
			StdDeck_CardMask_SET(board, nR_adj);

			// Calculate the per-river frequencies (used to see distributions)
			nFE1 = (n1_E < transParams.nFlushableNum) ? 1 : nBoardFreqTable[nR1_E];
			nFE2 = (n2_E < transParams.nFlushableNum) ? 1 : nBoardFreqTable[nR2_E];
			pEquityRange[nE].dCacheFreqOnRiver[nR] = pEquityRange[nE].dFreq*pEquityRange[nE].pFreq->calculate(nFE1, nFE2);
			
			double dSumEquityFreq_R = 0.0, dSumFreq_R = 0.0; 
			
			for (int nO = 0; nO < nOppoRange; nO++) {												
				int n1_O = pOppoRange[nO].nHole1, n2_O = pOppoRange[nO].nHole2;
				int nR1_O = StdDeck_RANK(n1_O); int nR2_O = StdDeck_RANK(n2_O);

				int nFO1 = (n1_O >= transParams.nFlushableNum) ? nBoardFreqTable[nR1_O] : 1;
				int nFO2 = (n2_O >= transParams.nFlushableNum) ? nBoardFreqTable[nR2_O] : 1;
				if ((StdDeck_CardMask_CARD_IS_SET(dead, n1_O) || StdDeck_CardMask_CARD_IS_SET(dead, n2_O))
						||
					((1 > nFO1)||(1 > nFO2)))
				{
					pEl->dEquityFreq = 0.0;
					pEl->dFreq = 0.0; // (just set these both to be 0 to avoid an "if" clause in the inner loop of the BR code)
					pEl++;
					continue; 
				}
				StdDeck_CardMask_SET(pockets[1], n1_O);
				StdDeck_CardMask_SET(pockets[1], n2_O);
				if (n1_O >= transParams.nFlushableNum) {
					nBoardFreqTable[nR1_O]--;
				}
				if (n2_O >= transParams.nFlushableNum) {
					nBoardFreqTable[nR2_O]--;
				}
				nFO1 = (n1_O >= transParams.nFlushableNum) ? nBoardFreqTable[nR1_O] : 1;
				nFO2 = (n2_O >= transParams.nFlushableNum) ? nBoardFreqTable[nR2_O] : 1;
				
			    err = enumExhaustive(game_holdem, pockets, board, dead, 2, 5, 0, &result);
			    if (err) {
					pEl->dEquityFreq = 0.0;
					pEl->dFreq = 0.0; // (just set these both to be 0 to avoid an "if" clause in the inner loop of the BR code)
			    }
			    else {
			    	double dEq = result.ev[0]/result.nsamples;
			    	
					pEl->dFreq = pOppoRange[nO].pFreq->calculate(nFO1, nFO2);
					
					if (pEl->dFreq > 1.0e-9) {
						pEl->dFreq *= pOppoRange[nO].dFreq;
						pEl->dEquityFreq = pEl->dFreq*dEq;
					}
					else {
						pEl->dFreq = 0.0;
						pEl->dEquityFreq = 0.0;						
					}
#if 0
//					if (nRR == 6 && (nR1_E == 6) && (nR2_E == 4) && (nR1_O == 4) && (nR2_O == 4))
					if ((nR > transParams.nFlushableNum) && (nRR == 1) && (nR1_E == 4) && (nR2_E == 4))
					{
						printf("\n>>>%d/%d, %d/%d = %d %d %d %d / %.2f %.2f and %.2f %.2f\n", 
								n1_O, n2_O, n1_E, n2_E, 
								nFO1, nFO2, nFE1, nFE2,
								pOppoRange[nO].pFreq->calculate(nFO1, nFO2), pEquityRange[nE].pFreq->calculate(nFE1, nFE2),
								pOppoRange[nO].dFreq, pEquityRange[nE].dFreq
								);
						printf("R=%s(%d %d=%d): %s%s v %s%s EF=%.2f F=%.2f\n\n", 
							CCard(nR,0).s().c_str(), nR,  transParams.nCollapsibleNum, nFR, 
							CCard(n1_O,0).s().c_str(), CCard(n2_O,0).s().c_str(), CCard(n1_E,0).s().c_str(), CCard(n2_E,0).s().c_str(),
							pEl->dEquityFreq, pEl->dFreq);
					}					
#endif 
			    	dSumEquityFreq_R += pEl->dEquityFreq;
			    	dSumFreq_R += pEl->dFreq;
			    	
			    } // end if eq calc succeeds
				StdDeck_CardMask_RESET(pockets[1]);

				// Reset loop parameters
				if (n1_O >= transParams.nFlushableNum) {
					nBoardFreqTable[nR1_O]++;
				}
				if (n2_O >= transParams.nFlushableNum) {
					nBoardFreqTable[nR2_O]++;
				}
				pEl++;
				
			} // end loop over E cards

			if (dSumFreq_R > 0.0) {
				double nAdj = (double)(nR1_E*13 + nR2_E)*0.0000001; // (tie breaker) 
				pmmEquityOrder->insert(std::pair<double, int>(dSumEquityFreq_R/dSumFreq_R + nAdj, nE));
			}
	    	dSumEquityFreq += dSumEquityFreq_R;
	    	dSumFreq += dSumFreq_R;

#if 0
printf(">>>>>>R Equity: C %s%s vs B range: %s%s%s%s,%s = %.2f (%.2f/%.2f) [running %.2f]\n", 
	CCard(pEquityRange[nE].nHole1,0).s().c_str(),CCard(pEquityRange[nE].nHole2,0).s().c_str(),
	CCard(nBoardTable[0]).s().c_str(),
	CCard(nBoardTable[1]).s().c_str(),
	CCard(nBoardTable[2]).s().c_str(),
	CCard(nBoardTable[3]).s().c_str(),
	CCard(nR,0).s().c_str(), dSumEquityFreq_R/dSumFreq_R, dSumEquityFreq_R, dSumFreq_R, dSumEquityFreq/dSumFreq);
#endif 

			// Reset loop parameters
			StdDeck_CardMask_UNSET(board, nR_adj);
			if (nR < transParams.nFlushableNum) {
				StdDeck_CardMask_UNSET(dead, nR); 
			}
			else {
				nBoardFreqTable[nRR]++;
			}			
		} // (end loop over R)
			
#if 0
printf(">>>>>>>>>>>T Equity: C %s%s vs B range: %s%s%s%s = %.2f (weight %.2f)\n", 
	CCard(pEquityRange[nE].nHole1,0).s().c_str(),CCard(pEquityRange[nE].nHole2,0).s().c_str(),
	CCard(nBoardTable[0]).s().c_str(),
	CCard(nBoardTable[1]).s().c_str(),
	CCard(nBoardTable[2]).s().c_str(),
	CCard(nBoardTable[3]).s().c_str(),
	dSumEquityFreq/dSumFreq, dSumFreq);
#endif 

		// Reset loop parameters
		if (n1_E < transParams.nFlushableNum) {
			StdDeck_CardMask_UNSET(dead, n1_E); 
		}
		else {
			nBoardFreqTable[nR1_E]++; 
		}
		if (n2_E < transParams.nFlushableNum) {
			StdDeck_CardMask_UNSET(dead, n2_E); 
		}
		else {
			nBoardFreqTable[nR2_E]++;
		}
		StdDeck_CardMask_RESET(pockets[0]);

	}// end loop over E cards
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// Calculate simple and approximate initial strategy for B

static void calculateInitialBettorStrategy(const EquityFreqTable& eqFreqBettor, Strategy *pStrategy_B, 
		const struct TranslatedHandInfo *pBettorRange, int nBettorRange, int nCallerRange,
		const struct TransformParameters &transParams)	
{	
	const EquityFreqTable::Element *pEl = eqFreqBettor.at(0, 0, 0);
	double dHands_B = 0.0;
	for (int nB = 0; nB < nBettorRange; ++nB) {

		if (eqFreqBettor.at(nB, 0, 0)->dEquityFreq < -3.5) { // (bettor hand invalid, -4)
			pEl = eqFreqBettor.at(nB + 1, 0, 0);
			continue; 
		}			
		double dBettorFreq = pBettorRange[nB].dCacheFreq; 

		// Possible rivers:
		
		double dSumEquityFreq = 0.0, dSumFreq = 0.0; 	
		
		for (int nR = 0; nR < transParams.nCollapsibleNum; ++nR) {

			if (eqFreqBettor.at(nB, 0, nR)->dEquityFreq < -2.5) { // (river invalid, -3)
				pEl = eqFreqBettor.at(nB, 0, nR + 1);
				continue; 
			}			
			double dRiverFreq = pBettorRange[nB].dRiverCacheFreq[nR]; 

			// Caller hands:
			
			double dSumEquityFreq_C = 0.0, dSumFreq_C = 0.0; 	

			for (int nC = 0; nC < nCallerRange; ++nC) {
				
				if (pEl->dEquityFreq >= -0.5) { // (caller and equity both valid, >-2 and >-1)

					dSumEquityFreq_C += pEl->dEquityFreq;
					dSumFreq_C += pEl->dFreq;  
						// (river play, only caller frequencies given board matter)
				}
				pEl++;
				
			} // end loop over C range
			
			if (dSumFreq_C > 0) {
				double dEq_C = dSumEquityFreq_C/dSumFreq_C;

				if ((dEq_C > 0.15)&&(dEq_C < 0.75)) 
				{
					// Equity >15% and <75% vs range, don't bet river  
					pStrategy_B->_dRiverAction[0][nR][0][nB] = 1.0;			
					pStrategy_B->_dRiverAction[1][nR][0][nB] = 1.0;			
					pStrategy_B->_dRiverAction[0][nR][1][nB] = 0.0;			
					pStrategy_B->_dRiverAction[1][nR][1][nB] = 0.0;			
				}
				else { //bet river 
					pStrategy_B->_dRiverAction[0][nR][0][nB] = 0.0;			
					pStrategy_B->_dRiverAction[1][nR][0][nB] = 0.0;			
					pStrategy_B->_dRiverAction[0][nR][1][nB] = 1.0;			
					pStrategy_B->_dRiverAction[1][nR][1][nB] = 1.0;			
				}
			}
			dSumEquityFreq += dRiverFreq*dSumEquityFreq_C;
			dSumFreq += dRiverFreq*dSumFreq_C;
				// (note when deciding turn play, take river frequencies into account)
				// (obv bettor frequencies are irrelevant here and above!)
			
		} // end loop over river
			
		dHands_B += dBettorFreq;

		if (dSumFreq > 0.0) {
			
			double dEq = dSumEquityFreq/dSumFreq;
			if (dEq < 0.6) { // Equity < 60% vs range, don't bet turn
				pStrategy_B->_dTurnAction[0][nB] = 1.0;			
				pStrategy_B->_dTurnAction[1][nB] = 0.0;			
				pStrategy_B->_dTurnActionOverRange[1] += dBettorFreq;			
			}
			else { // bet turn
				pStrategy_B->_dTurnAction[0][nB] = 0.0;						
				pStrategy_B->_dTurnAction[1][nB] = 1.0;						
				pStrategy_B->_dTurnActionOverRange[0] += dBettorFreq;			
			}
		}
	} // end loop over bettor range

	pStrategy_B->_dTurnActionOverRange[0] /= dHands_B;
	pStrategy_B->_dTurnActionOverRange[1] /= dHands_B;

#if 0
	printf("TURN BETTING STRATEGY:\n");
	printf("BET (%.2f):\n", pStrategy_B->_dTurnActionOverRange[0]);
	for (int nB = 0; nB < nBettorRange; ++nB) {
		if (pStrategy_B->_dTurnAction[1][nB] > 0.99) {
			printf("%s%s [", CCard(pBettorRange[nB].nHole1,0).s().c_str(), CCard(pBettorRange[nB].nHole2,0).s().c_str());
#if 1
			int nR_start = -1, nR_end = -1, n1stR = 0, nLastR = transParams.nCollapsibleNum - 1;
			for (int nR = 0; nR < transParams.nCollapsibleNum; ++nR) {
				if (pStrategy_B->_dRiverAction[1][nR][1][nB] > 0.99) {
					if (-1 == nR_start) {
						nR_start = nR;
					}
					nR_end = nR;
					nLastR = nR;
				}
				else if (pStrategy_B->_dRiverAction[0][nR][0][nB] < 0.01) { // (just a disallowed combo)
					if (nR == n1stR) {
						n1stR++;
					}
				}
				else if (-1 != nR_start) { // end of the run
					if (nR_start == nR_end) {
						printf("%s/", CCard(nR_end,0).s().c_str());
					}
					else {
						printf("%s-%s/", CCard(nR_start,0).s().c_str(), CCard(nR_end,0).s().c_str());						
					}
					nR_start = -1;
					nR_end = -1;
					nLastR = nR;
				}
				else {
					nLastR = nR;					
				}
			}
			if (-1 != nR_start) {
				if (nR_start == nR_end) {
					printf("%s/", CCard(nR_end,0).s().c_str());					
				}
				else if ((n1stR == nR_start) && (nLastR == nR_end)) {
					printf("*");
				}
				else {
					printf("%s-%s/", CCard(nR_start,0).s().c_str(), CCard(nR_end,0).s().c_str());						
				}				
			}
			printf("],  ");
#endif 		
		}
	}
	printf("\n\n");
	printf("CHECK (%.2f):\n", pStrategy_B->_dTurnActionOverRange[1]);
	for (int nB = 0; nB < nBettorRange; ++nB) {
		if (pStrategy_B->_dTurnAction[0][nB] > 0.99) {
			printf("%s%s [", CCard(pBettorRange[nB].nHole1,0).s().c_str(), CCard(pBettorRange[nB].nHole2,0).s().c_str());			
#if 1
			int nR_start = -1, nR_end = -1, n1stR = 0, nLastR = transParams.nCollapsibleNum - 1;
			for (int nR = 0; nR < transParams.nCollapsibleNum; ++nR) {
				if (pStrategy_B->_dRiverAction[1][nR][1][nB] > 0.99) {
					if (-1 == nR_start) {
						nR_start = nR;
					}
					nR_end = nR;
					nLastR = nR;
				}
				else if (pStrategy_B->_dRiverAction[0][nR][0][nB] < 0.01) { // (just a disallowed combo)
					if (nR == n1stR) {
						n1stR++;
					}
				}
				else if (-1 != nR_start) { // end of the run
					if (nR_start == nR_end) {
						printf("%s/", CCard(nR_end,0).s().c_str());
					}
					else {
						printf("%s-%s/", CCard(nR_start,0).s().c_str(), CCard(nR_end,0).s().c_str());						
					}
					nR_start = -1;
					nR_end = -1;
					nLastR = nR;
				}
				else {
					nLastR = nR;					
				}
			}
			if (-1 != nR_start) {
				if (nR_start == nR_end) {
					printf("%s/", CCard(nR_end,0).s().c_str());					
				}
				else if ((n1stR == nR_start) && (nLastR == nR_end)) {
					printf("*");
				}
				else {
					printf("%s-%s/", CCard(nR_start,0).s().c_str(), CCard(nR_end,0).s().c_str());						
				}				
			}
			printf("],  ");
#endif 		
		}
	}
	printf("\n\n");
#endif 	

#if 0
	printf("ALTERNATIVE TURN BETTING STRATEGY:\n");
	printf("TURN BET (%.2f):\n", pStrategy_B->_dTurnActionOverRange[0]);
	for (int nB = 0; nB < nBettorRange; ++nB) {
		if (pStrategy_B->_dTurnAction[1][nB] > 0.99) {
			printf("%s%s,  ", CCard(pBettorRange[nB].nHole1,0).s().c_str(), CCard(pBettorRange[nB].nHole2,0).s().c_str());
		}
	}
	printf("\n\n");
	printf("ALTERNATIVE RIVER BETTING STRATEGY:\n");
	for (int nR = 0; nR < transParams.nCollapsibleNum; ++nR) {
		char szTurn[8];
		printf("BET %s: ", CCard(nR,0).s().c_str());
		for (int nB = 0; nB < nBettorRange; ++nB) {
			if (pStrategy_B->_dRiverAction[1][nR][1][nB] > 0.99) {
				if (pStrategy_B->_dTurnAction[1][nB] > 0.99) { // (bet turn)
					sprintf(szTurn, "");
				}
				else {
					sprintf(szTurn, "(*)");
				}
				printf("%s%s%s,  ", CCard(pBettorRange[nB].nHole1,0).s().c_str(), CCard(pBettorRange[nB].nHole2,0).s().c_str(), szTurn);
			}			
		}
		printf("\n");
	}
	printf("\n");
#endif 	

}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// Calculates one player's strategy based on his opponent's

// Some performance benchmarking...
// 10 iterations (25,5% and 30,0% ranges): 80-90 cycles (all in inner loop over bettor hands)
// (investigate a cache to speed up, or a faster/multicore CPU!)

///////////////////////////////////////////////////////////////////////////////////////////

static double calculateBestResponse_caller(
		Strategy *pStrategy_C, Strategy *pValue_C, const EquityFreqTable& eqFreqCaller, const struct TranslatedHandInfo *pCallerRange, int nCallerRange,
		const Strategy *pStrategy_B, const struct TranslatedHandInfo *pBettorRange, int nBettorRange, 
		double dAlpha, double dInitialPot, const struct TransformParameters& transParams,
		bool bFinalIt)		
{		
	double dNegAlpha = 1.0 - dAlpha;
	
	// EV calcs, set-up
	double dEV_Tx_ft = 0.0; // (T EV | ft = flop,turn cards)

	(*pStrategy_C)._dTurnActionOverRange[0] *= dNegAlpha;
	(*pStrategy_C)._dTurnActionOverRange[1] *= dNegAlpha;
	(*pValue_C)._dTurnActionOverRange[0] *= dNegAlpha;
	(*pValue_C)._dTurnActionOverRange[1] *= dNegAlpha;

	// (temp variable for above)
	double dStrategy_C_dTurnActionOverRange[Strategy::CALLER_RIVER_NUMACTIONS] = { 0.0, 0.0 };
	double dValue_C_dTurnActionOverRange[Strategy::CALLER_RIVER_NUMACTIONS] = { 0.0, 0.0 };
	
	double dHands_C = 0; // total caller hand weighting
	double dHands_C_[Strategy::CALLER_TURN_NUMACTIONS][transParams.nCollapsibleNum];
	if (bFinalIt) {
		for (int x = 0; x < Strategy::CALLER_TURN_NUMACTIONS; ++x) {
			for (int i = 0; i < transParams.nCollapsibleNum; ++i) {
				dHands_C_[x][i] = 0.0;
			}
		}
	}
	
	const EquityFreqTable::Element *pEl = eqFreqCaller.at(0, 0, 0);
	for (int nC = 0; nC < nCallerRange; ++nC) {
		
		if (pEl->dEquityFreq < -3.5) {
			pStrategy_C->_dTurnAction[0][nC] = -4.0;
			pEl = eqFreqCaller.at(nC + 1, 0, 0);
			continue;
		}
		double dHandFreq_C = pCallerRange[nC].dCacheFreq; // caller hand combos given board

// 4.1 TURN ACTIONS			

		// EV calcs, set-up
		double dEV_Tx_T_ft = 0.0; // (T EV | T=flop actions, t = flop,turn cards)

		for (int nTAct_B = Strategy::BETTOR_TURN_CHECK; nTAct_B < Strategy::BETTOR_TURN_NUMACTIONS; ++nTAct_B) {
			
			double dTurnBet = (Strategy::BETTOR_TURN_CHECK == nTAct_B)?(0.0):dInitialPot;
			double dTurnPot_onCall = dInitialPot + 2.0*dTurnBet;
			const double *dStrategyTable_turn = (*pStrategy_B)._dTurnAction[nTAct_B];
			
			pEl = eqFreqCaller.at(nC, 0, 0); // (need to reset because of multiple turn actions)
			
// 4.2 RIVER			
			
			// EV calcs, set-up
			double dEV_Rx_T_ft = 0.0; // (R EV | T=turn actions; t=flop,turn cards)

			// Calculate EV over rivers
			double dCards_R_C = 0; // total river card weighting

			for (int nRiver = 0; nRiver < transParams.nCollapsibleNum; ++nRiver) {
				if (pEl->dEquityFreq < -2.5) {
					pStrategy_C->_dRiverAction[0][nRiver][0][nC] = -3.0;
					pEl = eqFreqCaller.at(nC, 0, nRiver + 1);
					continue;
				}
				if (_nRiverOnly != nRiver) {
					pEl = eqFreqCaller.at(nC, 0, nRiver + 1);
					continue;					
				}
				
				double dRiverFreq = pCallerRange[nC].dRiverCacheFreq[nRiver]; // river frequency given caller hand, board
				
//#define __INNER_LOOP_DEBUG_CALLER	

#ifdef __INNER_LOOP_DEBUG_CALLER
bool bDisp = false;					
CCard h1(pCallerRange[nC].nHole1,0), h2(pCallerRange[nC].nHole2,0), r(nRiver,0); 
if (dAlpha < 0.001) bDisp = true;
//if ((r.getRank() == 9) && (h1.getRank() == 14) && (h2.getRank() == 2)) bDisp = true;
bDisp &= (r.getSuit() != CCard::HEARTS);  
//bDisp &= ((h1.getSuit() == CCard::HEARTS) && (h2.getSuit() == CCard::HEARTS));  
#endif 

// 4.3 RIVER ACTIONS								
				// EV calcs, set-up
				double dEV_Rx_T_ftr = 0.0; // (R EV | T=turn actions; ftr=flop,turn,river cards)
				// Calculate EV over bettor river actions
				for (int nRAct_B = Strategy::BETTOR_RIVER_CHECK; nRAct_B < Strategy::BETTOR_RIVER_NUMACTIONS; ++nRAct_B) {

					double dRiverBet = (Strategy::BETTOR_TURN_CHECK == nRAct_B)?(0.0):dTurnPot_onCall;
					double dRiverPot_onCall = dTurnPot_onCall + 2.0*dRiverBet;

					pEl = eqFreqCaller.at(nC, 0, nRiver); // (need to reset because of multiple river actions)
					
					// EV calcs, set-up
					const double *dStrategyTable_river = (*pStrategy_B)._dRiverAction[nTAct_B][nRiver][nRAct_B];

					// Calculate weighted equity of C vs B given cards, action
					
					double dHands_B_C = 0.0;
					double dWeightedSumProbs_C = 0.0;
					double dWeightedEquity_C = 0.0;

					for (int nB = nBettorRange; nB > 0; --nB) { // (pointlessly optimized loop!)

						double dTurnStratFreq = *dStrategyTable_turn++; 
						double dRiverStratFreq = *dStrategyTable_river++;

						double dHandStratFreq = dTurnStratFreq*pEl->dFreq;
							// (Adjustment for turn action) * (combinations of B hands on this board) 								
						dHands_B_C += dHandStratFreq; // (hands that have reached here) 
						
						dWeightedEquity_C += dRiverStratFreq*(dTurnStratFreq*pEl->dEquityFreq);
						dWeightedSumProbs_C += dRiverStratFreq*dHandStratFreq;

#ifdef __INNER_LOOP_DEBUG_CALLER							
#if 1
if ((bDisp)&&(dTurnStratFreq*dRiverStratFreq>0.0)) 
	printf("[%d %d] B %s%s (%.2f) v C %s%s (%.2f) on R %s (%.2f) = %.2f (p=%.2f*%.2f) [%.2f %.2f %.2f]\n", nTAct_B, nRAct_B,  
CCard(pBettorRange[nBettorRange-nB].nHole1,0).s().c_str(), CCard(pBettorRange[nBettorRange-nB].nHole2,0).s().c_str(), pEl->dFreq,
CCard(pCallerRange[nC].nHole1,0).s().c_str(), CCard(pCallerRange[nC].nHole2,0).s().c_str(), dHandFreq_C,
CCard(nRiver,0).s().c_str(), dRiverFreq,
pEl->dEquityFreq, dTurnStratFreq, dRiverStratFreq,
dHands_B_C, dWeightedEquity_C, dWeightedSumProbs_C);
#endif //1|0
#endif
						pEl++; 
													
					} // end loop over V==B hands					
					dStrategyTable_turn -= nBettorRange; // (reset for next iteration)

					double dEV_Rc_TR_ftr = dRiverPot_onCall*(dWeightedEquity_C/dWeightedSumProbs_C) - dRiverBet;
					if (dWeightedSumProbs_C < 1.0e-9) {
						dEV_Rc_TR_ftr = dTurnPot_onCall;
					}
						// (EV of call/check-behind on this river, given the action)

#ifdef __INNER_LOOP_DEBUG_CALLER
if (bDisp) 
		printf("(%d %d) %s%s r %s.. SUM %f = %f*%f/%f - %f (%f)\n", nTAct_B, nRAct_B, 
	CCard(pCallerRange[nC].nHole1,0).s().c_str(), CCard(pCallerRange[nC].nHole2,0).s().c_str(), CCard(nRiver,0).s().c_str(), 
	dEV_Rc_TR_ftr, dRiverPot_onCall, dWeightedEquity_C, dWeightedSumProbs_C, dRiverBet, dHands_B_C);
#endif
					(*pValue_C)._dRiverAction[nTAct_B][nRiver][nRAct_B][nC] = 
						dNegAlpha*(*pValue_C)._dRiverAction[nTAct_B][nRiver][nRAct_B][nC] + 
							dAlpha*dEV_Rc_TR_ftr;
					
					if (dEV_Rc_TR_ftr > 0.0) { // call river
						(*pStrategy_C)._dRiverAction[nTAct_B][nRiver][nRAct_B][nC] = 
							dNegAlpha*(*pStrategy_C)._dRiverAction[nTAct_B][nRiver][nRAct_B][nC] + dAlpha;
					}
					else { // fold river
						(*pStrategy_C)._dRiverAction[nTAct_B][nRiver][nRAct_B][nC] *= dNegAlpha;
						dEV_Rc_TR_ftr = 0.0;
					}
					// Value of strategy 
					// Just for display purposes...
					if (bFinalIt) {		
						(*pStrategy_C)._dRiverActionOverRange[nTAct_B][nRiver][nRAct_B] +=
							(*pStrategy_C)._dTurnAction[nTAct_B][nC]*dHandFreq_C*
								(*pStrategy_C)._dRiverAction[nTAct_B][nRiver][nRAct_B][nC];
								
						if (dEV_Rc_TR_ftr > 0.0) {					
							(*pValue_C)._dRiverActionOverRange[nTAct_B][nRiver][nRAct_B] +=
								(*pStrategy_C)._dTurnAction[nTAct_B][nC]*dHandFreq_C*
									(*pValue_C)._dRiverAction[nTAct_B][nRiver][nRAct_B][nC];
						}						
						// (else fold with this hand)
						
						dHands_C_[nTAct_B][nRiver] += (*pStrategy_C)._dTurnAction[nTAct_B][nC]*dHandFreq_C;
					}

					if (dHands_B_C > 1.0e-9) {
						dEV_Rx_T_ftr += dEV_Rc_TR_ftr*(dWeightedSumProbs_C/dHands_B_C); 
							// = (river EV) * (prob of river action) = (EV of river given cards, action to turn)
					}
					else if (Strategy::BETTOR_TURN_CHECK == nTAct_B) { // entire range checked, meaning entire range is null
						(*pStrategy_C)._dRiverAction[nTAct_B][nRiver][nRAct_B][nC] = -1.0;
					}
					
				} // end loop over V==B river actions										
// ^^^ 4.3 RIVER ACTIONS					
				
				dCards_R_C += dRiverFreq;
				dEV_Rx_T_ft += dRiverFreq*dEV_Rx_T_ftr; 
					// (sum of EV across all river cards, times river frequencies)
				
			} // end loop over river cards

// ^^^ 4.2 RIVER
							
			dEV_Rx_T_ft /= dCards_R_C; // (calc river EV by multiplying by const river frequency)
			
			double dEV_Tc_T_ft = dEV_Rx_T_ft - dTurnBet;
				// (EV of turn call/check-behind on this turn, given the action)

			(*pValue_C)._dTurnAction[nTAct_B][nC] = dNegAlpha*(*pValue_C)._dTurnAction[nTAct_B][nC] + dAlpha*dEV_Tc_T_ft;
			if (dEV_Tc_T_ft > 0.0) { // call turn
				(*pStrategy_C)._dTurnAction[nTAct_B][nC] = dNegAlpha*(*pStrategy_C)._dTurnAction[nTAct_B][nC] + dAlpha;
				dStrategy_C_dTurnActionOverRange[nTAct_B] += dAlpha*dHandFreq_C;
			}
			else { // fold turn
				(*pStrategy_C)._dTurnAction[nTAct_B][nC] *= dNegAlpha;
				//dStrategy_C_dTurnActionOverRange += 0; // (<- obviously don't bother including this)
				dEV_Tc_T_ft = 0.0;
			}
			//TODO: need to adjust somehow turn action over range to take into account what hands I hold? (ie hand replacement issues...)
			
			// Value of strategy
			dValue_C_dTurnActionOverRange[nTAct_B] += dAlpha*dEV_Tc_T_ft*dHandFreq_C;
			
			dEV_Tx_T_ft += dEV_Tc_T_ft*(*pStrategy_B)._dTurnActionOverRange[nTAct_B]; 
				// (value multiplied by probability that B took this action, over his range)
			
		} // end loop over V==B turn actions

		dHands_C += dHandFreq_C;
		dEV_Tx_ft += dHandFreq_C*dEV_Tx_T_ft; // (sum of EV across all C cards, times caller frequencies)
		
// ^^^ 4.1 TURN ACTIONS			
		
	} // end loop over H==C hands


	double dInvHands_C = 1.0/dHands_C;
	dEV_Tx_ft *= dInvHands_C;

	for (int x = Strategy::CALLER_TURN_FOLD; x < Strategy::CALLER_TURN_NUMACTIONS; ++x) {
		pStrategy_C->_dTurnActionOverRange[x] += dStrategy_C_dTurnActionOverRange[x]*dInvHands_C;
		pValue_C->_dTurnActionOverRange[x] += dValue_C_dTurnActionOverRange[x]*dInvHands_C;
		
		if (bFinalIt) {
			for (int nR = 0; nR < transParams.nCollapsibleNum; nR++) {
				for (int nRA = Strategy::CALLER_RIVER_FOLD; nRA <  Strategy::CALLER_RIVER_NUMACTIONS; nRA++)
				{
					pStrategy_C->_dRiverActionOverRange[x][nR][nRA] *= dInvHands_C;
					pValue_C->_dRiverActionOverRange[x][nR][nRA] *= dInvHands_C;
				}
			}
		}				
	}
	return dEV_Tx_ft;
}

///////////////////////////////////////////////////////////////////////////////////////////

// And in the other direction

// (note here I'm manually unrolling the loop, to see what difference it makes...)
// Takes 60 cycles (vs 80 for above, same parameters)

static double calculateBestResponse_bettor(
		Strategy *pStrategy_B, Strategy *pValue_B, const EquityFreqTable& eqFreqBettor, const struct TranslatedHandInfo *pBettorRange, int nBettorRange,
		const Strategy *pStrategy_C, const struct TranslatedHandInfo *pCallerRange, int nCallerRange, 
		double dAlpha, double dInitialPot, const struct TransformParameters& transParams,
		bool bFinalIt)		
{		
	double dNegAlpha = 1.0 - dAlpha;
	
	// EV calcs, set-up
	double dEV_Tx_ft = 0.0; // (T EV | ft = flop,turn cards)

	(*pStrategy_B)._dTurnActionOverRange[0] *= dNegAlpha;
	(*pStrategy_B)._dTurnActionOverRange[1] *= dNegAlpha;
	(*pValue_B)._dTurnActionOverRange[0] *= dNegAlpha;
	(*pValue_B)._dTurnActionOverRange[1] *= dNegAlpha;

	// (temp variable for above)
	double dStrategy_B_dTurnActionOverRange[Strategy::CALLER_RIVER_NUMACTIONS] = { 0.0, 0.0 };
	double dValue_B_dTurnActionOverRange[Strategy::CALLER_RIVER_NUMACTIONS] = { 0.0, 0.0 };

	double dPot_tKK_rKK = dInitialPot;
	double dPot_tKK_rBC = 3.0*dInitialPot;
	double dPot_tBC_rKK = 3.0*dInitialPot;
	double dPot_tBC_rBC = 9.0*dInitialPot;

	double dHands_B = 0;
	double dHands_B_[2][transParams.nCollapsibleNum];
	if (bFinalIt) {
		for (int x = 0; x < Strategy::BETTOR_TURN_NUMACTIONS; ++x) {
			for (int i = 0; i < transParams.nCollapsibleNum; ++i) {
				dHands_B_[x][i] = 0.0;
			}
		}
	}
	
	const EquityFreqTable::Element *pEl = eqFreqBettor.at(0, 0, 0);
	for (int nB = 0; nB < nBettorRange; ++nB) {
		
		if (pEl->dEquityFreq < -3.5) {
			pStrategy_B->_dTurnAction[0][nB] = -4.0;
			pEl = eqFreqBettor.at(nB + 1, 0, 0);
			continue;
		}
		double dHandFreq_B = pBettorRange[nB].dCacheFreq; // hand combos given board

// 5.1 TURN ACTIONS			

		// EV calcs, set-up
		
		// EV(turn B)=pFOLD*(initial pot) + pCALL*EV(river|turn BC) - bet
		// EV(turn K) = EV(river|turn KK)
		// EV(river B|turn BC)=pFOLD*(bet pot) + pCALL*eq(|turn B)*(2xbet pot) - bet
		// EV(river K|turn BC) = eq(|turn B)*(bet pot)
		// EV(river B|turn K) = pFOLD*(initial pot) + pCALL*eq(|turnK)*(bet pot) - bet
		// EV(river K|turn K) = eq(|turn K)*(initial pot)
		
		// Which can all be written: (pFOLD - 1.0)*bet + pCALL*X*pot, where X is equity or river EV
		// eq(|turn K) = unweighted flop frequencies

		double *dStrategyTable_turn = (*pStrategy_C)._dTurnAction[Strategy::CALLER_TURN_CALL]; 
			// (obviously not interested in turn folds, and range doesn't change if bettor checks)
		
// 5.2 RIVER			
		
		// Calculate EV over rivers
		double dCards_R_B = 0; // total river card weighting given board, C
		
		double dEV_Rx_T_ft_tKK = 0.0;
		double dEV_Rx_T_ft_tBC = 0.0;
		
		for (int nRiver = 0; nRiver < transParams.nCollapsibleNum; ++nRiver) {
			if (pEl->dEquityFreq < -2.5) {
				pStrategy_B->_dRiverAction[0][nRiver][0][nB] = -3.0;
				pEl = eqFreqBettor.at(nB, 0, nRiver + 1);
				continue;
			}			
			if (_nRiverOnly != nRiver) {
				pEl = eqFreqBettor.at(nB, 0, nRiver + 1);
				continue;
			}
			
			double dRiverFreq = pBettorRange[nB].dRiverCacheFreq[nRiver]; // river combos given board, bettor hands			
			double *dStrategyTable_river_tKK = (*pStrategy_C)._dRiverAction[Strategy::CALLER_TURN_FOLD][nRiver][Strategy::CALLER_RIVER_CALL];
			double *dStrategyTable_river_tBC = (*pStrategy_C)._dRiverAction[Strategy::CALLER_TURN_CALL][nRiver][Strategy::CALLER_RIVER_CALL];
				// (obviously not interested in river folds)

//#define __INNER_LOOP_DEBUG_BETTOR	

#ifdef __INNER_LOOP_DEBUG_BETTOR
bool bDisp = false;					
//CCard h1(pBettorRange[nB].nHole1,0), h2(pBettorRange[nB].nHole2,0), r(nRiver,0); 
//if ((nRiver == 21) && (h1.getRank() == 14) && (h2.getRank() == 10) && bFinalIt) bDisp = true;
if (bFinalIt) bDisp = true;
#endif 

// 4.3 RIVER ACTIONS								

			// EV calcs, set-up

			//double dHands_B_tBC = 0.0, dHands_B_tKK = 0.0; equivalent to dWeightedSumProbs_X_rKK
			
			double dWeightedSumProbs_B_tKK_rKK = 0.0, dWeightedSumProbs_B_tKK_rBC = 0.0, 
					dWeightedSumProbs_B_tBC_rBC = 0.0, dWeightedSumProbs_B_tBC_rKK = 0.0;
			
			double dWeightedEquity_B_tKK_rKK = 0.0, dWeightedEquity_B_tKK_rBC = 0.0, 
					dWeightedEquity_B_tBC_rBC = 0.0, dWeightedEquity_B_tBC_rKK = 0.0;

			for (int nC = nCallerRange; nC > 0; --nC) { // (pointlessly optimized loop!)
	
				double dTurnStratFreq = *dStrategyTable_turn++; 
				double dRiverStratFreq_tKK = *dStrategyTable_river_tKK++;
				double dRiverStratFreq_tBC = *dStrategyTable_river_tBC++;
	
				double dElFreq = pEl->dFreq;
				double dElEqFreq = pEl->dEquityFreq;
				
				double dHandStratFreq_tBC = dTurnStratFreq*dElFreq;
					// (Adjustment for turn action) * (combinations of B hands on this board) 								
				
				double dEqFreq_tBC = dTurnStratFreq*dElEqFreq;
				
				// All the different combinations:
				dWeightedEquity_B_tBC_rKK += dEqFreq_tBC;
				dWeightedSumProbs_B_tBC_rKK += dHandStratFreq_tBC;
				
				dWeightedEquity_B_tBC_rBC += dRiverStratFreq_tBC*dEqFreq_tBC;
				dWeightedSumProbs_B_tBC_rBC += dRiverStratFreq_tBC*dHandStratFreq_tBC;
				
				dWeightedEquity_B_tKK_rBC += dRiverStratFreq_tKK*dElEqFreq;
				dWeightedSumProbs_B_tKK_rBC += dRiverStratFreq_tKK*dElFreq;
				
				dWeightedEquity_B_tKK_rKK += dElEqFreq;
				dWeightedSumProbs_B_tKK_rKK += dElFreq;
	
#ifdef __INNER_LOOP_DEBUG_BETTOR					
#if 1
if (bDisp && (pEl->dFreq > 0.00001)) 
printf("C %s%s (%.2f) v B %s%s (%.2f) on R %s (%.2f) = %.2f: KK/KK << KK/BC %.2f/ BC/KK %.2f BC/BC %.2f\n",   
CCard(pCallerRange[nCallerRange-nC].nHole1,0).s().c_str(), CCard(pCallerRange[nCallerRange-nC].nHole2,0).s().c_str(), pEl->dFreq,
CCard(pBettorRange[nB].nHole1,0).s().c_str(), CCard(pBettorRange[nB].nHole2,0).s().c_str(), dHandFreq_B,
CCard(nRiver,0).s().c_str(), dRiverFreq,
pEl->dEquityFreq,
dRiverStratFreq_tKK*dElFreq, dTurnStratFreq*dElFreq, dRiverStratFreq_tBC*dTurnStratFreq*dElFreq
);
#endif //1|0
#endif
				pEl++; 
											
			} // end loop over V==C hands					
			
			dStrategyTable_turn -= nCallerRange; // (reset for the next loop) 
			
			// EVs of all the different scenarios:
			
			// River checked:
			double dEV_Rc_TR_ftr_tKK_rKK = dPot_tKK_rKK*(dWeightedEquity_B_tKK_rKK/dWeightedSumProbs_B_tKK_rKK);
			if (dWeightedSumProbs_B_tKK_rKK < 1.0e-9) {
				dEV_Rc_TR_ftr_tKK_rKK = dPot_tKK_rKK;
			}
			double dEV_Rc_TR_ftr_tBC_rKK = dPot_tBC_rKK*(dWeightedEquity_B_tBC_rKK/dWeightedSumProbs_B_tBC_rKK);
			if (dWeightedSumProbs_B_tBC_rKK < 1.0e-9) {
				dEV_Rc_TR_ftr_tBC_rKK = 0.0;
			}
			
			// River bet:
			double dPCall_tKK_rBx = dWeightedSumProbs_B_tKK_rBC/dWeightedSumProbs_B_tKK_rKK;
			double dEq_tKK_rBX = dWeightedEquity_B_tKK_rBC/dWeightedSumProbs_B_tKK_rBC;
			double dEV_Rc_TR_ftr_tKK_rBx = dPCall_tKK_rBx*(dPot_tKK_rBC*dEq_tKK_rBX - 2.0*dPot_tKK_rKK) + dPot_tKK_rKK;
			if ((dWeightedSumProbs_B_tKK_rBC < 1.0e-9)||(dWeightedSumProbs_B_tKK_rKK < 1.0e-9)) {
				dEV_Rc_TR_ftr_tKK_rBx = dPot_tKK_rKK; // (ie C never calls so just get the pot...)
			}
			
			double dPCall_tBC_rBx = dWeightedSumProbs_B_tBC_rBC/dWeightedSumProbs_B_tBC_rKK;
			double dEq_tBC_rBX = dWeightedEquity_B_tBC_rBC/dWeightedSumProbs_B_tBC_rBC;
			double dEV_Rc_TR_ftr_tBC_rBx = dPCall_tBC_rBx*(dPot_tBC_rBC*dEq_tBC_rBX - 2.0*dPot_tBC_rKK) + dPot_tBC_rKK;
			if ((dWeightedSumProbs_B_tBC_rKK < 1.0e-9)||(dWeightedSumProbs_B_tBC_rBC < 1.0e-9)) {
				dEV_Rc_TR_ftr_tBC_rBx = 0.0;
			}

#ifdef __INNER_LOOP_DEBUG_BETTOR
if (bDisp) 
printf(">> %s%s r %s.. KK/KK = %.2f e %.2f KK/Bx = %.2f e %.2f (%.2f) BC/KK = %.2f e %.2f BC/Bx = %.2f e %.2f (%.2f)\n",  
CCard(pBettorRange[nB].nHole1,0).s().c_str(), CCard(pBettorRange[nB].nHole2,0).s().c_str(), CCard(nRiver,0).s().c_str(),
dEV_Rc_TR_ftr_tKK_rKK, dWeightedEquity_B_tKK_rKK/dWeightedSumProbs_B_tKK_rKK,
dEV_Rc_TR_ftr_tKK_rBx, dEq_tKK_rBX, dPCall_tKK_rBx, 
dEV_Rc_TR_ftr_tBC_rKK, (dWeightedEquity_B_tBC_rKK/dWeightedSumProbs_B_tBC_rKK),
dEV_Rc_TR_ftr_tBC_rBx, dEq_tBC_rBX, dPCall_tBC_rBx);
#endif

			// Calculate river actions and values:

			(*pValue_B)._dRiverAction[Strategy::BETTOR_TURN_CHECK][nRiver][Strategy::BETTOR_RIVER_CHECK][nB] *= dNegAlpha;
			(*pValue_B)._dRiverAction[Strategy::BETTOR_TURN_CHECK][nRiver][Strategy::BETTOR_RIVER_CHECK][nB] += dAlpha*dEV_Rc_TR_ftr_tKK_rKK;
			(*pValue_B)._dRiverAction[Strategy::BETTOR_TURN_CHECK][nRiver][Strategy::BETTOR_RIVER_POT][nB] *= dNegAlpha;
			(*pValue_B)._dRiverAction[Strategy::BETTOR_TURN_CHECK][nRiver][Strategy::BETTOR_RIVER_POT][nB] += dAlpha*dEV_Rc_TR_ftr_tKK_rBx;
			(*pValue_B)._dRiverAction[Strategy::BETTOR_TURN_POT][nRiver][Strategy::BETTOR_RIVER_CHECK][nB] *= dNegAlpha;
			(*pValue_B)._dRiverAction[Strategy::BETTOR_TURN_POT][nRiver][Strategy::BETTOR_RIVER_CHECK][nB] += dAlpha*dEV_Rc_TR_ftr_tBC_rKK;
			(*pValue_B)._dRiverAction[Strategy::BETTOR_TURN_POT][nRiver][Strategy::BETTOR_RIVER_POT][nB] *= dNegAlpha;
			(*pValue_B)._dRiverAction[Strategy::BETTOR_TURN_POT][nRiver][Strategy::BETTOR_RIVER_POT][nB] += dAlpha*dEV_Rc_TR_ftr_tBC_rBx;

			double dEV_Rx_T_ftr_tKK, dEV_Rx_T_ftr_tBC;
			if (dEV_Rc_TR_ftr_tKK_rKK > dEV_Rc_TR_ftr_tKK_rBx) { // Check the river (turn checked)				
				(*pStrategy_B)._dRiverAction[Strategy::BETTOR_TURN_CHECK][nRiver][Strategy::BETTOR_RIVER_CHECK][nB] = 
					dNegAlpha*(*pStrategy_B)._dRiverAction[Strategy::BETTOR_TURN_CHECK][nRiver][Strategy::BETTOR_RIVER_CHECK][nB] + dAlpha;				
				(*pStrategy_B)._dRiverAction[Strategy::BETTOR_TURN_CHECK][nRiver][Strategy::BETTOR_RIVER_POT][nB] *= dNegAlpha;
				dEV_Rx_T_ftr_tKK = dEV_Rc_TR_ftr_tKK_rKK;
			}
			else { // Bet the river (turn checked)
				(*pStrategy_B)._dRiverAction[Strategy::BETTOR_TURN_CHECK][nRiver][Strategy::BETTOR_RIVER_CHECK][nB] *= dNegAlpha; 
				(*pStrategy_B)._dRiverAction[Strategy::BETTOR_TURN_CHECK][nRiver][Strategy::BETTOR_RIVER_POT][nB] = 
					dNegAlpha*(*pStrategy_B)._dRiverAction[Strategy::BETTOR_TURN_CHECK][nRiver][Strategy::BETTOR_RIVER_POT][nB] + dAlpha;				
				dEV_Rx_T_ftr_tKK = dEV_Rc_TR_ftr_tKK_rBx;
			}
			if (dEV_Rc_TR_ftr_tBC_rKK > dEV_Rc_TR_ftr_tBC_rBx) { // Check the river (turn bet)				
				(*pStrategy_B)._dRiverAction[Strategy::BETTOR_TURN_POT][nRiver][Strategy::BETTOR_RIVER_CHECK][nB] = 
					dNegAlpha*(*pStrategy_B)._dRiverAction[Strategy::BETTOR_TURN_POT][nRiver][Strategy::BETTOR_RIVER_CHECK][nB] + dAlpha;				
				(*pStrategy_B)._dRiverAction[Strategy::BETTOR_TURN_POT][nRiver][Strategy::BETTOR_RIVER_POT][nB] *= dNegAlpha; 
				dEV_Rx_T_ftr_tBC = dEV_Rc_TR_ftr_tBC_rKK;
			}
			else { // Bet the river (turn bet)
				(*pStrategy_B)._dRiverAction[Strategy::BETTOR_TURN_POT][nRiver][Strategy::BETTOR_RIVER_CHECK][nB] *= dNegAlpha; 
				(*pStrategy_B)._dRiverAction[Strategy::BETTOR_TURN_POT][nRiver][Strategy::BETTOR_RIVER_POT][nB] = 
					dNegAlpha*(*pStrategy_B)._dRiverAction[Strategy::BETTOR_TURN_POT][nRiver][Strategy::BETTOR_RIVER_POT][nB] + dAlpha;				
				dEV_Rx_T_ftr_tBC = dEV_Rc_TR_ftr_tBC_rBx;
			}
			// Handle cases where there are no hands in the range:
			if (dWeightedSumProbs_B_tKK_rKK < 1.0e-9) {
				(*pStrategy_B)._dRiverAction[Strategy::BETTOR_TURN_CHECK][nRiver][Strategy::BETTOR_RIVER_CHECK][nB] = -1;
				(*pStrategy_B)._dRiverAction[Strategy::BETTOR_TURN_CHECK][nRiver][Strategy::BETTOR_RIVER_POT][nB] = -1;
				(*pStrategy_B)._dRiverAction[Strategy::BETTOR_TURN_POT][nRiver][Strategy::BETTOR_RIVER_CHECK][nB] = -1;
				(*pStrategy_B)._dRiverAction[Strategy::BETTOR_TURN_POT][nRiver][Strategy::BETTOR_RIVER_POT][nB] = -1;				
			}
		
			// Value of strategy 
			// Just for display purposes...
			if (bFinalIt) {
//TODO: move all this functionality over to the caller code	
				for (int i = Strategy::BETTOR_TURN_CHECK; i < Strategy::BETTOR_TURN_NUMACTIONS; ++i) {
					for (int j = Strategy::BETTOR_RIVER_CHECK; j < Strategy::BETTOR_RIVER_NUMACTIONS; ++j) {
								
						(*pStrategy_B)._dRiverActionOverRange[i][nRiver][j] +=
							(*pStrategy_B)._dTurnAction[i][nB]*dHandFreq_B*
								(*pStrategy_B)._dRiverAction[i][nRiver][j][nB];
								
						(*pValue_B)._dRiverActionOverRange[i][nRiver][j] +=
							(*pStrategy_B)._dTurnAction[i][nB]*dHandFreq_B*
								(*pValue_B)._dRiverAction[i][nRiver][j][nB];
					}
					dHands_B_[i][nRiver] += (*pStrategy_B)._dTurnAction[i][nB]*dHandFreq_B;
				} // (end loop over turn,river actions)		
			}

// ^^^ 5.3 RIVER ACTIONS								
						
			dCards_R_B += dRiverFreq;
			dEV_Rx_T_ft_tKK += dRiverFreq*dEV_Rx_T_ftr_tKK; 
			dEV_Rx_T_ft_tBC += dRiverFreq*dEV_Rx_T_ftr_tBC; 
				// (sum of EV across all river cards, times river frequencies)
			
		} // end loop over rivers
		
// ^^^ 5.2 RIVER
				
		dEV_Rx_T_ft_tKK /= dCards_R_B; // (calc river EV by multiplying by const river frequency)
		dEV_Rx_T_ft_tBC /= dCards_R_B;
			
		double dEV_Tc_T_ft_tKK = dEV_Rx_T_ft_tKK;
		double dProbTurnCall = (*pStrategy_C)._dTurnActionOverRange[Strategy::CALLER_TURN_CALL];
		double dEV_Tc_T_ft_tBx = dPot_tKK_rKK + dProbTurnCall*(dEV_Rx_T_ft_tBC - 2.0*dPot_tKK_rKK);
			// (EV of turn actions on this turn)
		
		//TODO: ^^^ I'm a bit worried about the turn action over range because of hand replacement issues... (eg my hand will affect his calling %) 
		//Also a problem for the other BR function I would imagine
		
		// Calculate values and ranges
		
		(*pValue_B)._dTurnAction[Strategy::BETTOR_TURN_CHECK][nB] = dNegAlpha*(*pValue_B)._dTurnAction[Strategy::BETTOR_TURN_CHECK][nB] + 
																		dAlpha*dEV_Tc_T_ft_tKK;
		(*pValue_B)._dTurnAction[Strategy::BETTOR_TURN_POT][nB] = dNegAlpha*(*pValue_B)._dTurnAction[Strategy::BETTOR_TURN_POT][nB] + 
																		dAlpha*dEV_Tc_T_ft_tBx;
#ifdef __OUTER_LOOP_DEBUG_BETTOR
		printf("%s%s (%.2f) KK %.2f Bx %.2f (p %.2f v %.2f)\n",
				CCard(pBettorRange[nB].nHole1,0).s().c_str(), CCard(pBettorRange[nB].nHole2,0).s().c_str(), dHandFreq_B,
				dEV_Tc_T_ft_tKK, dEV_Tc_T_ft_tBx, dProbTurnCall, dEV_Rx_T_ft_tBC);
#endif
		
		double dEV_Tx_T_ft;
		if ((-1 != _nRiverOnly) || (dEV_Tc_T_ft_tKK > dEV_Tc_T_ft_tBx)) { // Check turn (forced if river only play)
			(*pStrategy_B)._dTurnAction[Strategy::BETTOR_TURN_CHECK][nB] = dNegAlpha*(*pStrategy_B)._dTurnAction[Strategy::BETTOR_TURN_CHECK][nB] + dAlpha;
			(*pStrategy_B)._dTurnAction[Strategy::BETTOR_TURN_POT][nB] *= dNegAlpha;
			dStrategy_B_dTurnActionOverRange[Strategy::BETTOR_TURN_CHECK] += dAlpha*dHandFreq_B;
			dEV_Tx_T_ft = dEV_Tc_T_ft_tKK;
		}
		else { // Bet turn
			(*pStrategy_B)._dTurnAction[Strategy::BETTOR_TURN_CHECK][nB] *= dNegAlpha;
			(*pStrategy_B)._dTurnAction[Strategy::BETTOR_TURN_POT][nB] = dNegAlpha*(*pStrategy_B)._dTurnAction[Strategy::BETTOR_TURN_POT][nB] + dAlpha;
			dStrategy_B_dTurnActionOverRange[Strategy::BETTOR_TURN_POT] += dAlpha*dHandFreq_B;
			dEV_Tx_T_ft = dEV_Tc_T_ft_tBx;
		}
		
		// Value of strategy
		dValue_B_dTurnActionOverRange[Strategy::BETTOR_TURN_CHECK] += dAlpha*dEV_Tc_T_ft_tKK*dHandFreq_B;
		dValue_B_dTurnActionOverRange[Strategy::BETTOR_TURN_POT] += dAlpha*dEV_Tc_T_ft_tBx*dHandFreq_B;
			
// ^^^ 5.1 TURN ACTIONS			
					
		dHands_B += dHandFreq_B;
		dEV_Tx_ft += dHandFreq_B*dEV_Tx_T_ft; // (sum of EV across all C cards, times caller frequencies)
		
	} // end loop over H==B hands

	double dInvHands_B = 1.0/dHands_B;
	dEV_Tx_ft *= dInvHands_B;

	for (int x = Strategy::BETTOR_TURN_CHECK; x < Strategy::BETTOR_TURN_NUMACTIONS; ++x) {
		pStrategy_B->_dTurnActionOverRange[x] += dStrategy_B_dTurnActionOverRange[x]*dInvHands_B;
		pValue_B->_dTurnActionOverRange[x] += dValue_B_dTurnActionOverRange[x]*dInvHands_B;
		
		if (bFinalIt) {
			for (int nR = 0; nR < transParams.nCollapsibleNum; nR++) {
				for (int nRA = Strategy::BETTOR_RIVER_CHECK; nRA <  Strategy::CALLER_RIVER_NUMACTIONS; nRA++)
				{
					pStrategy_B->_dRiverActionOverRange[x][nR][nRA] /= dHands_B_[x][nR];
					pValue_B->_dRiverActionOverRange[x][nR][nRA] /= dHands_B_[x][nR];
				}
			}
		}				
	}
	return dEV_Tx_ft;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// Main loop for GTO solution for scenario described above

// Hero = caller
// Villain = bettor

void GTO_demo2(std::deque<CCard::Hole>& lHeroRange, std::deque<CCard::Hole>& lVillainRange, int *nBoardTable)
{
	std::deque<CCard::Hole>::iterator itB, itC;

// Some parameters
	
	const double dInitialPot = 15;

// 1. Translate Bettor and Caller ranges:

	struct TransformParameters transParams;
	::translateTurnBoard(nBoardTable, &transParams);

	// Update board information
	char szBoard[128];
	if (-1 != nBoardTable[4]) {
		CCard rO(nBoardTable[4]);
		_nRiverOnly = rO.changeSuit(transParams.nTransformTable[rO.getSuit()]).convertToPokerEnum();
		sprintf(szBoard, "BOARD = %s %s %s, %s, %s", 
				CCard(nBoardTable[0]).s().c_str(), CCard(nBoardTable[1]).s().c_str(), CCard(nBoardTable[2]).s().c_str(), CCard(nBoardTable[3]).s().c_str(),
				rO.s().c_str());		
	}
	else {
		sprintf(szBoard, "BOARD = %s %s %s, %s", CCard(nBoardTable[0]).s().c_str(), CCard(nBoardTable[1]).s().c_str(), CCard(nBoardTable[2]).s().c_str(), CCard(nBoardTable[3]).s().c_str());
	}
	printf("%s\n", szBoard);
	
	time_t nCpu_compression = ::clock();
	
	int nBoardFreqTable[::NRANK];
	for (int x = 0; x < ::NRANK; ++x) {
		nBoardFreqTable[x] = transParams.nCollapsibleFreq0;
	}
	// (Flop,turn dead cards)
	StdDeck_CardMask board;
	StdDeck_CardMask dead;
	StdDeck_CardMask_RESET(board);
	StdDeck_CardMask_RESET(dead);
	for (int i = 0; i < 4; ++i) {
		CCard c(nBoardTable[i]);
		int nCard = c.convertToPokerEnum();
		StdDeck_CardMask_SET(board, nCard);		
		
		if (nCard < transParams.nFlushableNum) {
			StdDeck_CardMask_SET(dead, nCard);
		}
		else {
			nBoardFreqTable[StdDeck_RANK(nCard)]--;
		}
	}

	// Update hand information
	int nBettorRange, nCallerRange;
	double dBettorFreq, dCallerFreq;
	struct TranslatedHandInfo *pBettorRange = ::getTranslatedHandTable(lVillainRange, &transParams, board, dead, &nBettorRange, &dBettorFreq);
	struct TranslatedHandInfo *pCallerRange = ::getTranslatedHandTable(lHeroRange, &transParams, board, dead, &nCallerRange, &dCallerFreq);

	printf("Compressed board, hand ranges: %d cycles\n", (int)(::clock() - nCpu_compression));	
	
/////////////////////////////////////////////////////	
	
	time_t nCpu_equity;
	
// 2. Calculate equity table including frequency information

	nCpu_equity = ::clock();	

	std::multimap<double, int> mmCallerEquityOrder[transParams.nCollapsibleNum];

	EquityFreqTable eqFreqCaller(nCallerRange, nBettorRange, transParams.nCollapsibleNum);
	// With 15% and 15%, 86 hands each, results in 5.3MB 

	::calculateEquityTable(eqFreqCaller, pCallerRange, nCallerRange, 
			pBettorRange, nBettorRange, board, dead, nBoardFreqTable, transParams, mmCallerEquityOrder);
	
	printf("Calculated CvsB equity: %d cycles\n", (int)(::clock() - nCpu_equity));	
		// (Note first call to this seems to take longer, not sure why)
	
	nCpu_equity = ::clock();	

	std::multimap<double, int> mmBettorEquityOrder[transParams.nCollapsibleNum];

	EquityFreqTable eqFreqBettor(nBettorRange, nCallerRange, transParams.nCollapsibleNum);
	// With 15% and 15%, 86 hands each, results in 5.3MB 

	::calculateEquityTable(eqFreqBettor, pBettorRange, nBettorRange, 
			pCallerRange, nCallerRange, board, dead, nBoardFreqTable, transParams, mmBettorEquityOrder);
	
	printf("Calculated BvsC equity: %d cycles\n", (int)(::clock() - nCpu_equity));	
	
/////////////////////////////////////////////////////
	
	time_t nCpu_strategy = ::clock();	

	Strategy *pStrategy_B = new Strategy(nBettorRange, transParams.nCollapsibleNum);
	Strategy *pStrategy_C = new Strategy(nCallerRange, transParams.nCollapsibleNum);
	Strategy *pValue_B = new Strategy(nBettorRange, transParams.nCollapsibleNum);
	Strategy *pValue_C = new Strategy(nCallerRange, transParams.nCollapsibleNum);
	pValue_B->initializeRange();
	pValue_C->initializeRange();
	// (uses 1.15MB with 15% and 15%, see above 86 hands each.)

// 3. Calculate initial B strategy: something fairly simple!

	::calculateInitialBettorStrategy(eqFreqBettor, pStrategy_B, pBettorRange, nBettorRange, 
										nCallerRange, transParams);
	
	printf("Calculated B initial strategy: %d cycles\n", (int)(::clock() - nCpu_strategy));
	
// 2. Iterate to correct solution through asymmetric fictious play algorithm	

	nCpu_strategy = ::clock();	

	int nIts = 0;
	static const int nMaxIts = 1000;
	double dEV_Tx_ft_C = 0.0, dEV_Tx_ft_B = 0.0; // (T EV | ft = flop,turn cards)
	
	for (nIts = 0; nIts <= nMaxIts; ++nIts) {

		double dAlpha = 1.0/(1 + nIts);
		
/////////////////////////////////////////////////////		
		
// 4. Calculate C strategy as best response to B's strategy
		
		dEV_Tx_ft_C = ::calculateBestResponse_caller(
				pStrategy_C, pValue_C, eqFreqCaller, pCallerRange, nCallerRange,
				pStrategy_B, pBettorRange, nBettorRange, dAlpha, dInitialPot, transParams, nIts==nMaxIts);

/////////////////////////////////////////////////////		
		
// 5. Calculate B strategy as best response to C's strategy		

		dEV_Tx_ft_B = ::calculateBestResponse_bettor(
				pStrategy_B, pValue_B, eqFreqBettor, pBettorRange, nBettorRange,
				pStrategy_C, pCallerRange, nCallerRange, dAlpha, dInitialPot, transParams, nIts==nMaxIts);
	}
	
	nCpu_strategy = ::clock() - nCpu_strategy;
	printf("\nGTO_demo2: strategy calculation: %d iterations, %d cycles\n", nMaxIts, (int)nCpu_strategy);
	printf("GTO_demo2: C overall value = %.2f B overall value = %.2f \n\n", dEV_Tx_ft_C, dEV_Tx_ft_B);

	(*pStrategy_C).summarizeProbs(szBoard, pValue_C, pCallerRange, 'C');
	(*pStrategy_B).summarizeProbs(szBoard, pValue_B, pBettorRange, 'B');

#ifdef __DISPLAY_HAND_DETAILS
	(*pStrategy_C).displayProbs(szBoard, pValue_C, pCallerRange, 'C');
	(*pStrategy_B).displayProbs(szBoard, pValue_B, pBettorRange, 'B');
#endif
	
	::display_equity_distribution(mmCallerEquityOrder, pStrategy_C, pCallerRange, mmBettorEquityOrder, pStrategy_B, pBettorRange, transParams);

//TODO: please to be deleting all the memory you've allocated (hand ranges, strategies/values, freq calculators, by my count!)	
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
