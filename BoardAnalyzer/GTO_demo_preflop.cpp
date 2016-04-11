///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
//
// GTO_demo_preflop
//
///////////////////////////////////////////////////////////////////////////////////////////
//
// Some Game Theory Optimal solutions
//
///////////////////////////////////////////////////////////////////////////////////////////
//
// This one looks at post-3-bet solutions, SB raises, BB 3-bets...
// Then SB can fold, 4-bet or push, BB can fold or 5-bet/call 
//
// Pot size: 3 (1/2 blinds) -> 8 (6 raise) -> 28 (22 3-bet) -> 77 (55 4-bet) -> 255 (200 5-bet-ai)
//                                                          -> 222 (200 4-bet ai)
//
// You'll need to remove the 3-bet calling range from SB's hand distribution
//  - Maybe in the future we can approximate the equity of calling 3-bet for SB
//  - Or more realistically calling 4-bet for BB (but it will probably be equity
//    minus 10% or something so dominated by 5-betting/folding)
//
///////////////////////////////////////////////////////////////////////////////////////////
//
// eg BoardAnalyzer.exe --myrange 6MUTGO --vsrange 6MUTGO --GTO preflop| more
//								^^^^^^^^			^^^^^^^
//						(SB PFR, not 3B-call)		(BB 3B range)
//
///////////////////////////////////////////////////////////////////////////////////////////
//
// Some sample pre-flop ranges:
// 1] HU
// 1.1] HU SB (!call) = 75.6%-{99-66, KQ, AJ, KJ, QJs ,ATs, JTs}=69.7%=
//						TT+, 55-22, AQ+, A9-A2, ATo, KT-K2, QT-Q4, Q3s-Q2s, QJo, J9-J7, J6s-J2s, JTo, T7+, T6s-T2s, 96+, 95s-92s, 85+, 84s-82s, 75+, 74s-72s, 64+, 63s-62s, 53+, 52s, 43, 42s, 32s
// 1.1.1] SB "Results": 4B55/C=TT-AA, AQ+ [4.7%]; 4B55/F=A9, Q9:0.75, QJo [3%] ... $1.6
// 1.2] HU BB polarized PF3b = 15.1%
//						99+, AJ+, A6s-A2s, ATo, K7s-K2s, KQo, T8s, 97s+, 86s+, 75s+, 64s+, 53s+, 43s
// 1.2.1] BB "Results": 5B/CAI: 99-AA, AQ+ [5.1%]; 5B: K5s, K4s:0.33 [0.4%] ... $26.4
// 1.2.1.1] Eq(K5s,K4s:0.33;TT-AA,AQ+)=30.08, Eq(99-AA,AQ+;TT-AA,AQ+)=51.42 ... heh not convinced, more display required... 
// 1.3] HU BB value-range-PF3b=17.9%
//						88+, A9+, A8s-A7s, KT+, K9s, QT+, Q9s, JT, J9s
// 1.3.1] SB "Results": 4B55/C=TT+,55,AQ+,A5s:0.6 [5.3%] 4B55/F=A8,K8 [2.4%]
// 1.3.2] BB "Results": 5B=88+,AJs:0.75,AQ+ [5.8%],KJs,KQs,KTs:0.7,QTs:0.33[0.9%]
//
// 2] 6M B v xB
// 2.1] B (!call) = 37.6% = [43.6%]{22+, A2+, A7o:0.5, A6o:0.5, A5o:0.5, A4o:0.5, A3o:0.5, A2o:0.5, K9+, K8s-K2s, Q9+, Q8s-Q4s, J9+, J8s-J7s, T9, T8s-T7s, 98, 97s-96s, 87, 86s-85s, 74s+, 63s+, 53s+, 43s}-(KQ, AQ, AJ, 88-99, TJs+, J9s+,AA:0.5,KK:0.5}[5.6%]
//					AA:0.5, KK:0.5, TT-QQ, 77-22, AK, AT-A2, KT-K9, K8s-K2s, KJo, Q9, Q8s-Q4s, QTo+, J8s-J7s, J9o+, T9, T8s-T7s, 98, 97s-96s, 87, 86s-85s, 74s+, 63s+, 53s+, 43s
// 2.1.1] "Results": 4B55/C=TT+,AK (3%); 4B55/F=KTs,KTo:0.6,KJo:0.67 (1.5%) ... $1.5
//
// 2.2] xB raise (value + bluffs) = [11.2%] =
//					99+, AJ+, ATs, A9s:0.5, A8s:0.5, A7s:0.5, A6s:0.5, A5s:0.5, A4s:0.5, A3s:0.5, A2s:0.5, KQ, KJs-KTs, QJs, 97s:0.5, 87s:0.5, 86s:0.5, 76s:0.5, 75s:0.5, 65s:0.5, 64s:0.5, 54s:0.5
// 2.2.1] "Results": 5B=TT+,AQs,AK [3.8%],A3s,KQs:0.5,QJs[0.6%] ... $26.5
//
// 3] 6M CO v B
// 3.1] CO = 24.9% = {22+, AT+, A9s-A2s, KT+, K9s-K6s, QJ, QTs-Q8s, J8s+, T8s+, 97s+, 86s+, 75s+, 64s+, 53s+, 43s}
// 3.1.1] "Results": 4B55/C=77:0.60, 88+, AJs:0.7, AQ+, KQs:0.2 [5.9%]; 4B55/F=AT:0.8,AJo,KQo [2.7%]; 4BAI=77:0.4,AJs:0.3 [0.3%] ... $4.8
// 3.1.1.1] (If you weight down calling hands for B, get diff 4B/C=77:0.9,KQs:0.0,AJs:0.0 4B55/F=AT:0.6,AJs:0.4; 4BAI=77:0.1)
//           So something like 4B55/C=77+,AQ+; 4B55/F=AT:0.6,AJs:0.4,AJo,KQo...
// 3.2] B = 0.5 of range minus "value 3-b" = 14.45% = {TT+, 99:0.5, 88:0.5, 77:0.5, 66:0.5, 55:0.5, 44:0.5, 33:0.5, 22:0.5, AQ+, AJ:0.5, AT:0.5, A9s:0.5, A8s:0.5, A7s:0.5, A6s:0.5, A5s:0.5, A4s:0.5, A3s:0.5, A2s:0.5, KQ, KQo:0.5, KJ:0.5, KT:0.5, K9s:0.5, K8s:0.5, K7s:0.5, K6s:0.5, QJ:0.5, QTs:0.5, Q9s:0.5, Q8s:0.5, JTs:0.5, J9s:0.5, J8s:0.5, T9s:0.5, T8s:0.5, 98s:0.5, 97s:0.5, 87s:0.5, 86s:0.5, 76s:0.5, 75s:0.5, 65s:0.5, 64s:0.5, 54s:0.5, 53s:0.5, 43s:0.5}
// 3.2.1] "Results": 5B/C=77:0.85, 88+, AJs,AQ+,KQs,KJs,QJs [7.1%] ... $23.2
//
// 4] 6M CO v LAG B
// 4.1] CO = 24.9% = {22+, AT+, A9s-A2s, KT+, K9s-K6s, QJ, QTs-Q8s, J8s+, T8s+, 97s+, 86s+, 75s+, 64s+, 53s+, 43s}
// 4.2] B = ~36% = {TT+, 99:0.66, 88:0.66, 77:0.33, 66:0.33, 55:0.33, 44:0.33, 33:0.33, 22:0.33, AQ+, A8o-A2o, AJ:0.66, AT:0.66, A9:0.66, A8s:0.66, A7s:0.66, A6s:0.66, A5s:0.66, A4s:0.66, A3s:0.66, A2s:0.66, KTs-K9s, KQ:0.66, KJ:0.66, KTs:0.66, K9s:0.66, K8s:0.66, K7s:0.66, K6s:0.66, K5s:0.66, K4s:0.66, K3s:0.66, K2s:0.66, QT-Q9, Q8s-Q4s, QJs:0.33, QJo:0.66, QTs:0.66, J9+, J8s-J7s, JTs:0.33, J9s:0.66, T9, T7s, T9s:0.33, T8s:0.66, 98, 96s, 98s:0.66, 97s:0.66, 87, 85s, 87s:0.66, 86s:0.66, 74s, 76s:0.66, 75s:0.66, 63s, 65s:0.66, 64s:0.66, 53s, 54s:0.66, 43s}
// 4.3] Results:
// 4.3.1] CO = $12.6 = 4B/C or 4Bai (see below) = 22+, A2s+, ATo+, KJ+, Q9s-QJs, J9s+, T9s ... 4B/F =KTo, K6s-K9s, Q8s, QJo 
// 4.3.1.1] TT+, AJs+ 4B55/C 100%, otherwise 30/70-70/30 depending on strength
// 4.3.2] B = $15.4  = 5B or C={33-AA, A3s+, A7o+, KQ+, KJs-KTs, QJs} 
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

//#define __MORE_STATS 

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// Player strategies:

///////////////////////////////////////////////////////////////////////////////////////////

struct Strategy_SB {
		
	int nHandsInRange;

	enum { SB_FOLD = 0, SB_4BET55_CALL, SB_4BET55_FOLD, SB_4BETAI, SB_MAX };
	
	double dOverallAction; // (actually only use for EV)

	double *(dAction[SB_MAX]);
	double dActionOverRange[SB_MAX];

	Strategy_SB(int nHands)
	{
		this->nHandsInRange = nHands;
		for (int i = 0; i < SB_MAX; ++i) {
			dAction[i] = new double [ nHands ];
		}
	}
	~Strategy_SB()
	{
		for (int i = 0; i < SB_MAX; ++i) {
			delete [] dAction[i];
		}		
	}
};

///////////////////////////////////////////////////////////////////////////////////////////

static void displayStrategy(Strategy_SB *pP_SB, Strategy_SB *pEV_SB, std::deque<std::list<CCard::Hole> >& llHandRange)
{
	//TODO: all sorts of extra information needed
	int nH = pP_SB->nHandsInRange;

	printf("SB EV over range: $%6.1f\n", pEV_SB->dOverallAction);
	
	double dCumProb[1 + Strategy_SB::SB_MAX] = { 0.0, 0.0, 0.0, 0.0, 0.0 };
	double dCumEv[Strategy_SB::SB_MAX] = { 0.0, 0.0, 0.0, 0.0 };
	
	for (int i = 0; i < nH; ++i) {
	
		double dFreq = llHandRange[i].begin()->dFreq;
		dCumProb[Strategy_SB::SB_FOLD] += pP_SB->dAction[Strategy_SB::SB_FOLD][i]*dFreq;
		dCumProb[Strategy_SB::SB_4BET55_CALL] += pP_SB->dAction[Strategy_SB::SB_4BET55_CALL][i]*dFreq;
		dCumEv[Strategy_SB::SB_4BET55_CALL] += pEV_SB->dAction[Strategy_SB::SB_4BET55_CALL][i]*pP_SB->dAction[Strategy_SB::SB_4BET55_CALL][i]*dFreq;
		dCumProb[Strategy_SB::SB_4BET55_FOLD] += pP_SB->dAction[Strategy_SB::SB_4BET55_FOLD][i]*dFreq;
		dCumEv[Strategy_SB::SB_4BET55_FOLD] += pEV_SB->dAction[Strategy_SB::SB_4BET55_FOLD][i]*pP_SB->dAction[Strategy_SB::SB_4BET55_FOLD][i]*dFreq;
		dCumProb[Strategy_SB::SB_4BETAI] += pP_SB->dAction[Strategy_SB::SB_4BETAI][i]*dFreq;
		dCumEv[Strategy_SB::SB_4BETAI] += pEV_SB->dAction[Strategy_SB::SB_4BETAI][i]*pP_SB->dAction[Strategy_SB::SB_4BETAI][i]*dFreq;
		dCumProb[Strategy_SB::SB_MAX] += dFreq;
		
		printf("%s %s (%5.2f): F= %3.0f%% ($   0.0) 4B55C= %3.0f%% ($%6.1f) 4B55F= %3.0f%% ($%6.1f) 4BAIN= %3.0f%% ($%6.1f)\n",
				llHandRange[i].begin()->first.s().c_str(), llHandRange[i].begin()->second.s().c_str(), dFreq,
				pP_SB->dAction[Strategy_SB::SB_FOLD][i]*100.0,
				pP_SB->dAction[Strategy_SB::SB_4BET55_CALL][i]*100.0, pEV_SB->dAction[Strategy_SB::SB_4BET55_CALL][i],
				pP_SB->dAction[Strategy_SB::SB_4BET55_FOLD][i]*100.0, pEV_SB->dAction[Strategy_SB::SB_4BET55_FOLD][i],
				pP_SB->dAction[Strategy_SB::SB_4BETAI][i]*100.0, pEV_SB->dAction[Strategy_SB::SB_4BETAI][i]
				);
		
	} // end loop over hands
	
	double dCumFreqInv = 1.0/dCumProb[Strategy_SB::SB_MAX];
	for (int j = 0; j < Strategy_SB::SB_MAX; ++j) {
		dCumEv[j] /= dCumProb[j];
		dCumProb[j] *= dCumFreqInv; 
	}	
	printf("             : F= %3.0f%% ($   0.0) 4B55C= %3.0f%% ($%6.1f) 4B55F= %3.0f%% ($%6.1f) 4BAIN= %3.0f%% ($%6.1f)\n",
		dCumProb[Strategy_SB::SB_FOLD]*100.0,
		dCumProb[Strategy_SB::SB_4BET55_CALL]*100.0, dCumEv[Strategy_SB::SB_4BET55_CALL],
		dCumProb[Strategy_SB::SB_4BET55_FOLD]*100.0, dCumEv[Strategy_SB::SB_4BET55_FOLD],
		dCumProb[Strategy_SB::SB_4BETAI]*100.0, dCumEv[Strategy_SB::SB_4BETAI]);	
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

struct Strategy_BB {
	
	int nHandsInRange;

	enum { BB_SB4BET55_FOLD = 0, BB_SB4BET55_5BETAI, BB_SB4BET55_MAX };
	enum { BB_SB4BETAI_FOLD = 0, BB_SB4BETAI_CALL, BB_SB4BETAI_MAX };

	double dOverallAction; // (actually only use for EV)
	
	double *(dAction_sb4bet55[BB_SB4BET55_MAX]);
	double dActionOverRange_sb4bet55[BB_SB4BET55_MAX];

#ifdef __MORE_STATS
	double *dCalled_5b;
#endif //__MORE_STATS
	
	double *(dAction_sb4betAi[BB_SB4BETAI_MAX]);
	double dActionOverRange_sb4betAi[BB_SB4BETAI_MAX];

	Strategy_BB(int nHands)
	{
		this->nHandsInRange = nHands;
		for (int i = 0; i < BB_SB4BET55_MAX; ++i) {
			dAction_sb4bet55[i] = new double [ nHands ];
		}
		for (int i = 0; i < BB_SB4BETAI_MAX; ++i) {
			dAction_sb4betAi[i] = new double [ nHands ];
		}
#ifdef __MORE_STATS
		dCalled_5b = new double [ nHands ];
#endif //__MORE_STATS
	}
	~Strategy_BB()
	{
		for (int i = 0; i < BB_SB4BET55_MAX; ++i) {
			delete [] dAction_sb4bet55[i];
		}
		for (int i = 0; i < BB_SB4BETAI_MAX; ++i) {
			delete [] dAction_sb4betAi[i];
		}
#ifdef __MORE_STATS
		delete [] dCalled_5b;
#endif //__MORE_STATS
	}
};

///////////////////////////////////////////////////////////////////////////////////////////

static void displayStrategy(Strategy_BB *pP_BB, Strategy_BB *pEV_BB, std::deque<std::list<CCard::Hole> >& llHandRange)
{
	int nH = pP_BB->nHandsInRange;
	
	printf("BB EV over range: $%6.1f\n", pEV_BB->dOverallAction);
	for (int i = 0; i < nH; ++i) {
	
#ifdef __MORE_STATS
		printf("%s %s (%5.2f): vs 4B55: 5B= %3.0f%% ($%6.1f) [Pcall= %3.0f%% EVcall= $%6.1f] vs 4BAI: C= %3.0f%% ($%6.1f)\n",
				llHandRange[i].begin()->first.s().c_str(), llHandRange[i].begin()->second.s().c_str(), llHandRange[i].begin()->dFreq,
				pP_BB->dAction_sb4bet55[Strategy_BB::BB_SB4BET55_5BETAI][i]*100.0, pEV_BB->dAction_sb4bet55[Strategy_BB::BB_SB4BET55_5BETAI][i],
				pP_BB->dCalled_5b[i]*100.0, pEV_BB->dCalled_5b[i], 
				pP_BB->dAction_sb4betAi[Strategy_BB::BB_SB4BETAI_CALL][i]*100.0, pEV_BB->dAction_sb4betAi[Strategy_BB::BB_SB4BETAI_CALL][i]
				);
#else // (normal case)
		printf("%s %s (%5.2f): vs 4B55: 5B= %3.0f%% ($%6.1f) vs 4BAI: C= %3.0f%% ($%6.1f)\n",
				llHandRange[i].begin()->first.s().c_str(), llHandRange[i].begin()->second.s().c_str(), llHandRange[i].begin()->dFreq,
				pP_BB->dAction_sb4bet55[Strategy_BB::BB_SB4BET55_5BETAI][i]*100.0, pEV_BB->dAction_sb4bet55[Strategy_BB::BB_SB4BET55_5BETAI][i],
				pP_BB->dAction_sb4betAi[Strategy_BB::BB_SB4BETAI_CALL][i]*100.0, pEV_BB->dAction_sb4betAi[Strategy_BB::BB_SB4BETAI_CALL][i]
				);
#endif //__MORE_STATS
		
	} // end loop over hands	
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// Some useful mini-structures:

struct Equity {
	double dFreq;
	double dEqFreq;
};

static double dMaxPotSize_ = 400.0;

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

static std::deque<std::string> _lsSmallBlindRange;
static std::deque<std::string> _lsBigBlindRange;

static void getCompressedHandRange(const std::deque<CCard::Hole>& lHandRange, 
									std::deque<std::list<CCard::Hole> >& llHandRange,
									std::deque<std::string>& lsHandRange)
{
	// (We won't do anything too clever at the moment, wait and see how long it takes!)
	
	std::deque<CCard::Hole>::const_iterator it;
	
	int nLastRank1 = -1, nLastRank2 = -1;
	int nLastSuited = -1;
	
	std::list<CCard::Hole>* pl = 0;
	
	for (it = lHandRange.begin(); it != lHandRange.end(); ++it) {
		
		int nSuited = int(it->first.getSuit() == it->second.getSuit());
		if ((it->first.getRank() == nLastRank1) && (it->second.getRank() == nLastRank2) && (nSuited == nLastSuited))
		{
///printf("%s %s, ", it->first.s().c_str(), it->second.s().c_str());	
			
			// (variation on an existing hand)
			pl->push_back(*it);			
		}
		else { // New hand
//printf("\n\nNEW %s %s\n", it->first.s().c_str(), it->second.s().c_str());
			
			llHandRange.resize(llHandRange.size() + 1);
			pl = &(*llHandRange.rbegin());
			
			pl->push_back(*it);
			
			nLastRank1 = it->first.getRank();
			nLastRank2 = it->second.getRank();
			nLastSuited = nSuited;
			
			lsHandRange.push_back(it->first.s() + it->second.s());
		}
		
	} // (end loop over hand range)
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

static void calculateEquityTable(Equity *pEastVsWestEquityTable,
		const std::deque<std::list<CCard::Hole> >& llEastHandRange, 
		const std::deque<std::list<CCard::Hole> >& llWestHandRange,
		std::multimap<double, int>& mmSortedEastEquity)
{
	//TODO: actually just pre-calculate (52x26)^2x2B ~= 4MB and save from disk

	int err;	
	StdDeck_CardMask pockets[2], dead, board;
	StdDeck_CardMask_RESET(board);
	StdDeck_CardMask_RESET(pockets[1]);
	StdDeck_CardMask_RESET(dead);
	enum_result_t result;

	std::deque<std::list<CCard::Hole> >::const_iterator itE, itW;
	
	Equity* pEq = pEastVsWestEquityTable;
	int nE = 0;
	for (itE = llEastHandRange.begin(); itE != llEastHandRange.end(); ++itE, ++nE) {
		
		double dOverallFreq = 0.0, dOverallFreqEq = 0.0;
		
		for (itW = llWestHandRange.begin(); itW != llWestHandRange.end(); ++itW) {
			
			std::list<CCard::Hole>::const_iterator litE, litW;
			const std::list<CCard::Hole>* plE = &(*itE);
			const std::list<CCard::Hole>* plW = &(*itW);
			
			double dFreq = 0.0, dFreqEq = 0.0;
			double dCachedFreq = 0.0, dCachedFreqEq = 0.0;				
						
			for (litE = plE->begin(); litE != plE->end(); ++litE) {
				
				// (Initialize equity calculator)
				StdDeck_CardMask_RESET(pockets[0]);
				StdDeck_CardMask_RESET(dead);
				
				StdDeck_CardMask_SET(pockets[0], litE->first.convertToPokerEnum());
				StdDeck_CardMask_SET(pockets[0], litE->second.convertToPokerEnum());				
				StdDeck_CardMask_SET(dead, litE->first.convertToPokerEnum());
				StdDeck_CardMask_SET(dead, litE->second.convertToPokerEnum());

				if (litE == plE->begin())
				{
					for (litW = plW->begin(); litW != plW->end(); ++litW) {
						if (!StdDeck_CardMask_CARD_IS_SET(dead, litW->first.convertToPokerEnum()) &&
								!StdDeck_CardMask_CARD_IS_SET(dead, litW->second.convertToPokerEnum()))
						{
							StdDeck_CardMask_SET(pockets[1], litW->first.convertToPokerEnum());
							StdDeck_CardMask_SET(pockets[1], litW->second.convertToPokerEnum());
							StdDeck_CardMask_SET(dead, litW->first.convertToPokerEnum());
							StdDeck_CardMask_SET(dead, litW->second.convertToPokerEnum());						
													
						    //err = enumExhaustive(game_holdem, pockets, board, dead, 2, 0, 0, &result);
							err = enumSample(game_holdem, pockets, board, dead, 2, 0, 100, 0, &result); // 1002 v 178 == 7180 cycles @ 100 samples

							if (!err) {
								double dFreq_el = (litE->dFreq)*(litW->dFreq);
								dCachedFreq += dFreq_el;
						    	double dEq = result.ev[0]/result.nsamples;
						    	dCachedFreqEq += dEq*dFreq_el;
//printf("%s%s v %s%s equity: %.2f\n", litE->first.s().c_str(), litE->second.s().c_str(), litW->first.s().c_str(), litW->second.s().c_str(), result.ev[0]/result.nsamples*100.0);		
							}
						    
							StdDeck_CardMask_RESET(pockets[1]);
							StdDeck_CardMask_UNSET(dead, litW->first.convertToPokerEnum());
							StdDeck_CardMask_UNSET(dead, litW->second.convertToPokerEnum());						
						}						
						
					} // end loop over W hands
					
//printf("----- %s %s equity: %.2f\n", litE->first.s().c_str(), litE->second.s().c_str(), dCachedFreq/dCachedFreqEq*100.0);		
					dFreq += dCachedFreq;
					dFreqEq += dCachedFreqEq;
				}
				else { // All the suited-ness is "symmetrical", so only need to calculate the equity once per "hand group"
					dFreq += dCachedFreq;
					dFreqEq += dCachedFreqEq;					
				}
								
			} // end loop over E hands

			pEq->dEqFreq = dFreqEq;
			pEq->dFreq = dFreq;			
			pEq++;
			
			dOverallFreq += dFreq;
			dOverallFreqEq += dFreqEq;
			
//printf("***** %s %s equity: %.2f\n", itE->begin()->first.s().c_str(), itE->begin()->second.s().c_str(), dFreqEq/dFreq*100.0);		

		} // end loop over west hand range
		
		mmSortedEastEquity.insert(std::pair<double, int>(dOverallFreqEq/dOverallFreq, nE));		

//printf("+++++ %s %s equity: %.2f\n", itE->begin()->first.s().c_str(), itE->begin()->second.s().c_str(), dOverallFreqEq/dOverallFreq*100.0);		
		
	} // end loop over east hand range	
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

static void calculateInitialSmallBlindStrategy(Strategy_SB* pP_SB, const std::multimap<double, int>& mmSortedEquity)
{
	// Plan will be just to 4-bet/call with top 25% of hands, 4-bet/fold with bottom 15% of hands
	// (jam 10%, normal 4-bet 90%)
	// Fold with the middle range

	int nHands = mmSortedEquity.size();
	int nBluffHands = (15*nHands)/100;
	int nValueHands = (25*nHands)/100;
	
	std::multimap<double, int>::const_iterator it;
	int n = 0;
	for (it = mmSortedEquity.begin(); it != mmSortedEquity.end(); ++it, ++n) {
		if (n <= nBluffHands) {
			pP_SB->dAction[Strategy_SB::SB_FOLD][n] = 0.0;
			pP_SB->dAction[Strategy_SB::SB_4BET55_CALL][n] = 0.0;
			pP_SB->dAction[Strategy_SB::SB_4BET55_FOLD][n] = 0.9;
			pP_SB->dAction[Strategy_SB::SB_4BETAI][n] = 0.1;
		}
		else if (n >= (nHands - nValueHands)) {
			pP_SB->dAction[Strategy_SB::SB_FOLD][n] = 0.0;
			pP_SB->dAction[Strategy_SB::SB_4BET55_CALL][n] = 0.9;
			pP_SB->dAction[Strategy_SB::SB_4BET55_FOLD][n] = 0.0;
			pP_SB->dAction[Strategy_SB::SB_4BETAI][n] = 0.1;			
		}
		else {
			pP_SB->dAction[Strategy_SB::SB_FOLD][n] = 1.0;
			pP_SB->dAction[Strategy_SB::SB_4BET55_CALL][n] = 0.0;
			pP_SB->dAction[Strategy_SB::SB_4BET55_FOLD][n] = 0.0;
			pP_SB->dAction[Strategy_SB::SB_4BETAI][n] = 0.0;			
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// Pot size: 3 (1/2 blinds) -> 8 (6 raise) -> 28 (22 3-bet, +20) -> 77 (55 4-bet, +49) -> 255 (200 5-bet-ai, +178)
//                                                               -> 222 (200 4-bet ai, +194)

static double dEV_SB_3b_fold_ = 28.0;
static double dEV_SB_4b_fold_ = -49.0; // (already have 6 invested)
static double dSB_4b_callOrPush_ = 194.0;

///////////////////////////////////////////////////////////////////////////////////////////

static double calculateSmallBlindStrategy(Strategy_SB* pP_SB, Strategy_SB* pEV_SB, Strategy_BB* pP_BB, 
											Equity* pEquityTable, double dAlpha)
{
	int nHands_SB = pP_SB->nHandsInRange;
	int nHands_BB = pP_BB->nHandsInRange;
	double dNegAlpha = 1.0 - dAlpha;

	double dFreqOverRange = 0.0, dEvOverRange = 0.0;
		
	for (int nSB = 0; nSB < nHands_SB; ++nSB) {

		// Half the alpha filter
		pP_SB->dAction[Strategy_SB::SB_FOLD][nSB] *= dNegAlpha;
		pP_SB->dAction[Strategy_SB::SB_4BET55_CALL][nSB] *= dNegAlpha;
		pP_SB->dAction[Strategy_SB::SB_4BET55_FOLD][nSB] *= dNegAlpha;
		pP_SB->dAction[Strategy_SB::SB_4BETAI][nSB] *= dNegAlpha;

		double dEV_4b55_fold = 0.0, dEV_4b55_call = 0.0, dEV_4bai = 0.0;
		double dTotalFreq = 0.0;
		for (int nBB = 0; nBB < nHands_BB; ++nBB) {

			Equity* pEq = &pEquityTable[nSB*nHands_BB + nBB];
			double dFreqEq = pEq->dEqFreq;
			double dFreq = pEq->dFreq;
			dTotalFreq += dFreq; 

			// If BB folds to 4B, both 4B/f and 4B/c get the same +EV
			double dEV1 = pP_BB->dAction_sb4bet55[Strategy_BB::BB_SB4BET55_FOLD][nBB]*dFreq*dEV_SB_3b_fold_;
			dEV_4b55_fold += dEV1;
			dEV_4b55_call += dEV1;
					
			// If I'm 4b/f, and he 5b-pushes, I just lose my 4B
			double dP2 = pP_BB->dAction_sb4bet55[Strategy_BB::BB_SB4BET55_5BETAI][nBB];
			dEV_4b55_fold += dP2*dEV_SB_4b_fold_*dFreq;

			// If I'm 4b/c, and he 5b-pushes, I gain my equity
			double dEV2 = dMaxPotSize_*dFreqEq - dSB_4b_callOrPush_*dFreq;
			dEV_4b55_call += dP2*dEV2;
				// (I gain pot equity minus my shove amount, minus the PFR I already made)

			// Finally, do a similar exercise for 4bai (except I can't fold)
			double dP3 = pP_BB->dAction_sb4betAi[Strategy_BB::BB_SB4BETAI_FOLD][nBB];
			dEV_4bai += dP3*dFreq*dEV_SB_3b_fold_;
			dP3 = 1.0 - dP3; // (since BB must either call or fold)
			dEV_4bai += dP3*dEV2; 
				// (as above, I gain pot equity minus my shove amount, minus the PFR I already made)
			
#if 0
//if ((nSB == 9)&&(nBB==22))  			
//printf("SB: %s v %s: %.1f [%.1f]\n", _lsSmallBlindRange[nSB].c_str(), _lsBigBlindRange[nBB].c_str(), dFreqEq/dFreq*100.0, dFreq);
//printf("\t\t\t\tP2 = %.1f%% P3 = %.1f%% EV1 = $%.1f EV2 = $%.1f\n", dP2*100.0, dP3*100.0, dEV1/dFreq, dEV2/dFreq);
#endif	
		} // end loop over BB hands, vs 4-bet to 55/ai

		// Work out the best option (including the fold):

		// Action probabilities:
		if ((dEV_4b55_fold > dEV_4b55_call) && (dEV_4b55_fold > dEV_4bai)) {
			if (dEV_4b55_fold > 0.0) {
				pP_SB->dAction[Strategy_SB::SB_4BET55_FOLD][nSB] += dAlpha;
					//  (can be better than folding to 3b, eg if BB folds everything but AA to 4b!)
				dEvOverRange += dEV_4b55_fold; // (already xdTotalFreq)
			}
			else { // Just fold
				pP_SB->dAction[Strategy_SB::SB_FOLD][nSB] += dAlpha;								
			}
		}
		else if ((dEV_4b55_call > dEV_4b55_fold) && (dEV_4b55_call > dEV_4bai)) {
			if (dEV_4b55_call > 0.0) {
				pP_SB->dAction[Strategy_SB::SB_4BET55_CALL][nSB] += dAlpha;
				dEvOverRange += dEV_4b55_call; // (already xdTotalFreq)
			}
			else { // Just fold
				pP_SB->dAction[Strategy_SB::SB_FOLD][nSB] += dAlpha;								
			}			
		}
		else if (dEV_4bai > 0.0) {
			pP_SB->dAction[Strategy_SB::SB_4BETAI][nSB] += dAlpha;			
			dEvOverRange += dEV_4bai; // (already xdTotalFreq)
		}
		else { // just fold
			pP_SB->dAction[Strategy_SB::SB_FOLD][nSB] += dAlpha;											
		}
		dFreqOverRange += dTotalFreq;

		// Values, including normalization:
		dTotalFreq = 1.0/dTotalFreq;
		pEV_SB->dAction[Strategy_SB::SB_FOLD][nSB] = 0.0;
		pEV_SB->dAction[Strategy_SB::SB_4BET55_CALL][nSB] *= dNegAlpha;
		pEV_SB->dAction[Strategy_SB::SB_4BET55_CALL][nSB] += dAlpha*dEV_4b55_call*dTotalFreq;
		pEV_SB->dAction[Strategy_SB::SB_4BET55_FOLD][nSB] *= dNegAlpha;
		pEV_SB->dAction[Strategy_SB::SB_4BET55_FOLD][nSB] += dAlpha*dEV_4b55_fold*dTotalFreq;
		pEV_SB->dAction[Strategy_SB::SB_4BETAI][nSB] *= dNegAlpha;			
		pEV_SB->dAction[Strategy_SB::SB_4BETAI][nSB] += dAlpha*dEV_4bai*dTotalFreq;			

	} // end loop over SB hands
		
	pEV_SB->dOverallAction = dNegAlpha*pEV_SB->dOverallAction + dAlpha*(dEvOverRange/dFreqOverRange);
	
	//TODO: values, actions over ranges
	return -1.0;
}

///////////////////////////////////////////////////////////////////////////////////////////

// Pot size: 3 (1/2 blinds) -> 8 (6 raise) -> 28 (22 3-bet) -> 77 (55 4-bet) -> 255 (200 5-bet-ai)
//                                                          -> 222 (200 4-bet ai)

static double dEV_SB_fold_ = 28;
static double dEV_BB_4b_fold_ = 77;
static double dBB_4bai_call_ = 178.0;

///////////////////////////////////////////////////////////////////////////////////////////

static double calculateBigBlindStrategy(Strategy_BB* pP_BB, Strategy_BB* pEV_BB, Strategy_SB* pP_SB, 
											Equity* pEquityTable, double dAlpha)
{
	int nHands_BB = pP_BB->nHandsInRange;
	int nHands_SB = pP_SB->nHandsInRange;
	
	double dNegAlpha = 1.0 - dAlpha;

	double dFreqOverRange = 0.0, dEvOverRange = 0.0;
		
	for (int nBB = 0; nBB < nHands_BB; ++nBB) {

		// Half the alpha filter
		pP_BB->dAction_sb4bet55[Strategy_BB::BB_SB4BET55_FOLD][nBB] *= dNegAlpha;
		pP_BB->dAction_sb4bet55[Strategy_BB::BB_SB4BET55_5BETAI][nBB] *= dNegAlpha;
		pP_BB->dAction_sb4betAi[Strategy_BB::BB_SB4BETAI_FOLD][nBB] *= dNegAlpha;
		pP_BB->dAction_sb4betAi[Strategy_BB::BB_SB4BETAI_CALL][nBB] *= dNegAlpha;
		
		double dEV_4b55_5b = 0.0, dEV_4bai_call = 0.0;
		double dP_4b55 = 0.0, dP_4bai = 0.0, dP_f = 0.0;
		
#ifdef __MORE_STATS
		pP_BB->dCalled_5b[nBB] = 0.0;
		pEV_BB->dCalled_5b[nBB] = 0.0;
#endif //__MORE_STATS
		
		for (int nSB = 0; nSB < nHands_SB; ++nSB) {
		
			Equity* pEq = &pEquityTable[nBB*nHands_SB + nSB];
			double dFreqEq = pEq->dEqFreq;
			double dFreq = pEq->dFreq;

//printf("BB: %s v %s: %.1f [%.1f]\n", _lsBigBlindRange[nBB].c_str(), _lsSmallBlindRange[nSB].c_str(), dFreqEq/dFreq*100.0, dFreq);			
			
			double dP1 = pP_SB->dAction[Strategy_SB::SB_4BET55_FOLD][nSB];
			double dP2 = pP_SB->dAction[Strategy_SB::SB_4BET55_CALL][nSB];

			dP_4b55 += (dP1 + dP2)*dFreq; // (P1+P2)*dF=P(4B55|nSB)P(nSB), ie summed up = P(4B55)
			//TODO: I've not quite convinced myself I haven't made a mistake in calc'ing the conditional probs here
			
			dEV_4b55_5b += dP1*dFreq*dEV_BB_4b_fold_;			
			dEV_4b55_5b += dP2*(dMaxPotSize_*dFreqEq - dBB_4bai_call_*dFreq);
				// 5B my stack ai (ie stack - 3-bet)
			
			dP1 = pP_SB->dAction[Strategy_SB::SB_4BETAI][nSB];
			dP_4bai += dP1*dFreq;
			dEV_4bai_call += dP1*(dMaxPotSize_*dFreqEq - dBB_4bai_call_*dFreq);
				// (P1*dF times, calling dBB_4bai_call_ to win dMaxPotSize_ with dFreqEq/dFreq pot equity) 
			
#ifdef __MORE_STATS
			pP_BB->dCalled_5b[nBB] += dFreq*dP2;
			pEV_BB->dCalled_5b[nBB] += (dP1 + dP2)*(dMaxPotSize_*dFreqEq - dBB_4bai_call_*dFreq);
#endif //__MORE_STATS
			
			dP_f += pP_SB->dAction[Strategy_SB::SB_FOLD][nSB]*dFreq;

		} // end loop over SB hands

#ifdef __MORE_STATS
		pP_BB->dCalled_5b[nBB] /= dP_4b55;
		pEV_BB->dCalled_5b[nBB] /= dP_4b55;
#endif //__MORE_STATS

		dFreqOverRange += dP_4b55 + dP_4bai + dP_f;
		
		// Action probabilities:
		
		if (dEV_4b55_5b > 0.0) { // ie better than folding		
			pP_BB->dAction_sb4bet55[Strategy_BB::BB_SB4BET55_5BETAI][nBB] += dAlpha;
			dEvOverRange += dEV_4b55_5b;
		}
		else { // else fold
			pP_BB->dAction_sb4bet55[Strategy_BB::BB_SB4BET55_FOLD][nBB] += dAlpha;			
		}
		if (dEV_4bai_call > 0.0) { // ie better than folding		
			pP_BB->dAction_sb4betAi[Strategy_BB::BB_SB4BETAI_CALL][nBB] += dAlpha;
			dEvOverRange += dEV_4bai_call;
		}
		else { // else fold
			pP_BB->dAction_sb4betAi[Strategy_BB::BB_SB4BETAI_FOLD][nBB] += dAlpha;			
		}
		dEvOverRange += dP_f*dEV_SB_fold_;
		
		// Some value normalizations/calculations:
		pEV_BB->dAction_sb4bet55[Strategy_BB::BB_SB4BET55_FOLD][nBB] = 0.0;
		pEV_BB->dAction_sb4bet55[Strategy_BB::BB_SB4BET55_5BETAI][nBB] *= dNegAlpha;
		pEV_BB->dAction_sb4betAi[Strategy_BB::BB_SB4BETAI_FOLD][nBB] = 0.0;
		pEV_BB->dAction_sb4betAi[Strategy_BB::BB_SB4BETAI_CALL][nBB] *= dNegAlpha;

		if (dP_4b55 > 0.0) {
			dEV_4b55_5b /= dP_4b55;
			pEV_BB->dAction_sb4bet55[Strategy_BB::BB_SB4BET55_5BETAI][nBB] += dAlpha*dEV_4b55_5b;
		}
		
		if (dP_4bai > 0.0) {
			dEV_4bai_call /= dP_4bai;
			pEV_BB->dAction_sb4betAi[Strategy_BB::BB_SB4BETAI_CALL][nBB] += dAlpha*dEV_4bai_call;
		}

	} // end loop over BB hands
	
	pEV_BB->dOverallAction = dNegAlpha*pEV_BB->dOverallAction + dAlpha*(dEvOverRange/dFreqOverRange);
	
	//TODO: values, actions over ranges (don't forget to take SB-fold-to-3B
	return -1.0;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// Main loop for GTO solution for scenario described above

// Hero = SB (open / 4-bet)
// Villain = BB (3-bet / 5-bet)

void GTO_demo_preflop(std::deque<CCard::Hole>& lSmallBlindRange, std::deque<CCard::Hole>& lBigBlindRange)
{
	static const int nMaxIts_ = 10000;
	
	time_t nCpu;

// 1. Create compressed subset of hands (associated with "compressed-out" hands, still needed for equity)
	
	nCpu = ::clock();
	
	std::deque<std::list<CCard::Hole> > llSmallBlindRange, llBigBlindRange;

	::getCompressedHandRange(lSmallBlindRange, llSmallBlindRange, _lsSmallBlindRange);
	::getCompressedHandRange(lBigBlindRange, llBigBlindRange, _lsBigBlindRange);
	
	printf("Compressed hand ranges: %d cycles\n", (int)(::clock() - nCpu));	
	
	int nSB = llSmallBlindRange.size();
	int nBB = llBigBlindRange.size();

	printf("SB: %d -> %d / BB: %d -> %d\n", lSmallBlindRange.size(), nSB, lBigBlindRange.size(), nBB);
	
// 2. Create equity table

	nCpu = ::clock();
	
	// 2.1 Allocate equity tables (now we know hand range sizes)
	
	//TBD card replacement?
	int nCombos = nSB*nBB;
	Equity *pSmallVsBigEquityTable = new Equity [ nCombos ];
	Equity *pBigVsSmallEquityTable = new Equity [ nCombos ];
	
	// 2.2 Calculate equity
	std::multimap<double, int> mmSmallBlindSortedEquity;
	::calculateEquityTable(pSmallVsBigEquityTable, llSmallBlindRange, llBigBlindRange, mmSmallBlindSortedEquity);
	
	// (We'll just invert the table rather than waste all that time using pokerenum a second time)
	Equity *pEq = pBigVsSmallEquityTable;
	for (int i = 0; i < nBB; i++) {
		for (int j = 0; j < nSB; j++) {
			Equity *pOtherEq = &pSmallVsBigEquityTable[j*nBB + i];
			double dFreq = pOtherEq->dFreq; 
			pEq->dFreq = dFreq;
			pEq->dEqFreq = dFreq - pOtherEq->dEqFreq; 
			++pEq;
		}
	}
	
	printf("Created equity tables: %d cycles\n", (int)(::clock() - nCpu));	

// 3. Create initial strategies

	nCpu = ::clock();
	
	// 3.1 Allocate strategy memory (now we know hand sizes)
	
	Strategy_SB *pStrat_SB = new Strategy_SB(nSB); 
	Strategy_SB *pEV_SB = new Strategy_SB(nSB); 
	Strategy_BB *pStrat_BB = new Strategy_BB(nBB); 
	Strategy_BB *pEV_BB = new Strategy_BB(nBB); 
	
	// 3.2 Calculate initial strategies

	::calculateInitialSmallBlindStrategy(pStrat_SB, mmSmallBlindSortedEquity);
	printf("Created intial strategies: %d cycles\n", (int)(::clock() - nCpu));		
	
// 4. Loop over iterations

	nCpu = ::clock();	
	
	int nIts = 0;
	
	for (nIts = 0; nIts <= nMaxIts_; ++nIts) {

		double dAlpha = 1.0/(1 + nIts);

		//printf(">>>>>>>>>>>>>Iteration %d\n", nIts);
		::calculateBigBlindStrategy(pStrat_BB, pEV_BB, pStrat_SB, pBigVsSmallEquityTable, dAlpha);
		::calculateSmallBlindStrategy(pStrat_SB, pEV_SB, pStrat_BB, pSmallVsBigEquityTable, dAlpha);
		
	} // (end loop over iterations)
	
	printf("Calculated co-optimal strategies: %d cycles\n", (int)(::clock() - nCpu));		

// 5. End game	
	
	// Display results:

	printf("Small Blind results:\n");
	::displayStrategy(pStrat_SB, pEV_SB, llSmallBlindRange);
	printf("\n\n");
	printf("Big Blind results:\n");
	::displayStrategy(pStrat_BB, pEV_BB, llBigBlindRange);
	printf("\n\n");
	
	// Tidy up:
	delete pStrat_SB;
	delete pEV_SB;
	delete pStrat_BB;
	delete pEV_BB;
	delete [] pBigVsSmallEquityTable;
	delete [] pSmallVsBigEquityTable;
}
