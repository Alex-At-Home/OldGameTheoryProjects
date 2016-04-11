//______________________________________________________________________________________________
//
// ZeroOneGames
//______________________________________________________________________________________________
//
// Some GTO solutions to [0,1] games I'm inventing
//
// Alex Piggott July 2008
//______________________________________________________________________________________________

// Some control parameters:

/**/
//#define __SINGLE_STREET
//#define __THREE_STREET
#define __VIRTUAL_TWO_STREET
//#define __VIRTUAL_THREE_STREET
#define __SIMPLESTGAME // (P1 KC/KF/B only, no raising)
#define __SIMPLESTGAME_NOLEADING // (in fact P1 KC/KF only)
	//(^ only has an effect if __SIMPLESTGAME #defined) 
#define __ABSTRACT_DISPLAY // (doesn't try to map strengths to hand categories)
//#define __PERTURBED_GAME // (generates thresholded vs weakly-dominated mixed strategies - NB currently introduces minor inaccuracies) 
	//(^ also this doesn't seem to work with the multi-street version as the "cond prev action" element of the "path prob" is 0?)
//#define __LIMIT
//#define __LOG_CONVERGENCE
#define __SMOOTH_FP
//#define __CONTINUOUS_FP

#include <memory.h>
#include <stdio.h>
#include <time.h>
#include <float.h>
#include <math.h>

#ifdef __LOG_CONVERGENCE
	int _nConvIts = 0;
	static const int _nConvItsPeriodicity = 10; // (ie display every 10 iterations)
	FILE *_fpConvergenceLog = 0;
#endif //__LOG_CONVERGENCE

#ifdef  __SMOOTH_FP
	static const double _dEntropyWeight = 0.01; // (0 for no randomness, <= 1)
	static const double _dEntropyWeightInv = 1.0/_dEntropyWeight;
#endif //__SMOOTH_FP

#ifdef __CONTINUOUS_FP
	//TODO
#endif //__CONTINUOUS_FP
	
	
	
namespace GameActions {

	static const double dPot = 1.0;
	//	static const double dPot = 5.0; // (eg limit)
	//TODO: currently, changing the pot value doesn't actually work, eg the cost values will be wrong, need to apply...
	static const double dBetScalar = 1.0; // ... in various places...

	enum { ACTION_K = 0, ACTION_B, ACTION_NUMACTIONS };
	static const int ACTIONS_NUMACTIONS_ACT = ACTION_B; 
	
// Full version
	enum { P1STRAT_KF = 0, P1STRAT_KC, P1STRAT_KRF, P1STRAT_KRC, P1STRAT_BF, P1STRAT_BC, P1STRAT_BR, P1STRAT_NUMACTS };
	static const char *szP1STRAT[] = { "KF.", "KC.", "KRF", "KRC", "BF.", "BC.", "BR.", "..." };

#ifdef __SIMPLESTGAME

#ifdef __SIMPLESTGAME_NOLEADING 
	static const int P1STRAT_NUMACTS_ACT = P1STRAT_KRF;
	static const int P1STRAT_NUMACTS_B_INIT = P1STRAT_KRF;
	static const int P1STRAT_NUMACTS_B_ACT = P1STRAT_KRF;
#else	
	static const int P1STRAT_NUMACTS_ACT = 3; // KF, KC, or Bx
	static const int P1STRAT_NUMACTS_B_INIT = P1STRAT_BF;
	static const int P1STRAT_NUMACTS_B_ACT = P1STRAT_BC;
#endif	
	static const int P1STRAT_NUMACTS_K_INIT = P1STRAT_KF;
	static const int P1STRAT_NUMACTS_K_ACT = P1STRAT_KRF;
#else //full game
	static const int P1STRAT_NUMACTS_ACT = P1STRAT_NUMACTS;
	static const int P1STRAT_NUMACTS_K_INIT = P1STRAT_KF;
	static const int P1STRAT_NUMACTS_K_ACT = P1STRAT_BF;
	static const int P1STRAT_NUMACTS_B_INIT = P1STRAT_BF;
	static const int P1STRAT_NUMACTS_B_ACT = P1STRAT_NUMACTS;
#endif	
	
	enum { P2STRAT_P1K_K = 0,  P2STRAT_P1K_BF, P2STRAT_P1K_BC,  P2STRAT_P1K_BR, P2STRAT_P1K_NUMACTS };
	static const char *szP2STRAT_P1K[] = { "(K)K.", "(K)BF", "(K)BC", "(K)BR", "....." };
	enum { P2STRAT_P1B_F = 0, P2STRAT_P1B_C, P2STRAT_P1B_RF, P2STRAT_P1B_RC, P2STRAT_P1B_NUMACTS };
	static const char *szP2STRAT_P1B[] = { "(B)F.", "(B)C.", "(B)RF", "(B)RC", "....." };

#ifdef __SIMPLESTGAME
	static const int P2STRAT_P1K_NUMACTS_ACT = P2STRAT_P1K_BC;
#ifdef __SIMPLESTGAME_NOLEADING 
	static const int P2STRAT_P1B_NUMACTS_ACT = P2STRAT_P1B_F;
#else
	static const int P2STRAT_P1B_NUMACTS_ACT = P2STRAT_P1B_RF;
#endif
	
#else //full game
	static const int P2STRAT_P1K_NUMACTS_ACT = P2STRAT_P1K_NUMACTS;
	static const int P2STRAT_P1B_NUMACTS_ACT = P2STRAT_P1B_NUMACTS;
#endif	
	
#ifdef __SIMPLESTGAME
	enum {  ACTIONS_KBF = 0, 			
#ifndef __SIMPLESTGAME_NOLEADING 
			ACTIONS_BF,
#endif		
			ACTIONS_KK, ACTIONS_KBC,
#ifndef __SIMPLESTGAME_NOLEADING 
			ACTIONS_BC,
#endif		
			ACTIONS_NUMACTS,
		// Not used: (but still needed so that all the actions compile) 
				ACTIONS_BRF = -1, ACTIONS_KBRRF = -1, 
#ifdef __SIMPLESTGAME_NOLEADING 
				ACTIONS_BF = -1, 
#endif		
				ACTIONS_BRRF = -1, ACTIONS_KBRF = -1,
				ACTIONS_KBRC = -1, ACTIONS_KBRRC = -1,
#ifdef __SIMPLESTGAME_NOLEADING 
				ACTIONS_BC = -1, 
#endif		
				ACTIONS_BRC = -1, ACTIONS_BRRC = -1
			 };
		// (ordering important in this enum)
#ifdef __SIMPLESTGAME_NOLEADING 
	static const char *szACTIONS[] = { "KBF..", "KK...", "KBC..", "....." };
	static const int ACTIONS_P2FOLD_ACT = -1; // (never folds)
#else
	static const char *szACTIONS[] = { "KBF..", "BF...", "KK...", "KBC..", "BC...", "....." };
	static const int ACTIONS_P2FOLD_ACT = ACTIONS_BF;
#endif		
	static const int ACTIONS_NUMACTS_ACT = ACTIONS_NUMACTS;
	static const int ACTIONS_P1FOLD_ACT = ACTIONS_KBF;
	
#else //full game
	enum {  ACTIONS_KBF = 0, ACTIONS_BRF, ACTIONS_KBRRF, 	// first line, all the P1 folds
			ACTIONS_BF, ACTIONS_BRRF, ACTIONS_KBRF, 		// second line, all the P2 folds
			ACTIONS_KK, ACTIONS_KBC, ACTIONS_KBRC, ACTIONS_KBRRC,
			ACTIONS_BC, ACTIONS_BRC, ACTIONS_BRRC, ACTIONS_NUMACTS };
		// (ordering important in this enum)
	static const char *szACTIONS[] = { "KBF..", "BRF..", "KBRRF", "BF...", "BRRF.", "KBRF.", "KK...", "KBC..", "KBRC.", "KBRRC", "BC...", "BRC..", "BRRC.", "....." };
	static const int ACTIONS_P1FOLD_ACT = ACTIONS_KBRRF;
	static const int ACTIONS_P2FOLD_ACT = ACTIONS_KBRF;
	static const int ACTIONS_NUMACTS_ACT = ACTIONS_NUMACTS;
#endif	

	int nActionTableP1_P1K[P1STRAT_BF][P2STRAT_P1K_NUMACTS] = {
		//P2: K   			, BF  			, BC  			, BR    
			{ ACTIONS_KK	, ACTIONS_KBF	, ACTIONS_KBF	, ACTIONS_KBF},		// P1STRAT_KF
			{ ACTIONS_KK	, ACTIONS_KBC	, ACTIONS_KBC	, ACTIONS_KBC},		// P1STRAT_KC
			{ ACTIONS_KK	, ACTIONS_KBRF	, ACTIONS_KBRC	, ACTIONS_KBRRF},	// P1STRAT_KRF
			{ ACTIONS_KK	, ACTIONS_KBRF	, ACTIONS_KBRC	, ACTIONS_KBRRC}	// P1STRAT_KRC
	};
	int nActionTableP1_P1B[P1STRAT_NUMACTS - P1STRAT_BF][P2STRAT_P1K_NUMACTS] = {
		//P2: F   			, C  			, RF  			, RC    
			{ ACTIONS_BF	, ACTIONS_BC	, ACTIONS_BRF	, ACTIONS_BRF},		// P1STRAT_BF
			{ ACTIONS_BF	, ACTIONS_BC	, ACTIONS_BRC	, ACTIONS_KBC},		// P1STRAT_BC
			{ ACTIONS_BF	, ACTIONS_BC	, ACTIONS_BRRF	, ACTIONS_BRRC}		// P1STRAT_BR
	};
	int nRevActionTable_P1[P1STRAT_NUMACTS][5] = { 
			{ ACTIONS_KK, ACTIONS_KBF, -1, 0, 0, }, 						//P1STRAT_KF
			{ ACTIONS_KK, ACTIONS_KBC, -1, 0, 0 },							//P1STRAT_KC
			{ ACTIONS_KK, ACTIONS_KBRF, ACTIONS_KBRC, ACTIONS_KBRRF, -1 }, 	//P1STRAT_KRF  
			{ ACTIONS_KK, ACTIONS_KBRF, ACTIONS_KBRC, ACTIONS_KBRRC, -1 }, 	//P1STRAT_KRC
			{ ACTIONS_BF, ACTIONS_BC, ACTIONS_BRF, -1, 0 },					//P1STRAT_BF 
			{ ACTIONS_BF, ACTIONS_BC, ACTIONS_BRC, -1, 0 },					//P1STRAT_BC 
			{ ACTIONS_BF, ACTIONS_BC, ACTIONS_BRRF, ACTIONS_BRRC, -1 }		//P1STRAT_BR 
	};
	int nRevActionTable_P1K_P2[P2STRAT_P1K_NUMACTS][5] = {
			{ ACTIONS_KK, -1, 0, 0, 0 }, 									// P2STRAT_P1K_K
			{ ACTIONS_KBF, ACTIONS_KBC, ACTIONS_KBRF, -1, 0 },				// P2STRAT_P1K_BF 
			{ ACTIONS_KBF, ACTIONS_KBC, ACTIONS_KBRC, -1, 0 },				// P2STRAT_P1K_BC 
			{ ACTIONS_KBF, ACTIONS_KBC, ACTIONS_KBRRF, ACTIONS_KBRRC, -1 }	// P2STRAT_P1K_BR 
	};
	int nRevActionTable_P1B_P2[P2STRAT_P1B_NUMACTS][5] = {
			{ ACTIONS_BF, -1, 0, 0, 0 }, 						// P2STRAT_P1B_F
			{ ACTIONS_BC, -1, 0, 0, 0 },						// P2STRAT_P1B_C 
			{ ACTIONS_BRF, ACTIONS_BRC, ACTIONS_BRRF, -1, 0 },	// P2STRAT_P1B_RF 
			{ ACTIONS_BRF, ACTIONS_BRC, ACTIONS_BRRC, -1, 0 }	// P2STRAT_P1B_RC 
	};	
	
#ifdef __LIMIT
	double dCostTableP1_P1K[P1STRAT_BF][P2STRAT_P1K_NUMACTS] = { 
		//P2: K   , BF  , BC  , BR    
			{ 00.0, 00.0, 00.0, 00.0},	// P1STRAT_KF
			{ 00.0, 01.0, 01.0, 01.0},	// P1STRAT_KC
			{ 00.0, 02.0, 02.0, 02.0},	// P1STRAT_KRF
			{ 00.0, 02.0, 02.0, 03.0}	// P1STRAT_KRC
	};
	double dCostTableP1_P1B[P1STRAT_NUMACTS - P1STRAT_BF][P2STRAT_P1B_NUMACTS] = { 
		//P2: F   , C   , RF  , RC    
			{ 01.0, 01.0, 01.0, 01.0},	// P1STRAT_BF
			{ 01.0, 01.0, 02.0, 02.0},	// P1STRAT_BC
			{ 01.0, 01.0, 03.0, 03.0}	// P1STRAT_BR
	};
	double dCostTableP2_P1K[P1STRAT_BF][P2STRAT_P1K_NUMACTS] = { 
		//P2: K   , BF  , BC  , BR    
			{ 00.0, 01.0, 01.0, 01.0},	// P1STRAT_KF
			{ 00.0, 01.0, 01.0, 01.0},	// P1STRAT_KC
			{ 00.0, 01.0, 02.0, 03.0},	// P1STRAT_KRF
			{ 00.0, 01.0, 02.0, 03.0}	// P1STRAT_KRC
	};
	double dCostTableP2_P1B[P1STRAT_NUMACTS - P1STRAT_BF][P2STRAT_P1B_NUMACTS] = { 
		//P2: F   , C   , RF  , RC    
			{ 00.0, 01.0, 02.0, 02.0},	// P1STRAT_BF
			{ 00.0, 01.0, 02.0, 02.0},	// P1STRAT_BC
			{ 00.0, 01.0, 02.0, 03.0}	// P1STRAT_BR
	};
#else
	double dCostTableP1_P1K[P1STRAT_BF][P2STRAT_P1K_NUMACTS] = { 
		//P2: K   , BF  , BC  , BR    
			{ 00.0, 00.0, 00.0, 00.0},	// P1STRAT_KF
			{ 00.0, 01.0, 01.0, 01.0},	// P1STRAT_KC
			{ 00.0, 04.0, 04.0, 04.0},	// P1STRAT_KRF
			{ 00.0, 04.0, 04.0, 13.0}	// P1STRAT_KRC
	};
	double dCostTableP1_P1B[P1STRAT_NUMACTS - P1STRAT_BF][P2STRAT_P1B_NUMACTS] = { 
		//P2: F   , C   , RF  , RC    
			{ 01.0, 01.0, 01.0, 01.0},	// P1STRAT_BF
			{ 01.0, 01.0, 04.0, 04.0},	// P1STRAT_BC
			{ 01.0, 01.0, 13.0, 13.0}	// P1STRAT_BR
	};
	double dCostTableP2_P1K[P1STRAT_BF][P2STRAT_P1K_NUMACTS] = { 
		//P2: K   , BF  , BC  , BR    
			{ 00.0, 01.0, 01.0, 01.0},	// P1STRAT_KF
			{ 00.0, 01.0, 01.0, 01.0},	// P1STRAT_KC
			{ 00.0, 01.0, 04.0, 13.0},	// P1STRAT_KRF
			{ 00.0, 01.0, 04.0, 13.0}	// P1STRAT_KRC
	};
	double dCostTableP2_P1B[P1STRAT_NUMACTS - P1STRAT_BF][P2STRAT_P1B_NUMACTS] = { 
		//P2: F   , C   , RF  , RC    
			{ 00.0, 01.0, 04.0, 04.0},	// P1STRAT_BF
			{ 00.0, 01.0, 04.0, 04.0},	// P1STRAT_BC
			{ 00.0, 01.0, 04.0, 13.0}	// P1STRAT_BR
	};
#endif //!LIMIT
//TODO: rotated/cache-aligned versions of these??	
//TODO: lots of optimization of all this, eg order it better to unroll the loops
	
}; // GameActions

//______________________________________________________________________________________________

namespace HandStrengths {

// The most common hand strengths	
	static const int n0P_START = 0;
	static const int nOP_STEPS = 6;
	static const int nOP_END = n0P_START + nOP_STEPS - 1;  
	static const int n1P_START = nOP_END + 1;
	static const int n1P_STEPS = 6;
	static const int n1P_END = n1P_START + n1P_STEPS - 1;
	
// Less common ones:		
//	static const int n2P_START = n1P_END + 1;
//	static const int n2P_STEPS = 3;
//	static const int n2P_END = n2P_START + n2P_STEPS - 1;  
//	static const int n3_START = n2P_END + 1;
//	static const int n3_STEPS = 3;
//	static const int n3_END = n3_START + n3_STEPS - 1;  
//	static const int nSTR8_START = n3_END + 1;
//	static const int nSTR8_STEPS = 3;
//	static const int nSTR8_END = nSTR8_START + nSTR8_STEPS - 1;  
//	static const int nFL_START = nSTR8_END + 1;
//	static const int nFL_STEPS = 3;
//	static const int nFL_END = nFL_START + nFL_STEPS - 1;  
//	static const int nFH_START = nFL_END + 1;
//	static const int nFH_STEPS = 3;
//	static const int nFH_END = nFH_START + nFH_STEPS - 1;  

// And a catch-all:
	static const int nNUTS = 12; 
/**/ // Hack for testing 3 street game	
#ifndef __VIRTUAL_THREE_STREET
	static const int nHANDS = 13;
#else	
	/**///NB doesn't work well with 10 for some reason (interpolates >1)
	static const int nHANDS = 13;
#endif	
#ifdef __ABSTRACT_DISPLAY
	static const char* szHandStrengths[nHANDS + 10/**/] = {
			"Hand Strength '2'..",
			"Hand Strength '3'..",
			"Hand Strength '4'..",
			"Hand Strength '5'..",
			"Hand Strength '6'..",
			"Hand Strength '7'..",
			"Hand Strength '8'..",
			"Hand Strength '9'..",
			"Hand Strength '10'.",
			"Hand Strength '11'.",
			"Hand Strength '12'.",
			"Hand Strength '13'.",
			"Hand Strength '14'.",
			"Strength (PF%)    \\"
	};
#else
	static const char* szHandStrengths[nHANDS + 1] = {
			"No pair (0 p-ocs) B",
			"No pair (0 p-ocs) A",
			"No pair (1 p-oc)  B",
			"No pair (1 p-oc)  A",
			"No pair (2 p-ocs) B",
			"No pair (2 p-ocs) A",
			"Weak pair         B",
			"Weak pair         A",
			"Medium pair       B",
			"Medium pair       A",
			"Good pair         B",
			"Good pair         A",
			"2 pair+           *",
			// (no others at the moment)
			"Strength (PF%)    \\"
	};
#endif //!__ABSTRACT_DISPLAY
}; // HandStrengths


//______________________________________________________________________________________________
//______________________________________________________________________________________________

struct P1Strategy {

	int nHandVal;
	
	double dPF_freq; // (St0 freq)
	double dSt1_freq[GameActions::ACTIONS_NUMACTS]; // (St1 freq)
	
	double dProbAct_St1[GameActions::P1STRAT_NUMACTS];	
	double dEV_St1[GameActions::P1STRAT_NUMACTS];	
	
#ifndef __SINGLE_STREET
	double dProbAct_St2[GameActions::ACTIONS_NUMACTS][GameActions::P1STRAT_NUMACTS];
	double dEV_St2[GameActions::ACTIONS_NUMACTS][GameActions::P1STRAT_NUMACTS];	
#endif //__SINGLE_STREET

	P1Strategy() { // (Just create a uniform strategy to start from)
		dPF_freq = 0.0;
		for (int i = 0; i < GameActions::P1STRAT_NUMACTS; ++i) {
			dProbAct_St1[i] = 1.0/(double)GameActions::P1STRAT_NUMACTS_ACT;
			dEV_St1[i] = 0.0;
#ifndef __SINGLE_STREET
			for (int j = 0; j < GameActions::ACTIONS_NUMACTS; ++j) {
				dProbAct_St2[j][i] = 1.0/(double)GameActions::P1STRAT_NUMACTS_ACT;				
			}
#endif //__SINGLE_STREET
		}
	}
	static void initializePreflop(P1Strategy *pHandRange, const double* pdPfArray)
	{
		P1Strategy *pHand = pHandRange;
		const double* dit = pdPfArray;		
		for (int j = 0; j < HandStrengths::nHANDS; ++j, ++pHand, ++dit) {			
			pHand->dPF_freq = *dit;
			pHand->nHandVal = j;
		}
	}
#if 0
	static void displayStrategy(P1Strategy *pHandRange, int nStreet, int *pnPrevStreetActions = 0)
	{
		double *dPF_st;
		if (1 == nStreet) {
			dPF_st = pHand->dProbAct_St1;
		}
		else if (2 == nStreet) {
			dPF_st = pHand->dProbAct_St1[pnPrevStreetActions[0]];			
		}
		else if (3 == nStreet) {
			dPF_st = pHand->dProbAct_St1[pnPrevStreetActions[0]][pnPrevStreetActions[1]];						
		}
		else {
			return;
		}
	}
#endif //0

	static void displayStrategy_St1(P1Strategy *pHandRange)
	{		
		double dTotals[GameActions::P1STRAT_NUMACTS] = { 0.0 };
		P1Strategy *pHand = pHandRange;
		printf("    %s  Action>  ", HandStrengths::szHandStrengths[1 + HandStrengths::nNUTS]); // (spaces to align with hand info)
		for (int i = 0; i < GameActions::P1STRAT_NUMACTS_B_ACT; ++i) {
			if (i == GameActions::P1STRAT_NUMACTS_K_ACT) {
				i = GameActions::P1STRAT_NUMACTS_B_INIT;				
			}
			printf("  %3s     ", GameActions::szP1STRAT[i]); // (so 3+6+1B per column, see below, matches 9B for vals)
		}
		printf("\n");
		for (int j = 0; j <= HandStrengths::nNUTS; ++j, ++pHand) {

			printf("%2d) %s  (%3d%%): ", j, HandStrengths::szHandStrengths[j], (int)(pHand->dPF_freq*100.0 + 0.001));

			for (int i = 0; i < GameActions::P1STRAT_NUMACTS_B_ACT; ++i) {
				if (i == GameActions::P1STRAT_NUMACTS_K_ACT) {
					i = GameActions::P1STRAT_NUMACTS_B_INIT;				
				}
				dTotals[i] += pHand->dPF_freq*pHand->dProbAct_St1[i];
				int nPct = 5*((int)(pHand->dProbAct_St1[i]*100 + 2.5)/5);
				if ((nPct > 0) && (pHand->dPF_freq > 0.01)) {
					printf("%3d%%%5.1f ", nPct, pHand->dEV_St1[i]); // (9B per column, +1 space)
				}
				else {
					printf("--------- "); // (9B per column, +1 space)					
				}
			}					
			printf("\n");
		}
		printf("\n  Total %% per action           :");
		for (int i = 0; i < GameActions::P1STRAT_NUMACTS_B_ACT; ++i) {
			if (i == GameActions::P1STRAT_NUMACTS_K_ACT) {
				i = GameActions::P1STRAT_NUMACTS_B_INIT;				
			}
			printf("   (%3d%%) ", (int)(dTotals[i]*100.0));
		}
		printf("\n\n\n");
	}
#ifndef __SINGLE_STREET
	static void displayStrategy_St2(P1Strategy *pHandRange, int nSt1Action)
	{
		double dTotals[GameActions::P1STRAT_NUMACTS] = { 0.0 };
		P1Strategy *pHand = pHandRange;
		printf("Previous Action = %s\n", GameActions::szACTIONS[nSt1Action]);
		printf("    %s  Action>  ", HandStrengths::szHandStrengths[1 + HandStrengths::nNUTS]); // (spaces to align with hand info)

		for (int i = 0; i < GameActions::P1STRAT_NUMACTS_ACT; ++i) {
			if (i == GameActions::P1STRAT_NUMACTS_K_ACT) {
				i = GameActions::P1STRAT_NUMACTS_B_INIT;				
			}
			printf("  %3s     ", GameActions::szP1STRAT[i]); // (so 3+6+1B per column, see below, matches 9B for vals)
		}
		printf("\n");
		for (int j = 0; j <= HandStrengths::nNUTS; ++j, ++pHand) {

			printf("%2d) %s  (%3d%%): ", j, HandStrengths::szHandStrengths[j], (int)(pHand->dSt1_freq[nSt1Action]*100.0 + 0.001));

			for (int i = 0; i < GameActions::P1STRAT_NUMACTS_B_ACT; ++i) {
				if (i == GameActions::P1STRAT_NUMACTS_K_ACT) {
					i = GameActions::P1STRAT_NUMACTS_B_INIT;				
				}
				dTotals[i] += pHand->dSt1_freq[nSt1Action]*pHand->dProbAct_St2[nSt1Action][i];
				int nPct = 5*((int)(pHand->dProbAct_St2[nSt1Action][i]*100 + 2.5)/5);
				if ((nPct > 0) && (pHand->dSt1_freq[nSt1Action] > 0.01)){
					printf("%3d%%%5.1f ", nPct, pHand->dEV_St2[nSt1Action][i]); // (9B per column, +1 space)
				}
				else {
					printf("--------- "); // (9B per column, +1 space)					
				}
			}					
			printf("\n");
		}
		printf("\n  Total %% per action           :");
		
		for (int i = 0; i < GameActions::P1STRAT_NUMACTS_B_ACT; ++i) {
			if (i == GameActions::P1STRAT_NUMACTS_K_ACT) {
				i = GameActions::P1STRAT_NUMACTS_B_INIT;				
			}
			printf("   (%3d%%) ", (int)(dTotals[i]*100.0));
		}
		printf("\n\n\n");
	}
#endif //__SINGLE_STREET
	
}; // P1Strategy

//______________________________________________________________________________________________

struct P2Strategy {

	int nHandVal;

	double dPF_freq; // (St0 freq)
	double dSt1_freq[GameActions::ACTIONS_NUMACTS]; // (St1 freq)
	
	double dProbAct_P1K_St1[GameActions::P2STRAT_P1K_NUMACTS];
	double dProbAct_P1B_St1[GameActions::P2STRAT_P1B_NUMACTS];
	double dEV_P1K_St1[GameActions::P2STRAT_P1K_NUMACTS];	
	double dEV_P1B_St1[GameActions::P2STRAT_P1B_NUMACTS];	
	
#ifndef __SINGLE_STREET
	double dProbAct_P1K_St2[GameActions::ACTIONS_NUMACTS][GameActions::P2STRAT_P1K_NUMACTS];
	double dProbAct_P1B_St2[GameActions::ACTIONS_NUMACTS][GameActions::P2STRAT_P1B_NUMACTS];
	double dEV_P1K_St2[GameActions::ACTIONS_NUMACTS][GameActions::P2STRAT_P1K_NUMACTS];	
	double dEV_P1B_St2[GameActions::ACTIONS_NUMACTS][GameActions::P2STRAT_P1B_NUMACTS];	
#endif //__SINGLE_STREET

	P2Strategy() { // (Just create a uniform strategy to start from)
		dPF_freq = 0.0;
		for (int i = 0; i < GameActions::P2STRAT_P1K_NUMACTS; ++i) {
			dProbAct_P1K_St1[i] = 1.0/(double)GameActions::P2STRAT_P1K_NUMACTS_ACT;
			dEV_P1K_St1[i] = 0.0;
#ifndef __SINGLE_STREET
			for (int j = 0; j < GameActions::ACTIONS_NUMACTS; ++j) {
				dProbAct_P1K_St2[j][i] = 1.0/(double)GameActions::P2STRAT_P1K_NUMACTS_ACT;				
			}
#endif //__SINGLE_STREET
		}
		for (int i = 0; i < GameActions::P2STRAT_P1B_NUMACTS; ++i) {
			double d = (GameActions::P2STRAT_P1B_NUMACTS_ACT > 0)?(double)GameActions::P2STRAT_P1B_NUMACTS_ACT:1.0;
			dProbAct_P1B_St1[i] = 1.0/d;
			dEV_P1B_St1[i] = 0.0;
#ifndef __SINGLE_STREET
			for (int j = 0; j < GameActions::ACTIONS_NUMACTS; ++j) {
				dProbAct_P1B_St2[j][i] = 1.0/(double)GameActions::P2STRAT_P1B_NUMACTS_ACT;				
			}
#endif //__SINGLE_STREET
		}
	}
	static void initializePreflop(P2Strategy *pHandRange, const double* pdPfArray)
	{
		P2Strategy *pHand = pHandRange;
		const double* dit = pdPfArray;		
		for (int j = 0; j < HandStrengths::nHANDS; ++j, ++pHand, ++dit) {			
			pHand->dPF_freq = *dit;
			pHand->nHandVal = j;
		}
	}	
	static void displayStrategy_St1(P2Strategy *pHandRange)
	{
		double dTotals_P1K[GameActions::P2STRAT_P1K_NUMACTS] = { 0.0 };
		double dTotals_P1B[GameActions::P2STRAT_P1B_NUMACTS] = { 0.0 };
		P2Strategy *pHand = pHandRange;
		printf("    %s  Action> ", HandStrengths::szHandStrengths[1 + HandStrengths::nNUTS]); // (spaces to align with hand info)
		for (int i = 0; i < GameActions::P2STRAT_P1K_NUMACTS_ACT; ++i) {
			printf("  %5s   ", GameActions::szP2STRAT_P1K[i]); // (so 3+6+1B per column, see below, matches 9B for vals)
		}
		printf("| ");
		for (int i = 0; i < GameActions::P2STRAT_P1B_NUMACTS_ACT; ++i) {
			printf("  %5s   ", GameActions::szP2STRAT_P1B[i]); // (so 3+6+1B per column, see below, matches 9B for vals)
		}
		printf("\n");
		for (int j = 0; j <= HandStrengths::nNUTS; ++j, ++pHand) {

			printf("%2d) %s  (%3d%%): ", j, HandStrengths::szHandStrengths[j], (int)(pHand->dPF_freq*100.0 + 0.001));
			for (int i = 0; i < GameActions::P2STRAT_P1K_NUMACTS_ACT; ++i) {
				dTotals_P1K[i] += pHand->dPF_freq*pHand->dProbAct_P1K_St1[i];
				int nPct = 5*((int)(pHand->dProbAct_P1K_St1[i]*100 + 2.5)/5);
				if ((nPct > 0) && (pHand->dPF_freq > 0.01)) {
					printf("%3d%%%5.1f ", nPct, pHand->dEV_P1K_St1[i]); // (9B per column, +1 space)
				}
				else {
					printf("--------- "); // (9B per column, +1 space)					
				}
			}					
			printf("| ");
			for (int i = 0; i < GameActions::P2STRAT_P1B_NUMACTS_ACT; ++i) {
				dTotals_P1B[i] += pHand->dPF_freq*pHand->dProbAct_P1B_St1[i];
				int nPct = 5*((int)(pHand->dProbAct_P1B_St1[i]*100 + 2.5)/5);
				if ((nPct > 0) && (pHand->dPF_freq > 0.01)) {
					printf("%3d%%%5.1f ", nPct, pHand->dEV_P1B_St1[i]); // (9B per column, +1 space)
				}
				else {
					printf("--------- "); // (9B per column, +1 space)					
				}
			}					
			printf("\n");
		}
		printf("\n  Total %% per action           : ");
		for (int i = 0; i < GameActions::P2STRAT_P1K_NUMACTS_ACT; ++i) {
			printf(" (%3d%%)   ", (int)(dTotals_P1K[i]*100.0));
		}
		printf("| ");
		for (int i = 0; i < GameActions::P2STRAT_P1B_NUMACTS_ACT; ++i) {
			printf(" (%3d%%)   ", (int)(dTotals_P1B[i]*100.0));
		}
		printf("\n\n\n");
	}
#ifndef __SINGLE_STREET
	static void displayStrategy_St2(P2Strategy *pHandRange, int nSt1Action)
	{
		double dTotals_P1K[GameActions::P2STRAT_P1K_NUMACTS] = { 0.0 };
		double dTotals_P1B[GameActions::P2STRAT_P1B_NUMACTS] = { 0.0 };
		P2Strategy *pHand = pHandRange;
		printf("Previous Action = %s\n", GameActions::szACTIONS[nSt1Action]);
		printf("    %s  Action> ", HandStrengths::szHandStrengths[1 + HandStrengths::nNUTS]); // (spaces to align with hand info)
		for (int i = 0; i < GameActions::P2STRAT_P1K_NUMACTS_ACT; ++i) {
			printf("  %5s   ", GameActions::szP2STRAT_P1K[i]); // (so 3+6+1B per column, see below, matches 9B for vals)
		}
		printf("| "); 
		for (int i = 0; i < GameActions::P2STRAT_P1B_NUMACTS_ACT; ++i) {
			printf(" %5s    ", GameActions::szP2STRAT_P1B[i]); // (so 3+6+1B per column, see below, matches 9B for vals)
		}
		printf("\n");
		for (int j = 0; j <= HandStrengths::nNUTS; ++j, ++pHand) {

			printf("%2d) %s  (%3d%%): ", j, HandStrengths::szHandStrengths[j], (int)(pHand->dSt1_freq[nSt1Action]*100.0 + 0.001));
			for (int i = 0; i < GameActions::P2STRAT_P1K_NUMACTS_ACT; ++i) {
				dTotals_P1K[i] += pHand->dSt1_freq[nSt1Action]*pHand->dProbAct_P1K_St2[nSt1Action][i];
				int nPct = 5*((int)(pHand->dProbAct_P1K_St2[nSt1Action][i]*100 + 2.5)/5);
				if ((nPct > 0) && (pHand->dSt1_freq[nSt1Action] > 0.01)){
					printf("%3d%%%5.1f ", nPct, pHand->dEV_P1K_St2[nSt1Action][i]); // (9B per column, +1 space)
				}
				else {
					printf("--------- "); // (9B per column, +1 space)					
				}
			}					
			printf("| ");
			for (int i = 0; i < GameActions::P2STRAT_P1B_NUMACTS_ACT; ++i) {
				dTotals_P1B[i] += pHand->dSt1_freq[nSt1Action]*pHand->dProbAct_P1B_St2[nSt1Action][i];
				int nPct = 5*((int)(pHand->dProbAct_P1B_St2[nSt1Action][i]*100 + 2.5)/5);
				if ((nPct > 0) && (pHand->dSt1_freq[nSt1Action] > 0.01)){
					printf("%3d%%%5.1f ", nPct, pHand->dEV_P1B_St2[nSt1Action][i]); // (9B per column, +1 space)
				}
				else {
					printf("--------- "); // (9B per column, +1 space)					
				}
			}					
			printf("\n");
		}
		printf("\n  Total %% per action           : ");
		for (int i = 0; i < GameActions::P2STRAT_P1K_NUMACTS_ACT; ++i) {
			printf(" (%3d%%)   ", (int)(dTotals_P1K[i]*100.0));
		}
		printf("| ");
		for (int i = 0; i < GameActions::P2STRAT_P1B_NUMACTS_ACT; ++i) {
			printf(" (%3d%%)   ", (int)(dTotals_P1B[i]*100.0));
		}
		printf("\n\n\n");
	}
#endif //__SINGLE_STREET
}; // P2Strategy

//______________________________________________________________________________________________
//______________________________________________________________________________________________

struct EquityCache {

// Street 1	
	
	double dEquityCache_P1_St1[GameActions::P1STRAT_NUMACTS][HandStrengths::nHANDS];
	double dWeightingCache_P1_St1[GameActions::P1STRAT_NUMACTS];
	double dEquityCache_P1K_P2_St1[GameActions::P2STRAT_P1K_NUMACTS][HandStrengths::nHANDS];
	double dWeightingCache_P1K_P2_St1[GameActions::P2STRAT_P1K_NUMACTS];
	double dEquityCache_P1B_P2_St1[GameActions::P2STRAT_P1B_NUMACTS][HandStrengths::nHANDS];
	double dWeightingCache_P1B_P2_St1[GameActions::P2STRAT_P1B_NUMACTS];

	double dWeight_P1K_St1;
	double dWeight_P1B_St1;
	
	void fillEquityCache_P1_St1(P1Strategy* pHandRange_P1);
	void fillEquityCache_P2_St1(P2Strategy* pHandRange_P2);

// Street 2	
#ifndef __SINGLE_STREET
	
#ifdef __VIRTUAL_TWO_STREET
	double dHandValueCache_P1_St2[GameActions::ACTIONS_NUMACTS][HandStrengths::nHANDS];
	double dHandValueCache_P2_St2[GameActions::ACTIONS_NUMACTS][HandStrengths::nHANDS];
	double dHandRange_P1_St2[GameActions::ACTIONS_NUMACTS][HandStrengths::nHANDS];
	double dHandRange_P2_St2[GameActions::ACTIONS_NUMACTS][HandStrengths::nHANDS];
#endif //__VIRTUAL_TWO_STREET
	
	double dEquityCache_P1_St2[GameActions::ACTIONS_NUMACTS][GameActions::P1STRAT_NUMACTS][HandStrengths::nHANDS];
	double dWeightingCache_P1_St2[GameActions::ACTIONS_NUMACTS][GameActions::P1STRAT_NUMACTS];
	double dEquityCache_P1K_P2_St2[GameActions::ACTIONS_NUMACTS][GameActions::P2STRAT_P1K_NUMACTS][HandStrengths::nHANDS];
	double dWeightingCache_P1K_P2_St2[GameActions::ACTIONS_NUMACTS][GameActions::P2STRAT_P1K_NUMACTS];
	double dEquityCache_P1B_P2_St2[GameActions::ACTIONS_NUMACTS][GameActions::P2STRAT_P1B_NUMACTS][HandStrengths::nHANDS];
	double dWeightingCache_P1B_P2_St2[GameActions::ACTIONS_NUMACTS][GameActions::P2STRAT_P1B_NUMACTS];

	double dWeight_P1K_St2[GameActions::ACTIONS_NUMACTS];
	double dWeight_P1B_St2[GameActions::ACTIONS_NUMACTS];

	double dEvCache_P1_St2[GameActions::ACTIONS_NUMACTS][HandStrengths::nHANDS];
	double dEvCache_P2_St2[GameActions::ACTIONS_NUMACTS][HandStrengths::nHANDS];
	double dEvIteration_P1_St2[GameActions::ACTIONS_NUMACTS][HandStrengths::nHANDS];
	double dEvIteration_P2_St2[GameActions::ACTIONS_NUMACTS][HandStrengths::nHANDS];

	// Basically copies of the _St1 versions
	void fillEquityCache_P1_St2(const P1Strategy* pHandRange_P1);
	void fillEquityCache_P2_St2(const P2Strategy* pHandRange_P2);
	
#endif //__SINGLE_STREET

}; // EquityCache

//______________________________________________________________________________________________

void EquityCache::fillEquityCache_P1_St1(P1Strategy* pHandRange_P1)
{
	//TODO: all this St1 hand frequency shouldn't be done here and should be optimized..(then make pHandRange_P1 const again)
	// Reset St1 hand frequency
	P1Strategy* pHand_P1 = pHandRange_P1;
	for (int j = 0; j < HandStrengths::nHANDS; ++j, pHand_P1++) {
		::memset((void*)pHand_P1->dSt1_freq, 0, sizeof(double)*GameActions::ACTIONS_NUMACTS_ACT);
	}	
	double dTotalSt1Freq[GameActions::ACTIONS_NUMACTS] = { 0.0 }; 

	int i = GameActions::P1STRAT_NUMACTS_K_INIT;
	dWeight_P1K_St1 = 0.0;
	for (; i < GameActions::P1STRAT_NUMACTS_K_ACT; ++i) {
		int *pnRevAct_init = GameActions::nRevActionTable_P1[i];

		double dFreq = 0.0;
		P1Strategy* pHand_P1 = pHandRange_P1;
		for (int j = 0; j < HandStrengths::nHANDS; ++j, pHand_P1++) {			
			double d = pHand_P1->dProbAct_St1[i]*pHand_P1->dPF_freq;

			dEquityCache_P1_St1[i][j] = dFreq + 0.5*d;
			dFreq += d;
			
			int *pnRevAct = pnRevAct_init;
			int nRevAct;
			for (; -1 != (nRevAct = *pnRevAct); ++pnRevAct) {
				pHand_P1->dSt1_freq[nRevAct] += d;
				dTotalSt1Freq[nRevAct] += d;
			}			
		}
		dWeightingCache_P1_St1[i] = dFreq;
		dWeight_P1K_St1 += dFreq;
		if (dFreq > 0.0001) dFreq = 1.0/dFreq;
		for (int j = 0; j < HandStrengths::nHANDS; ++j) {			
			dEquityCache_P1_St1[i][j] *= dFreq;
		}
	}
	dWeight_P1B_St1 = 0.0;
	for (i = GameActions::P1STRAT_NUMACTS_B_INIT; i < GameActions::P1STRAT_NUMACTS_B_ACT; ++i) {
		int *pnRevAct_init = GameActions::nRevActionTable_P1[i];

		double dFreq = 0.0;
		P1Strategy* pHand_P1 = pHandRange_P1;
		for (int j = 0; j < HandStrengths::nHANDS; ++j, pHand_P1++) {			
			double d = pHand_P1->dProbAct_St1[i]*pHand_P1->dPF_freq;
			dEquityCache_P1_St1[i][j] = dFreq + 0.5*d;
			dFreq += d;

			int *pnRevAct = pnRevAct_init;
			int nRevAct;
			for (; -1 != (nRevAct = *pnRevAct); ++pnRevAct) {
				pHand_P1->dSt1_freq[nRevAct] += d;
				dTotalSt1Freq[nRevAct] += d;
			}			
		}
		dWeightingCache_P1_St1[i] = dFreq;
		dWeight_P1B_St1 += dFreq;
		if (dFreq > 0.0001) dFreq = 1.0/dFreq;
		for (int j = 0; j < HandStrengths::nHANDS; ++j) {			
			dEquityCache_P1_St1[i][j] *= dFreq;			
		}
	}
	double dTotalWeight = dWeight_P1B_St1 + dWeight_P1K_St1;
	if (dTotalWeight > 0.0001) {
		dWeight_P1B_St1 /= dTotalWeight; 
		dWeight_P1K_St1 /= dTotalWeight;
	}

	// Complete St1 hand frequency calculations 
	pHand_P1 = pHandRange_P1;
	for (int j = 0; j < HandStrengths::nHANDS; ++j, pHand_P1++) {
		for (int i = 0; i < GameActions::ACTIONS_NUMACTS_ACT; ++i) {
			if (0 == j) {
				if (dTotalSt1Freq[i] > 0.0001) dTotalSt1Freq[i] = 1.0/dTotalSt1Freq[i]; 
			}
			pHand_P1->dSt1_freq[i] *= dTotalSt1Freq[i];
#ifdef __VIRTUAL_TWO_STREET
			dHandRange_P1_St2[i][j] = pHand_P1->dSt1_freq[i]; 
#endif //__VIRTUAL_TWO_STREET
		}
	}	
}

//______________________________________________________________________________________________

void EquityCache::fillEquityCache_P2_St1(P2Strategy* pHandRange_P2)
{	
	// Reset St1 hand frequency
	P2Strategy* pHand_P2 = pHandRange_P2;
	for (int j = 0; j < HandStrengths::nHANDS; ++j, pHand_P2++) {
		::memset((void*)pHand_P2->dSt1_freq, 0, sizeof(double)*GameActions::ACTIONS_NUMACTS_ACT);
	}	
	double dTotalSt1Freq[GameActions::ACTIONS_NUMACTS_ACT] = { 0.0 }; 
	
	for (int i = 0; i < GameActions::P2STRAT_P1K_NUMACTS_ACT; ++i) {
		int *pnRevAct_init = GameActions::nRevActionTable_P1K_P2[i];
		
		double dFreq = 0.0;
		P2Strategy* pHand_P2 = pHandRange_P2;
		for (int j = 0; j < HandStrengths::nHANDS; ++j, pHand_P2++) {			
			double d = pHand_P2->dProbAct_P1K_St1[i]*pHand_P2->dPF_freq;
			dEquityCache_P1K_P2_St1[i][j] = dFreq + 0.5*d;
			dFreq += d;

			int *pnRevAct = pnRevAct_init;
			int nRevAct;
			for (; -1 != (nRevAct = *pnRevAct); ++pnRevAct) {
				pHand_P2->dSt1_freq[nRevAct] += d;
				dTotalSt1Freq[nRevAct] += d;
			}			
		}
		dWeightingCache_P1K_P2_St1[i] = dFreq;
		if (dFreq > 0.0001) dFreq = 1.0/dFreq;
		for (int j = 0; j < HandStrengths::nHANDS; ++j) {			
			dEquityCache_P1K_P2_St1[i][j] *= dFreq;			
		}
	}
	for (int i = 0; i < GameActions::P2STRAT_P1B_NUMACTS_ACT; ++i) {
		int *pnRevAct_init = GameActions::nRevActionTable_P1B_P2[i];
		
		double dFreq = 0.0;
		P2Strategy* pHand_P2 = pHandRange_P2;
		for (int j = 0; j < HandStrengths::nHANDS; ++j, pHand_P2++) {			
			double d = pHand_P2->dProbAct_P1B_St1[i]*pHand_P2->dPF_freq;
			dEquityCache_P1B_P2_St1[i][j] = dFreq + 0.5*d;
			dFreq += d;
			
			int *pnRevAct = pnRevAct_init;
			int nRevAct;
			for (; -1 != (nRevAct = *pnRevAct); ++pnRevAct) {
				pHand_P2->dSt1_freq[nRevAct] += d;
				dTotalSt1Freq[nRevAct] += d;
			}			
		}
		dWeightingCache_P1B_P2_St1[i] = dFreq;
		if (dFreq > 0.0001) dFreq = 1.0/dFreq;
		for (int j = 0; j < HandStrengths::nHANDS; ++j) {			
			dEquityCache_P1B_P2_St1[i][j] *= dFreq;			
		}
	}

	// Complete St1 hand frequency calculations 
	pHand_P2 = pHandRange_P2;
	for (int j = 0; j < HandStrengths::nHANDS; ++j, pHand_P2++) {
		for (int i = 0; i < GameActions::ACTIONS_NUMACTS_ACT; ++i) {			
			if (0 == j) {
				if (dTotalSt1Freq[i] > 0.0001) dTotalSt1Freq[i] = 1.0/dTotalSt1Freq[i]; 
			}
			pHand_P2->dSt1_freq[i] *= dTotalSt1Freq[i];
#ifdef __VIRTUAL_TWO_STREET
			dHandRange_P2_St2[i][j] = pHand_P2->dSt1_freq[i]; 
#endif //__VIRTUAL_TWO_STREET
		}
	}	
}

//______________________________________________________________________________________________

#ifndef __SINGLE_STREET

void EquityCache::fillEquityCache_P1_St2(const P1Strategy* pHandRange_P1)
{
	for (int k = 0; k < GameActions::ACTIONS_NUMACTS_ACT; ++k) {
		
		for (int j = 0; j < HandStrengths::nHANDS; ++j) {			
			dEvIteration_P2_St2[k][j] = 2.0;
				// (note this one's the other way round because you need P1 weighting while processing P2)
		}		
		int i = GameActions::P1STRAT_NUMACTS_K_INIT;
		dWeight_P1K_St2[k] = 0.0;
		for (; i < GameActions::P1STRAT_NUMACTS_K_ACT; ++i) {
	
			double dFreq = 0.0;
			const P1Strategy* pHand_P1 = pHandRange_P1;
			for (int j = 0; j < HandStrengths::nHANDS; ++j, pHand_P1++) {			
				double d = pHand_P1->dProbAct_St2[k][i]*pHand_P1->dSt1_freq[k];
				dEquityCache_P1_St2[k][i][j] = dFreq + 0.5*d;
				dFreq += d;
			}
			dWeightingCache_P1_St2[k][i] = dFreq;
			dWeight_P1K_St2[k] += dFreq;
			if (dFreq > 0.0001) dFreq = 1.0/dFreq;
			for (int j = 0; j < HandStrengths::nHANDS; ++j) {			
				dEquityCache_P1_St2[k][i][j] *= dFreq;			
			}
		}
		dWeight_P1B_St2[k] = 0.0;
		for (i = GameActions::P1STRAT_NUMACTS_B_INIT; i < GameActions::P1STRAT_NUMACTS_B_ACT; ++i) {
	
			double dFreq = 0.0;
			const P1Strategy* pHand_P1 = pHandRange_P1;
			for (int j = 0; j < HandStrengths::nHANDS; ++j, pHand_P1++) {			
				double d = pHand_P1->dProbAct_St2[k][i]*pHand_P1->dSt1_freq[k];
				dEquityCache_P1_St2[k][i][j] = dFreq + 0.5*d;
				dFreq += d;
			}
			dWeightingCache_P1_St2[k][i] = dFreq;
			dWeight_P1B_St2[k] += dFreq;
			if (dFreq > 0.0001) dFreq = 1.0/dFreq;
			for (int j = 0; j < HandStrengths::nHANDS; ++j) {			
				dEquityCache_P1_St2[k][i][j] *= dFreq;			
			}
		}
		double dTotalWeight = dWeight_P1B_St2[k] + dWeight_P1K_St2[k];
		if (dTotalWeight > 0.0001) {
			dWeight_P1B_St2[k] /= dTotalWeight; 
			dWeight_P1K_St2[k] /= dTotalWeight;
		}
	}
}

//______________________________________________________________________________________________

void EquityCache::fillEquityCache_P2_St2(const P2Strategy* pHandRange_P2)
{	
	for (int k = 0; k < GameActions::ACTIONS_NUMACTS_ACT; ++k) {

		for (int j = 0; j < HandStrengths::nHANDS; ++j) {			
			dEvIteration_P1_St2[k][j] = 10.0;
				// (note this one's the other way round because you need P2 weighting while processing P1)
		}
		for (int i = 0; i < GameActions::P2STRAT_P1K_NUMACTS_ACT; ++i) {
			
			double dFreq = 0.0;
			const P2Strategy* pHand_P2 = pHandRange_P2;
			for (int j = 0; j < HandStrengths::nHANDS; ++j, pHand_P2++) {			
				double d = pHand_P2->dProbAct_P1K_St2[k][i]*pHand_P2->dSt1_freq[k];
				dEquityCache_P1K_P2_St2[k][i][j] = dFreq + 0.5*d;
				dFreq += d;
			}
			dWeightingCache_P1K_P2_St2[k][i] = dFreq;
			if (dFreq > 0.0001) dFreq = 1.0/dFreq;
			for (int j = 0; j < HandStrengths::nHANDS; ++j) {			
				dEquityCache_P1K_P2_St2[k][i][j] *= dFreq;			
			}
		}
		for (int i = 0; i < GameActions::P2STRAT_P1B_NUMACTS_ACT; ++i) {
			
			double dFreq = 0.0;
			const P2Strategy* pHand_P2 = pHandRange_P2;
			for (int j = 0; j < HandStrengths::nHANDS; ++j, pHand_P2++) {			
				double d = pHand_P2->dProbAct_P1B_St2[k][i]*pHand_P2->dSt1_freq[k];
				dEquityCache_P1B_P2_St2[k][i][j] = dFreq + 0.5*d;
				dFreq += d;
			}
			dWeightingCache_P1B_P2_St2[k][i] = dFreq;
			if (dFreq > 0.0001) dFreq = 1.0/dFreq;
			for (int j = 0; j < HandStrengths::nHANDS; ++j) {			
				dEquityCache_P1B_P2_St2[k][i][j] *= dFreq;			
			}
		}
	}
}

#endif //__SINGLE_STREET

//________________________________________________________________________________s______________
//______________________________________________________________________________________________

namespace Strategy {

	// Detailed calcs - these are basically copies of one another
	static double calculateBRandEV_Hand_P1_St1(P1Strategy* pHand_P1, const P2Strategy *pHandRange_P2, double dAlpha);
	static double calculateBRandEV_Hand_P2_St1(P2Strategy* pHand_P2, const P1Strategy *pHandRange_P1, double dAlpha);
#ifndef __SINGLE_STREET
	static double calculateBRandEV_Hand_P1_St2(P1Strategy* pHand_P1, const P2Strategy *pHandRange_P2, int nSt1Action, double dAlpha);
	static double calculateBRandEV_Hand_P2_St2(P2Strategy* pHand_P2, const P1Strategy *pHandRange_P1, int nSt1Action, double dAlpha);
#endif //__SINGLE_STREET

	// Simple umbrella calling functions
	static double calculateBRandEV_P1_St1(P1Strategy *pHandRange_P1, const P2Strategy *pHandRange_P2, double dAlpha);
	static double calculateBRandEV_P2_St1(P2Strategy *pHandRange_P2, const P1Strategy *pHandRange_P1, double dAlpha);

	static EquityCache equityCache;
	
}; // namespace Strategy

//______________________________________________________________________________________________

static double Strategy::calculateBRandEV_Hand_P1_St1(P1Strategy* pHand_P1, const P2Strategy *pHandRange_P2, double dAlpha)
{
	double dFactor = (1.0 - dAlpha);

#ifdef __SINGLE_STREET
	int nHandVal_P1 = pHand_P1->nHandVal; // (only needed for single street version)
#endif //__SINGLE_STREET

	int nBestStrat = GameActions::P1STRAT_KF;
	double dEV_max = 0.0;

#ifdef __SMOOTH_FP
	double dExpEvSum = 0.0;
#endif //__SMOOTH_FP
	
	// Loop over check-actions:
	int i = GameActions::P1STRAT_NUMACTS_K_INIT;
	for (; i < GameActions::P1STRAT_NUMACTS_K_ACT; ++i) {

		double dEV = 0.0;
		for (int j = 0; j < GameActions::P2STRAT_P1K_NUMACTS_ACT; ++j) {
			int nAction = GameActions::nActionTableP1_P1K[i][j]; 

			double dWeight = Strategy::equityCache.dWeightingCache_P1K_P2_St1[j]; 

			if (nAction <= GameActions::ACTIONS_P1FOLD_ACT) { //P1 folds			
				dEV -= dWeight*(GameActions::dCostTableP1_P1K[i][j]);
			}
			else if (nAction <= GameActions::ACTIONS_P2FOLD_ACT) { // P2 folds
				dEV += dWeight*(GameActions::dPot + GameActions::dCostTableP2_P1K[i][j]);
			}
			else {			
				if (dWeight > 0.0001) { // (if this combination ever occurs)
#ifndef __SINGLE_STREET
					// 2+ street action
#ifdef __VIRTUAL_TWO_STREET
					double d = Strategy::equityCache.dHandValueCache_P1_St2[nAction][pHand_P1->nHandVal];
#else					
					double d = Strategy::calculateBRandEV_Hand_P1_St2(pHand_P1, pHandRange_P2, nAction, dAlpha);
#endif //__VIRTUAL_TWO_STREET
					dEV -= dWeight*GameActions::dCostTableP1_P1K[i][j];
					dEV += dWeight*d*(GameActions::dCostTableP2_P1K[i][j] + GameActions::dCostTableP1_P1K[i][j] + GameActions::dPot);
#else					
					// Single street action
					double dEq = Strategy::equityCache.dEquityCache_P1K_P2_St1[j][nHandVal_P1];
					dEV += dWeight*((dEq - 1.0)*GameActions::dCostTableP1_P1K[i][j] + dEq*(GameActions::dPot + GameActions::dCostTableP2_P1K[i][j]));
#endif //!SINGLE_STREET
				}
			}
		}
		if (dEV > dEV_max) {
			nBestStrat = i;
			dEV_max = dEV;
		}
		// Adjust using Fictitious Play algorithm
#ifndef __SMOOTH_FP
		pHand_P1->dEV_St1[i] = dFactor*pHand_P1->dEV_St1[i] + dAlpha*dEV;
#else
		pHand_P1->dEV_St1[i] = dEV;
		dExpEvSum += exp(_dEntropyWeightInv*dEV);
#endif //__SMOOTH_FP
	}
	// Loop over bet actions:
	int k = 0;
	for (i = GameActions::P1STRAT_NUMACTS_B_INIT; i < GameActions::P1STRAT_NUMACTS_B_ACT; ++i, ++k) {
		double dEV = 0.0;
		for (int j = 0; j < GameActions::P2STRAT_P1B_NUMACTS_ACT; ++j) {
			
			int nAction = GameActions::nActionTableP1_P1B[k][j]; 			
			double dWeight = Strategy::equityCache.dWeightingCache_P1B_P2_St1[j];
			
			if (nAction <= GameActions::ACTIONS_P1FOLD_ACT) { //P1 folds			
				dEV -= dWeight*(GameActions::dCostTableP1_P1B[k][j]);
			}
			else if (nAction <= GameActions::ACTIONS_P2FOLD_ACT) { // P2 folds
				dEV += dWeight*(GameActions::dPot + GameActions::dCostTableP2_P1B[k][j]);
			}
			else {
				if (dWeight > 0.0001) { // (if this combination ever occurs) 
#ifndef __SINGLE_STREET
					// 2+ street action
#ifdef __VIRTUAL_TWO_STREET
					double d = Strategy::equityCache.dHandValueCache_P1_St2[nAction][pHand_P1->nHandVal];
#else					
					double d = Strategy::calculateBRandEV_Hand_P1_St2(pHand_P1, pHandRange_P2, nAction, dAlpha);
#endif //__VIRTUAL_TWO_STREET					
					dEV -= dWeight*GameActions::dCostTableP1_P1B[k][j];
					dEV += dWeight*d*(GameActions::dCostTableP2_P1B[k][j] + GameActions::dCostTableP1_P1B[k][j] + GameActions::dPot);
#else					
					// Single street action
					double dEq = Strategy::equityCache.dEquityCache_P1B_P2_St1[j][nHandVal_P1];
					dEV += dWeight*((dEq - 1.0)*GameActions::dCostTableP1_P1B[k][j] + dEq*(GameActions::dPot + GameActions::dCostTableP2_P1B[k][j]));
#endif //!SINGLE_STREET
				}
			}
		}
		if (dEV > dEV_max) {
			nBestStrat = i;
			dEV_max = dEV;
		}
		// Adjust using Fictitious Play algorithm
#ifndef __SMOOTH_FP
		pHand_P1->dEV_St1[i] = dFactor*pHand_P1->dEV_St1[i] + dAlpha*dEV;
#else
		pHand_P1->dEV_St1[i] = dEV;
		dExpEvSum += exp(_dEntropyWeightInv*dEV);
#endif //__SMOOTH_FP
	}
	
#ifdef __SMOOTH_FP
	dExpEvSum = dAlpha/dExpEvSum;
#endif //__SMOOTH_FP
	// Adjust using Fictitious Play algorithm
	for (int j = 0; j < GameActions::P1STRAT_NUMACTS; ++j) {
		pHand_P1->dProbAct_St1[j] = dFactor*pHand_P1->dProbAct_St1[j];
#ifdef __SMOOTH_FP
		pHand_P1->dProbAct_St1[j] += dExpEvSum*exp(_dEntropyWeightInv*pHand_P1->dEV_St1[j]);
#endif //__SMOOTH_FP
	
#ifdef __PERTURBED_GAME
		if ((pHand_P1->dProbAct_St1[j] < 0.01) && (nBestStrat != j)) {
			pHand_P1->dProbAct_St1[j] = 0.01;
		}
#endif //__PERTURBED_GAME
	}
#ifndef __SMOOTH_FP
	pHand_P1->dProbAct_St1[nBestStrat] += dAlpha;
#endif //!__SMOOTH_FP
	
	return dEV_max;
}

//______________________________________________________________________________________________

static double Strategy::calculateBRandEV_Hand_P2_St1(P2Strategy* pHand_P2, const P1Strategy *pHandRange_P1, double dAlpha)
{
	double dFactor = (1.0 - dAlpha);
#ifdef __SINGLE_STREET
	int nHandVal_P2 = pHand_P2->nHandVal; // (only needed for single street version)
#endif //__SINGLE_STREET
#ifdef __SMOOTH_FP
	double dExpEvSum_K = 0.0, dExpEvSum_B = 0.0;
#endif //__SMOOTH_FP
	
	// Loop over check-actions:
	int nBestStrat_P1K = GameActions::P2STRAT_P1K_K; 
	double dEV_max_P1K = 0.0;
	for (int i = 0; i < GameActions::P2STRAT_P1K_NUMACTS_ACT; ++i) { 
		double dEV = 0.0;
		double dTotalWeight_K = 0.0;
		for (int j = GameActions::P1STRAT_NUMACTS_K_INIT; j < GameActions::P1STRAT_NUMACTS_K_ACT; ++j) {
			int nAction = GameActions::nActionTableP1_P1K[j][i]; 
			double dWeight = Strategy::equityCache.dWeightingCache_P1_St1[j];
			dTotalWeight_K += dWeight;
			if (nAction <= GameActions::ACTIONS_P1FOLD_ACT) { //P1 folds			
				dEV += dWeight*(GameActions::dPot + GameActions::dCostTableP1_P1K[j][i]);
			}
			else if (nAction <= GameActions::ACTIONS_P2FOLD_ACT) { // P2 folds
				dEV -= dWeight*(GameActions::dCostTableP2_P1K[j][i]);
			}
			else {			
				if (dWeight > 0.0001) { // (if this combination ever occurs) 
					// 2+ street action
#ifndef __SINGLE_STREET
#ifdef __VIRTUAL_TWO_STREET
					double d = Strategy::equityCache.dHandValueCache_P2_St2[nAction][pHand_P2->nHandVal];
#else					
					double d = Strategy::calculateBRandEV_Hand_P2_St2(pHand_P2, pHandRange_P1, nAction, dAlpha);
#endif //__VIRTUAL_TWO_STREET
					dEV -= dWeight*GameActions::dCostTableP2_P1K[j][i];
					dEV += dWeight*d*(GameActions::dCostTableP2_P1K[j][i] + GameActions::dCostTableP1_P1K[j][i] + GameActions::dPot);
#else					
					// Single street action
					double dEq = Strategy::equityCache.dEquityCache_P1_St1[j][nHandVal_P2]; 
					dEV += dWeight*((dEq - 1.0)*GameActions::dCostTableP2_P1K[j][i] + dEq*(GameActions::dPot + GameActions::dCostTableP1_P1K[j][i]));
#endif //SINGLE_STREET
					
				}
			}
		}
		if (dTotalWeight_K < 0.0001) {
			dEV = 0.0;
		}
		else {
			dEV /= dTotalWeight_K;
		}
		if (dEV > dEV_max_P1K) {
			nBestStrat_P1K = i;
			dEV_max_P1K = dEV;
		}
		// Adjust using Fictitious Play algorithm
#ifndef __SMOOTH_FP
		pHand_P2->dEV_P1K_St1[i] = dFactor*pHand_P2->dEV_P1K_St1[i] + dAlpha*dEV; 
#else
		pHand_P2->dEV_P1K_St1[i] = dEV;
		dExpEvSum_K += exp(_dEntropyWeightInv*dEV);
#endif //__SMOOTH_FP
	}
	// Adjust using Fictitious Play algorithm
#ifdef __SMOOTH_FP
	dExpEvSum_K = dAlpha/dExpEvSum_K;
#endif //__SMOOTH_FP
	for (int j = 0; j < GameActions::P2STRAT_P1K_NUMACTS_ACT; ++j) {
		pHand_P2->dProbAct_P1K_St1[j] = dFactor*pHand_P2->dProbAct_P1K_St1[j];
#ifdef __SMOOTH_FP
		pHand_P2->dProbAct_P1K_St1[j] += dExpEvSum_K*exp(_dEntropyWeightInv*pHand_P2->dEV_P1K_St1[j]);
#endif //__SMOOTH_FP

#ifdef __PERTURBED_GAME
		if ((pHand_P2->dProbAct_P1K_St1[j] < 0.01) && (nBestStrat_P1K != j)) {
			pHand_P2->dProbAct_P1K_St1[j] = 0.01;
		}
#endif //__PERTURBED_GAME
	}
#ifndef __SMOOTH_FP
	pHand_P2->dProbAct_P1K_St1[nBestStrat_P1K] += dAlpha;
#endif //!__SMOOTH_FP

	// Loop over bet-actions:
	int nBestStrat_P1B = GameActions::P2STRAT_P1B_F; 
	double dEV_max_P1B = 0.0;
	for (int i = 0; i < GameActions::P2STRAT_P1B_NUMACTS_ACT; ++i) {		
		double dEV = 0.0;
		double dTotalWeight_B = 0.0;
		for (int j = GameActions::P1STRAT_NUMACTS_B_INIT, k = 0; j < GameActions::P1STRAT_NUMACTS_B_ACT; ++j, ++k) {			
			int nAction = GameActions::nActionTableP1_P1B[k][i]; 
			double dWeight = Strategy::equityCache.dWeightingCache_P1_St1[j];
			dTotalWeight_B += dWeight;
			if (nAction <= GameActions::ACTIONS_P1FOLD_ACT) { //P1 folds			
				dEV += dWeight*(GameActions::dPot + GameActions::dCostTableP1_P1B[k][i]);
			}
			else if (nAction <= GameActions::ACTIONS_P2FOLD_ACT) { // P2 folds
				dEV -= dWeight*(GameActions::dCostTableP2_P1B[k][i]);
			}
			else {			
				if (dWeight > 0.0001) { // (if this combination ever occurs) 
					// 2+ street action
#ifndef __SINGLE_STREET
#ifdef __VIRTUAL_TWO_STREET
					double d = Strategy::equityCache.dHandValueCache_P2_St2[nAction][pHand_P2->nHandVal];
#else					
					double d = Strategy::calculateBRandEV_Hand_P2_St2(pHand_P2, pHandRange_P1, nAction, dAlpha);
#endif //__VIRTUAL_TWO_STREET
					dEV -= dWeight*GameActions::dCostTableP2_P1B[k][i];
					dEV += dWeight*d*(GameActions::dCostTableP2_P1B[k][i] + GameActions::dCostTableP1_P1B[k][i] + GameActions::dPot);
#else					
					// Single street action
					double dEq = Strategy::equityCache.dEquityCache_P1_St1[j][nHandVal_P2]; 
					dEV += dWeight*((dEq - 1.0)*GameActions::dCostTableP2_P1B[k][i] + dEq*(GameActions::dPot + GameActions::dCostTableP1_P1B[k][i]));
#endif //SINGLE_STREET					
				}
			}
		}
		if (dTotalWeight_B < 0.0001) {
			dEV = 0.0;
		}
		else {
			dEV /= dTotalWeight_B;
		}
		if (dEV > dEV_max_P1B) {
			nBestStrat_P1B = i;
			dEV_max_P1B = dEV;
		}
		// Adjust using Fictitious Play algorithm
#ifndef __SMOOTH_FP
		pHand_P2->dEV_P1B_St1[i] = dFactor*pHand_P2->dEV_P1B_St1[i] + dAlpha*dEV; 
#else
		pHand_P2->dEV_P1B_St1[i] = dEV;
		dExpEvSum_B += exp(_dEntropyWeightInv*dEV);
#endif //__SMOOTH_FP
	}
	
	// Adjust using Fictitious Play algorithm
#ifdef __SMOOTH_FP
	dExpEvSum_B = dAlpha/dExpEvSum_B;
#endif //__SMOOTH_FP
	for (int j = 0; j < GameActions::P2STRAT_P1B_NUMACTS_ACT; ++j) {
		pHand_P2->dProbAct_P1B_St1[j] = dFactor*pHand_P2->dProbAct_P1B_St1[j];
#ifdef __SMOOTH_FP
		pHand_P2->dProbAct_P1B_St1[j] += dExpEvSum_B*exp(_dEntropyWeightInv*pHand_P2->dEV_P1B_St1[j]);
#endif //__SMOOTH_FP
#ifdef __PERTURBED_GAME
		if ((pHand_P2->dProbAct_P1B_St1[j] < 0.01) && (nBestStrat_P1B != j)) {
			pHand_P2->dProbAct_P1B_St1[j] = 0.01;
		}
#endif //__PERTURBED_GAME
	}
#ifndef __SMOOTH_FP
	pHand_P2->dProbAct_P1B_St1[nBestStrat_P1B] += dAlpha;
#endif //__SMOOTH_FP

	return Strategy::equityCache.dWeight_P1B_St1*dEV_max_P1B + Strategy::equityCache.dWeight_P1K_St1*dEV_max_P1K;
		// P2 EV is a weighted average of EV vs different P1 actions
}

//______________________________________________________________________________________________

#ifndef __SINGLE_STREET

static double Strategy::calculateBRandEV_Hand_P1_St2(P1Strategy* pHand_P1, const P2Strategy *pHandRange_P2, int nSt1Action, double dAlpha)
{
//TODO: capped pots...?	
	
	int nHandVal_P1 = pHand_P1->nHandVal;

	if (dAlpha == Strategy::equityCache.dEvIteration_P1_St2[nSt1Action][nHandVal_P1]) {
		double dCachedEv = Strategy::equityCache.dEvCache_P1_St2[nSt1Action][nHandVal_P1];
		return dCachedEv;
	}

	double dFactor = (1.0 - dAlpha);

	int nBestStrat = GameActions::P1STRAT_KF;
	double dEV_max = 0.0;
	
	// Loop over check-actions:
	int i = GameActions::P1STRAT_NUMACTS_K_INIT;
	for (; i < GameActions::P1STRAT_NUMACTS_K_ACT; ++i) {

		double dEV = 0.0;
		for (int j = 0; j < GameActions::P2STRAT_P1K_NUMACTS_ACT; ++j) {
			int nAction = GameActions::nActionTableP1_P1K[i][j]; 
			double dWeight = Strategy::equityCache.dWeightingCache_P1K_P2_St2[nSt1Action][j]; 

			if (nAction <= GameActions::ACTIONS_P1FOLD_ACT) { //P1 folds			
				dEV -= dWeight*(GameActions::dCostTableP1_P1K[i][j]);
			}
			else if (nAction <= GameActions::ACTIONS_P2FOLD_ACT) { // P2 folds
				dEV += dWeight*(GameActions::dPot + GameActions::dCostTableP2_P1K[i][j]);
			}
			else {			
				if (dWeight > 0.0001) { // (if this combination ever occurs)
					// 3-street action
					// dEV += dWeight*Strategy::calculateBRandEV_St3_Hand_P1(pHand_P1, pHandRange_P2, nAction, dAlpha);
					// 2-street action
					double dEq = Strategy::equityCache.dEquityCache_P1K_P2_St2[nSt1Action][j][nHandVal_P1];
					dEV += dWeight*((dEq - 1.0)*GameActions::dCostTableP1_P1K[i][j] + dEq*(GameActions::dPot + GameActions::dCostTableP2_P1K[i][j]));
				}
			}
		}
		if (dEV > dEV_max) {
			nBestStrat = i;
			dEV_max = dEV;
		}
		// Adjust using Fictitious Play algorithm
		pHand_P1->dEV_St2[nSt1Action][i] = dFactor*pHand_P1->dEV_St2[nSt1Action][i] + dAlpha*dEV; 
	}
	int k = 0;
	for (i = GameActions::P1STRAT_NUMACTS_B_INIT; i < GameActions::P1STRAT_NUMACTS_B_ACT; ++i, ++k) {
		
		double dEV = 0.0;
		for (int j = 0; j < GameActions::P2STRAT_P1B_NUMACTS_ACT; ++j) {			
			int nAction = GameActions::nActionTableP1_P1B[k][j]; 			
			double dWeight = Strategy::equityCache.dWeightingCache_P1B_P2_St2[nSt1Action][j];
			
			if (nAction <= GameActions::ACTIONS_P1FOLD_ACT) { //P1 folds			
				dEV -= dWeight*(GameActions::dCostTableP1_P1B[k][j]);
			}
			else if (nAction <= GameActions::ACTIONS_P2FOLD_ACT) { // P2 folds
				dEV += dWeight*(GameActions::dPot + GameActions::dCostTableP2_P1B[k][j]);
			}
			else {
				if (dWeight > 0.0001) { // (if this combination ever occurs) 
					// 3-street action
					// dEV += dWeight*Strategy::calculateBRandEV_Hand_P1_St2(pHand_P1, pHandRange_P2, nAction, dAlpha);
					// 2-street action
					double dEq = Strategy::equityCache.dEquityCache_P1B_P2_St2[nSt1Action][j][nHandVal_P1]; 
					dEV += dWeight*((dEq - 1.0)*GameActions::dCostTableP1_P1B[k][j] + dEq*(GameActions::dPot + GameActions::dCostTableP2_P1B[k][j]));
				}
			}
		}
		if (dEV > dEV_max) {
			nBestStrat = i;
			dEV_max = dEV;
		}
		// Adjust using Fictitious Play algorithm
		pHand_P1->dEV_St2[nSt1Action][i] = dFactor*pHand_P1->dEV_St2[nSt1Action][i] + dAlpha*dEV; 
	}
	
	// Adjust using Fictitious Play algorithm
	for (int j = 0; j < GameActions::P1STRAT_NUMACTS; ++j) {
		pHand_P1->dProbAct_St2[nSt1Action][j] = dFactor*pHand_P1->dProbAct_St2[nSt1Action][j];

#ifdef __PERTURBED_GAME
		if ((pHand_P1->dProbAct_St2[nSt1Action][j] < 0.01) && (nBestStrat != j)) {
			pHand_P1->dProbAct_St2[nSt1Action][j] = 0.01;
		}
#endif //__PERTURBED_GAME
	}
	pHand_P1->dProbAct_St2[nSt1Action][nBestStrat] += dAlpha;

	Strategy::equityCache.dEvIteration_P1_St2[nSt1Action][nHandVal_P1] = dAlpha;
	Strategy::equityCache.dEvCache_P1_St2[nSt1Action][nHandVal_P1] = dEV_max;
	return dEV_max;
}

//______________________________________________________________________________________________

static double Strategy::calculateBRandEV_Hand_P2_St2(P2Strategy* pHand_P2, const P1Strategy *pHandRange_P1, int nSt1Action, double dAlpha)
{
	int nHandVal_P2 = pHand_P2->nHandVal;
	
	if (dAlpha == Strategy::equityCache.dEvIteration_P2_St2[nSt1Action][nHandVal_P2]) {
		double dCachedEv = Strategy::equityCache.dEvCache_P2_St2[nSt1Action][nHandVal_P2]; 
		return dCachedEv;
	}
	double dFactor = (1.0 - dAlpha);

	// Loop over check-actions:
	int nBestStrat_P1K = GameActions::P2STRAT_P1K_K; 
	double dEV_max_P1K = 0.0;
	for (int i = 0; i < GameActions::P2STRAT_P1K_NUMACTS_ACT; ++i) { 
		double dEV = 0.0;
		double dTotalWeight_K = 0.0;
		for (int j = GameActions::P1STRAT_NUMACTS_K_INIT; j < GameActions::P1STRAT_NUMACTS_K_ACT; ++j) {
			int nAction = GameActions::nActionTableP1_P1K[j][i]; 
			double dWeight = Strategy::equityCache.dWeightingCache_P1_St2[nSt1Action][j]; 
			dTotalWeight_K += dWeight;
			
			if (nAction <= GameActions::ACTIONS_P1FOLD_ACT) { //P1 folds			
				dEV += dWeight*(GameActions::dPot + GameActions::dCostTableP1_P1K[j][i]);
			}
			else if (nAction <= GameActions::ACTIONS_P2FOLD_ACT) { // P2 folds
				dEV -= dWeight*(GameActions::dCostTableP2_P1K[j][i]);
			}
			else {			
				if (dWeight > 0.0001) { // (if this combination ever occurs) 
					// 3-street action
					// dEV += dWeight*Strategy::calculateBRandEV_Hand_P2_St2(pHand_P2, pHandRange_P1, nAction, dAlpha);
					// 2-street action
					double dEq = Strategy::equityCache.dEquityCache_P1_St2[nSt1Action][j][nHandVal_P2]; 
					dEV += dWeight*((dEq - 1.0)*GameActions::dCostTableP2_P1K[j][i] + dEq*(GameActions::dPot + GameActions::dCostTableP1_P1K[j][i]));
				}
			}
		}
		if (dTotalWeight_K < 0.0001) {
			dEV = 0.0;
		}
		else {
			dEV /= dTotalWeight_K;
		}
		if (dEV > dEV_max_P1K) {
			nBestStrat_P1K = i;
			dEV_max_P1K = dEV;
		}
		// Adjust using Fictitious Play algorithm
		pHand_P2->dEV_P1K_St2[nSt1Action][i] = dFactor*pHand_P2->dEV_P1K_St2[nSt1Action][i] + dAlpha*dEV; 
	}
	// Adjust using Fictitious Play algorithm
	for (int j = 0; j < GameActions::P2STRAT_P1K_NUMACTS_ACT; ++j) {
		pHand_P2->dProbAct_P1K_St2[nSt1Action][j] = dFactor*pHand_P2->dProbAct_P1K_St2[nSt1Action][j];
#ifdef __PERTURBED_GAME
		if ((pHand_P2->dProbAct_P1K_St2[nSt1Action][j] < 0.01) && (nBestStrat_P1K != j)) {
			pHand_P2->dProbAct_P1K_St2[nSt1Action][j] = 0.01;
		}
#endif //__PERTURBED_GAME
	}
	pHand_P2->dProbAct_P1K_St2[nSt1Action][nBestStrat_P1K] += dAlpha;

	// Loop over bet-actions:
	int nBestStrat_P1B = GameActions::P2STRAT_P1B_F; 
	double dEV_max_P1B = 0.0;
	for (int i = 0; i < GameActions::P2STRAT_P1B_NUMACTS_ACT; ++i) {		
		double dEV = 0.0;
		double dTotalWeight_B = 0.0;
		for (int j = GameActions::P1STRAT_NUMACTS_B_INIT, k = 0; j < GameActions::P1STRAT_NUMACTS_B_ACT; ++j, ++k) {			
			int nAction = GameActions::nActionTableP1_P1B[k][i]; 
			double dWeight = Strategy::equityCache.dWeightingCache_P1_St2[nSt1Action][j]; 
			dTotalWeight_B += dWeight;
			
			if (nAction <= GameActions::ACTIONS_P1FOLD_ACT) { //P1 folds			
				dEV += dWeight*(GameActions::dPot + GameActions::dCostTableP1_P1B[k][i]);
			}
			else if (nAction <= GameActions::ACTIONS_P2FOLD_ACT) { // P2 folds
				dEV -= dWeight*(GameActions::dCostTableP2_P1B[k][i]);
			}
			else {			
				if (dWeight > 0.0001) { // (if this combination ever occurs) 
					// 3-street action
					// dEV += dWeight*Strategy::calculateBRandEV_Hand_P2_St2(pHand_P2, pHandRange_P1, nAction, dAlpha);
					// 2-street action
					double dEq = Strategy::equityCache.dEquityCache_P1_St2[nSt1Action][j][nHandVal_P2]; 
					dEV += dWeight*((dEq - 1.0)*GameActions::dCostTableP2_P1B[k][i] + dEq*(GameActions::dPot + GameActions::dCostTableP1_P1B[k][i]));
				}
			}
		}
		if (dTotalWeight_B < 0.0001) {
			dEV = 0.0;
		}
		else {
			dEV /= dTotalWeight_B;
		}
		if (dEV > dEV_max_P1B) {
			nBestStrat_P1B = i;
			dEV_max_P1B = dEV;
		}
		// Adjust using Fictitious Play algorithm
		pHand_P2->dEV_P1B_St2[nSt1Action][i] = dFactor*pHand_P2->dEV_P1B_St2[nSt1Action][i] + dAlpha*dEV; 
	}
	
	// Adjust using Fictitious Play algorithm
	for (int j = 0; j < GameActions::P2STRAT_P1B_NUMACTS_ACT; ++j) {
		pHand_P2->dProbAct_P1B_St2[nSt1Action][j] = dFactor*pHand_P2->dProbAct_P1B_St2[nSt1Action][j];
#ifdef __PERTURBED_GAME
		if ((pHand_P2->dProbAct_P1B_St2[nSt1Action][j] < 0.01) && (nBestStrat_P1B != j)) {
			pHand_P2->dProbAct_P1B_St2[nSt1Action][j] = 0.01;
		}
#endif //__PERTURBED_GAME
	}
	pHand_P2->dProbAct_P1B_St2[nSt1Action][nBestStrat_P1B] += dAlpha;

	Strategy::equityCache.dEvIteration_P2_St2[nSt1Action][nHandVal_P2] = dAlpha;
	double dEV_max = Strategy::equityCache.dWeight_P1B_St2[nSt1Action]*dEV_max_P1B + Strategy::equityCache.dWeight_P1K_St2[nSt1Action]*dEV_max_P1K; 
		// P2 EV is a weighted average of EV vs different P1 actions
	Strategy::equityCache.dEvCache_P2_St2[nSt1Action][nHandVal_P2] = dEV_max;	
	return dEV_max;
}

#endif //__SINGLE_STREET
//______________________________________________________________________________________________
//______________________________________________________________________________________________

static double Strategy::calculateBRandEV_P1_St1(P1Strategy *pHandRange_P1, const P2Strategy *pHandRange_P2, double dAlpha)
{
	double dEV = 0.0;

	P1Strategy *pHand_P1 = pHandRange_P1;
	for (int i = 0; i < HandStrengths::nHANDS; ++i, pHand_P1++) {
		
		double dEV_hand = Strategy::calculateBRandEV_Hand_P1_St1(pHand_P1, pHandRange_P2, dAlpha);
		dEV += dEV_hand*pHand_P1->dPF_freq;
	}	
	return dEV;
}

//______________________________________________________________________________________________

static double Strategy::calculateBRandEV_P2_St1(P2Strategy *pHandRange_P2, const P1Strategy *pHandRange_P1, double dAlpha)
{
	double dEV = 0.0;

	P2Strategy *pHand_P2 = pHandRange_P2;
	for (int i = 0; i < HandStrengths::nHANDS; ++i, pHand_P2++) {
		
		double dEV_hand = Strategy::calculateBRandEV_Hand_P2_St1(pHand_P2, pHandRange_P1, dAlpha);
		dEV += dEV_hand*pHand_P2->dPF_freq;
	}
	return dEV;
}
//______________________________________________________________________________________________
//______________________________________________________________________________________________

static void calculateCompleteSingleStreetSolution(
		const double* pdHandRange_P1, const double* pdHandRange_P2,
		double* pdHandValues_P1, double *pdHandValues_P2, bool bDisplay = false)
{
	static const double dBetScalarInv = 1.0/(1.0 + GameActions::dBetScalar);
	static const double dPotOnBetCall = 1.0 + 2.0*GameActions::dBetScalar;
	double dMainExitCriterion, dMainExitCriterion_prev = -DBL_MAX; 

#ifdef __SIMPLESTGAME_NOLEADING
	
	// Hand indices ("points to" the "bottom of the bucket", eg w/ 10 hand bins, 1==10%, 9==90% etc)
	int nIndex_KB_P2 = HandStrengths::nHANDS, nIndex_BK_P2 = 0, nIndex_FC_P1 = HandStrengths::nHANDS;
	
	double dExactIndex_BK_P2, delta_BKP2,
			dExactIndex_KB_P2, delta_KBP2,
				dExactIndex_FC_P1, delta_FCP1;
	double dGrad_BKP2, dGrad1_BKP2, dGrad_KBP2, dGrad_FCP1, dGrad2_FCP1; 
		// (the number after "Grad" denotes the player, where not the obvious one which is implied)
	
	// Cumulative probabilities
	double dCumProb_low_BKP2_P2 = 0.0, 
			dCumProb_KBP2_high_P2 = 0.0,
			dCumProb_FCP1_high_P1 = 0.0,
			dCumProb_FCP1_KBP2_P2 = 0.0, 
			dCumProb_low_BKP2_P1 = 0.0,
			dCumProb_BKP2_FCP1_P1 = 0.0;

	// Saved state:
	double dCumProb_KBP2_high_P2_prev, dCumProb_low_BKP2_P2_prev, 
			dCumProb_FCP1_KBP2_P2_prev, dCumProb_FCP1_high_P1_prev, dCumProb_BKP2_FCP1_P1_prev;
	int nIndex_BK_P2_prev, nIndex_KB_P2_prev, nIndex_FC_P1_prev;	
	
	// We are solving the following simultaneous equation:
	// (1/(1+bet%))*dCumProb_KBP2_high_P2 = dCumProb_low_BKP2_P2 ([1], P2 bluffing indifference)
	// dCumProb_KBP2_high_P2 = dCumProb_FCP1_KBP2_P2 ([2], P2 v-betting indifference)
	// bet%*dCumProb_FCP1_high_P1 = dCumProb_BKP2_FCP1_P1 ([3], P1 calling indifference)
		// note: dCumProb_BKP2_FCP1_P1 = 1.0 -  dCumProb_FCP1_high_P1 - dCumProb_low_BKP2_P1 (3.1)
	
// (A) We're going to search across nIndex_KB_P2 and get an approximate answer (ie an integer bounded solution)
		// (Cumulative prob being monotonic is heavily used here)
	
	for (nIndex_KB_P2--; nIndex_KB_P2 >= 0; nIndex_KB_P2--) {
		dGrad_KBP2 = pdHandRange_P2[nIndex_KB_P2];
		if (0.0 == dGrad_KBP2) continue; // (this doesn't add any more information, so can avoid wasting time)
		
		dCumProb_KBP2_high_P2 += dGrad_KBP2;
		dCumProb_FCP1_KBP2_P2 -= dGrad_KBP2; // (added back again - and more - in (A.2))

//printf("KB_P2=%d, cum vb = %.1f\n", nIndex_KB_P2, 100.0*dCumProb_KBP2_high_P2);

// (A.1) Find nIndex_BK_P2 satisfying (1)
		
		double dInternalExitCriterion = dBetScalarInv*dCumProb_KBP2_high_P2;
		for (; (nIndex_BK_P2 < HandStrengths::nHANDS) && (dCumProb_low_BKP2_P2 < dInternalExitCriterion); nIndex_BK_P2++) {
			// (note dCumProb_low_BKP2_P2 will start too low, so look for transition)
		
			dGrad_BKP2 = pdHandRange_P2[nIndex_BK_P2];
			dGrad1_BKP2 = pdHandRange_P1[nIndex_BK_P2];
			dCumProb_low_BKP2_P2 += dGrad_BKP2;
			// Needed for (3):
			dCumProb_low_BKP2_P1 += dGrad1_BKP2;			
		}

//printf("Found1! BK_P2=%d, cum bluff = %.1f\n", nIndex_BK_P2, 100.0*dCumProb_low_BKP2_P2);				
		
// (A.2) Find nIndex_FC_P1 satisfying (2)
		
		for (nIndex_FC_P1--; (nIndex_FC_P1 >= 0); nIndex_FC_P1--) {
			// (note dCumProb_FCP1_KBP2_P2 starts too low)
			dGrad2_FCP1 = pdHandRange_P2[nIndex_FC_P1];
			dGrad_FCP1 = pdHandRange_P1[nIndex_FC_P1];

			dCumProb_FCP1_KBP2_P2 += dGrad2_FCP1; 
			// Needed for (3):
			dCumProb_FCP1_high_P1 += dGrad_FCP1;	

			// Here need to do the break before the index decrements (because we're counting down, ie like for nIndex_KB_P2)
			if (dCumProb_FCP1_KBP2_P2 >= dCumProb_KBP2_high_P2) { 
				break;
			}
		}

//printf("Found2! FC_P1=%d, cum call = %.1f also cumP2(fcp1->kbp2)=%.1f\n", nIndex_FC_P1, 100.0*dCumProb_FCP1_high_P1, 100.0*dCumProb_FCP1_KBP2_P2);				

// (A.3) Calculate dCumProb_BKP2_FCP1_P1 using (3.1)
		
		dCumProb_BKP2_FCP1_P1 = 1.0 -  dCumProb_FCP1_high_P1 - dCumProb_low_BKP2_P1;

//printf("Compare cum call and %.1f (from %.1f)\n", 100.0*dCumProb_BKP2_FCP1_P1, 100.0*dCumProb_low_BKP2_P1);
		
// (A.4) Look for LHS(3) becoming > than RHS(3) 
			// (starts < because dCumProb_FCP1_high_P1==0 when FCP1==high)

		// Overall exit criterion
		dMainExitCriterion = GameActions::dBetScalar*dCumProb_FCP1_high_P1 - dCumProb_BKP2_FCP1_P1;
		if (dMainExitCriterion >= 0) {

			// Do we want to use previous iteration?
			// (since cum_FC_P1=2.0*cum_KB_P2 for pot-sized bets, there's a risk that delta_FC_P1 can be >1 which obv breaks the math)
			if (-dMainExitCriterion_prev < dMainExitCriterion) { // (yes)
				dCumProb_KBP2_high_P2 = dCumProb_KBP2_high_P2_prev;
				dCumProb_low_BKP2_P2 = dCumProb_low_BKP2_P2_prev;
				dCumProb_FCP1_KBP2_P2 = dCumProb_FCP1_KBP2_P2_prev;
				dCumProb_FCP1_high_P1 = dCumProb_FCP1_high_P1_prev;
				dCumProb_BKP2_FCP1_P1 = dCumProb_BKP2_FCP1_P1_prev;
				nIndex_BK_P2 = nIndex_BK_P2_prev;
				nIndex_KB_P2 = nIndex_KB_P2_prev;
				nIndex_FC_P1 = nIndex_FC_P1_prev;
				// Want the gradiants which go from N to N+1 (whereas in other direction, want gradiants from N to N-1)
				dGrad_KBP2 = pdHandRange_P2[nIndex_KB_P2];
				dGrad_BKP2 = pdHandRange_P2[nIndex_BK_P2];
				dGrad2_FCP1 = pdHandRange_P2[nIndex_FC_P1];
				dGrad_FCP1 = pdHandRange_P1[nIndex_FC_P1];
				dGrad1_BKP2 = pdHandRange_P1[nIndex_BK_P2];
			}
			
// (A.5) Now use interpolation to calculate the exact values for the threshold indices			
			
			//+++Interpolation
			// OK now I've turned (1)-(3), which were non-linear equations, into a set of linear equations (1')-(3'):
			
			// (1/(1+bet%))*(dCumProb_KBP2_high_P2 + delta_KBP2*dGrad_KBP2) = (dCumProb_low_BKP2_P2 + delta_BKP2*dGrad_BKP2) [1']
				// or: A*(B + g1*X) = C + g2*Y [1'']
			// dCumProb_KBP2_high_P2 + delta_KBP2*dGrad_KBP2 = dCumProb_FCP1_KBP2_P2 + delta_FCP1*dGrad2_FCP1 - delta_KBP2*dGrad_KBP2 [2']
				// or: B + g1*X = E - g1*X + g3*Z [2'']
			// bet%*(dCumProb_FCP1_high_P1 + delta_FCP1*dGrad_FCP1) = dCumProb_BKP2_FCP1_P1 - delta_BKP2*dGrad1_BKP2 - delta_FCP1*dGrad_FCP1;[3']
				// or: F*(G + g4*Z) = H - g5*Y - g3*Z [3'']
			{
				double A = dBetScalarInv, B = dCumProb_KBP2_high_P2;
				double g1 = dGrad_KBP2;
				double C = dCumProb_low_BKP2_P2;
				double g2 = dGrad_BKP2;
				double E = dCumProb_FCP1_KBP2_P2;
				double g3 = dGrad2_FCP1;
				double F = GameActions::dBetScalar, G = dCumProb_FCP1_high_P1;
				double g4 = dGrad_FCP1;
				double H = dCumProb_BKP2_FCP1_P1;
				double g5= dGrad1_BKP2;

				// From http://www.numberempire.com/equationsolver.php:
				delta_KBP2 = (g2*(g3*(H-F*G)+E*(g4*F+g3))+B*(g2*(-g4*F-g3)-g3*g5*A)+g3*g5*C)/(g1*g2*(2*g4*F+2*g3)+g1*g3*g5*A);
				delta_BKP2 = -(A*(g3*(F*G-H)+E*(-g4*F-g3))+C*(2*g4*F+2*g3)+A*B*(-g4*F-g3))/(g2*(2*g4*F+2*g3)+g3*g5*A);
				delta_FCP1 = (g2*(2*H-2*F*G)-g5*A*E+2*g5*C-g5*A*B)/(g2*(2*g4*F+2*g3)+g3*g5*A);
/**/if (bDisplay)
printf("Deltas KB=%.3f BK=%.3f FC=%.3f on KB=%d BK=%d FC=%d\n", delta_KBP2, delta_BKP2, delta_FCP1, nIndex_KB_P2, nIndex_BK_P2, nIndex_FC_P1);				
				
				// (At some point could consider a robustness measure where we check the deltas and use simple independent (iterative?) interpolation
				//  for each variable)
				
				// Exact indices
				dExactIndex_BK_P2 = (double)nIndex_BK_P2 + delta_BKP2; 
				dExactIndex_KB_P2 = (double)nIndex_KB_P2 - delta_KBP2; // (this index was descending in the loop above)
				dExactIndex_FC_P1 = (double)nIndex_FC_P1 - delta_FCP1; // (-"-)
				
				// Exact cum probs
				dCumProb_KBP2_high_P2 += delta_KBP2*dGrad_KBP2;
				dCumProb_low_BKP2_P2 += delta_BKP2*dGrad_BKP2;
				dCumProb_FCP1_high_P1 += delta_FCP1*dGrad_FCP1;

				// To make life easy for the value calculations (see B.*), ensure the integer index is always > the exact index
				// Also handle as robustly as possible (not massively) the case where the deltas are abs(.)>1
				
				if (delta_BKP2 > 0.0) {
					nIndex_BK_P2 += 1 + (int)delta_BKP2;
					delta_BKP2 -= 1.0 + (int)delta_BKP2;
				}
				else if (delta_BKP2 < -1.0) {
					nIndex_BK_P2 -= (int)-delta_BKP2;
					delta_BKP2 += (int)-delta_BKP2;					
				}
				if (delta_KBP2 < 0.0) { // (the next two go in the other direction...)
					nIndex_KB_P2 += 1 + (int)-delta_KBP2;
					delta_KBP2 += 1.0 + (int)-delta_KBP2;
				}
				else if (delta_KBP2 > 1.0) { // (the next two go in the other direction...)
					nIndex_KB_P2 -= (int)delta_KBP2;
					delta_KBP2 -= (int)delta_KBP2;
				}
				if (delta_FCP1 < 0.0) { // (the next two go in the other direction...)
					nIndex_FC_P1 += 1 + (int)-delta_FCP1;
					delta_FCP1 += 1.0 + (int)-delta_FCP1;
				}
				else if (delta_FCP1 > 1.0) { // (the next two go in the other direction...)
					nIndex_FC_P1 -= (int)delta_FCP1;
					delta_FCP1 -= (int)delta_FCP1;
				}

/**/if (bDisplay)
printf("Found3! KB_P2*=%.2f(%d) BK_P2*=%.2f(%d) FC_P1*=%.2f(%d) (P2 vb=%.1f, P2 bl=%.1f, P1 c=%.1f\n", 
		dExactIndex_KB_P2, nIndex_KB_P2, dExactIndex_BK_P2, nIndex_BK_P2,dExactIndex_FC_P1, nIndex_FC_P1,
		100.0*dCumProb_KBP2_high_P2, 100.0*dCumProb_low_BKP2_P2, 100.0*dCumProb_FCP1_high_P1);						
			}			
			//---Interpolation
			break;
		}
		else { // Save lots of the state so we can flip between this index and the previous one if necessary 
			dMainExitCriterion_prev = dMainExitCriterion;

			dCumProb_KBP2_high_P2_prev = dCumProb_KBP2_high_P2;
			dCumProb_low_BKP2_P2_prev = dCumProb_low_BKP2_P2;
			dCumProb_FCP1_KBP2_P2_prev = dCumProb_FCP1_KBP2_P2;
			dCumProb_FCP1_high_P1_prev = dCumProb_FCP1_high_P1;
			dCumProb_BKP2_FCP1_P1_prev = dCumProb_BKP2_FCP1_P1;
			nIndex_BK_P2_prev = nIndex_BK_P2;
			nIndex_KB_P2_prev = nIndex_KB_P2;
			nIndex_FC_P1_prev = nIndex_FC_P1;
		}
		
	} // end decreasing loop over P2 hands 

// (B) Calculate value for each player and hand:

	double dEqInc_P2, dEqInc_P1;
	
// (B.1) Calculate P2 value for game 
	
// (B.1.1) P2 value betting range 	
	
	double dPotEquity_P2 = 1.0;
	double dCumProb_low_FCP1_P1 = 1.0 - dCumProb_FCP1_high_P1;
	double dGivenP1Calls = 1.0/dCumProb_FCP1_high_P1;
	int nH_P2 = HandStrengths::nHANDS - 1; 
	for (; nH_P2 >= nIndex_KB_P2; --nH_P2) {
		dEqInc_P2 = 0.5*pdHandRange_P1[nH_P2]*dGivenP1Calls;
		dPotEquity_P2 -= dEqInc_P2;
		pdHandValues_P2[nH_P2] = dCumProb_low_FCP1_P1 + 
			dCumProb_FCP1_high_P1*(dPotOnBetCall*dPotEquity_P2 - GameActions::dBetScalar);
		dPotEquity_P2 -= dEqInc_P2;
		
	} // end loop over P2 hands, value betting

// (B.1.2) Transition from P2 value betting range to P2 checking range 	

	// (B.1.2.1) Handle the "delta" for nIndex_KB_P2, part 1:
	
	dEqInc_P2 = 0.5*delta_KBP2*pdHandRange_P1[nH_P2]*dGivenP1Calls; // (note delta_KBP2 > 0.0, see above)
	dPotEquity_P2 -= dEqInc_P2;		
	
	pdHandValues_P2[nH_P2] = delta_KBP2*
		(dCumProb_low_FCP1_P1 + 
				dCumProb_FCP1_high_P1*(dPotOnBetCall*dPotEquity_P2 - GameActions::dBetScalar));	
	dPotEquity_P2 -= dEqInc_P2;
	
	// (B.1.2.2) Handle the "1 - delta" for nIndex_KB_P2, part 2:

	dPotEquity_P2 = 1.0 - (1.0 - dPotEquity_P2)*dCumProb_FCP1_high_P1;
		// (transform from equity-vs-call to equity-vs-check, equivalent to 1.0 - dCumProb_KBP2_high_P1, which isn't calculated)
	
	dEqInc_P2 = 0.5*(1.0 - delta_KBP2)*pdHandRange_P1[nH_P2];
	dPotEquity_P2 -= dEqInc_P2;		
	
	pdHandValues_P2[nH_P2] += (1.0 - delta_KBP2)*dPotEquity_P2;
	dPotEquity_P2 -= dEqInc_P2;

	nH_P2--;
	
// (B.1.3) P2 checking range 	
	
	for (; nH_P2 >= nIndex_BK_P2; --nH_P2) {
		dEqInc_P2 = 0.5*pdHandRange_P1[nH_P2];
		dPotEquity_P2 -= dEqInc_P2;
		pdHandValues_P2[nH_P2] = dPotEquity_P2;
		dPotEquity_P2 -= dEqInc_P2;
		
	} // end loop over P2 hands, "value checking"
	
// (B.1.4) Transition from P2 checking range to P2 bluffing range 	

	// (B.1.4.1) Handle the "1 - delta" for nIndex_BK_P2, part 1:
		
	dEqInc_P2 = 0.5*(-delta_BKP2)*pdHandRange_P1[nH_P2]; // (note delta_BKP2 < 0.0, see above)
	dPotEquity_P2 -= dEqInc_P2;		
		
	pdHandValues_P2[nH_P2] = (-delta_BKP2)*dPotEquity_P2;
	dPotEquity_P2 -= dEqInc_P2;
		
	// (B.1.4.2) Handle the delta for nIndex_BK_P2, part 2:
	double dBluffVal = dCumProb_low_FCP1_P1 - dCumProb_FCP1_high_P1*GameActions::dBetScalar;

	dEqInc_P2 = -0.5*(1.0 + delta_BKP2)*pdHandRange_P1[nH_P2]; // (note delta_BKP2 < 0.0, see above)
	dPotEquity_P2 -= dEqInc_P2;
	
	pdHandValues_P2[nH_P2] += (1.0 + delta_BKP2)*dBluffVal;
	dPotEquity_P2 -= dEqInc_P2;

	nH_P2--;

// (B.1.5) P2 bluffing range		
		
	for (; nH_P2 >= 0; --nH_P2) {		
		pdHandValues_P2[nH_P2] = dBluffVal;
		
	} // end loop over P1 hands, bluffing
	
// (B.2) Calculate P2 value for game 

// (B.2.1) P1 calls in his overlap with P2's value range
	
	int nH_P1 = HandStrengths::nHANDS - 1; 
	double dTotalProbCheck_P2 = 1.0 - (dCumProb_low_BKP2_P2 + dCumProb_KBP2_high_P2);
	double dPotEquity_P1 = 1.0;
	double dGivenP2Vbets = 1.0/dCumProb_KBP2_high_P2;
	for (; nH_P1 >= nIndex_KB_P2; --nH_P1) {
		dEqInc_P1 = 0.5*pdHandRange_P2[nH_P1]*dGivenP2Vbets;
		dPotEquity_P1 -= dEqInc_P1;
		pdHandValues_P1[nH_P1] = dCumProb_low_BKP2_P2*(dPotOnBetCall - GameActions::dBetScalar) + // bluff, always win
									dCumProb_KBP2_high_P2*(dPotOnBetCall*dPotEquity_P1 - GameActions::dBetScalar) + // value-bet, get equity
									dTotalProbCheck_P2; // checks, always win
		dPotEquity_P1 -= dEqInc_P1;
		
	} // (end loop over P1 hands, calling in P2's value-range)

// (B.2.2) Transition into P1's "indifference region"	
	
	// (B.2.2.1) Handle the "delta" for nIndex_KB_P2, part 1:
	
	dEqInc_P1 = 0.5*delta_KBP2*pdHandRange_P2[nH_P1]*dGivenP2Vbets; // (note delta_KBP2 > 0.0, see above)
	dPotEquity_P1 -= dEqInc_P1;		
	
	pdHandValues_P1[nH_P1] = delta_KBP2*
		(dCumProb_low_BKP2_P2*(dPotOnBetCall - GameActions::dBetScalar) + // bluff, always win
											dCumProb_KBP2_high_P2*(dPotOnBetCall*dPotEquity_P1 - GameActions::dBetScalar) + // value-bet, get equity
											dTotalProbCheck_P2); // checks, always win
	//dPotEquity_P1 -= dEqInc_P1; // (don't care since about to reset equity)
	
	// (B.2.2.2) Handle the "1 - delta" for nIndex_KB_P2, part 2:

	dPotEquity_P1 = 1.0; // reset since this is pot equity given a check, ie top of range is missing
	double dGivenP2Checks = 1.0/dTotalProbCheck_P2;
	
	dEqInc_P1 = 0.5*(1.0 - delta_KBP2)*pdHandRange_P2[nH_P1]*dGivenP2Checks;
	dPotEquity_P1 -= dEqInc_P1;		
	
	pdHandValues_P1[nH_P1] += (1.0 - delta_KBP2)*dTotalProbCheck_P2*dPotEquity_P1;
	dPotEquity_P1 -= dEqInc_P1;

	nH_P1--;
	
// (B.2.3) P1 calls in between P2's betting ranges	
		
	for (; nH_P1 >= nIndex_BK_P2; --nH_P1) {
		dEqInc_P1 = 0.5*pdHandRange_P2[nH_P1]*dGivenP2Checks;
		dPotEquity_P1 -= dEqInc_P1;
		pdHandValues_P1[nH_P1] = dTotalProbCheck_P2*dPotEquity_P1;
			// (value is 0 if P2 bets since P1 folds in this range ... or P2 is at best indifferent to calling vs folding ... either way)
		dPotEquity_P1 -= dEqInc_P1;
		
	} // (end loop over P1 hands, folding with equity against checks)	
	
// (B.2.4) Transition from calling to folding	
	
	// (B.2.4.1) Handle the "1 - delta" for nIndex_BK_P2, part 1:
		
	dEqInc_P1 = 0.5*(-delta_BKP2)*pdHandRange_P2[nH_P1]; // (note delta_BKP2 < 0.0, see above)
	dPotEquity_P1 -= dEqInc_P1;		
		
	pdHandValues_P1[nH_P1] = (-delta_BKP2)*dTotalProbCheck_P2*dPotEquity_P1;
	// dPotEquity_P1 -= dEqInc_P1; (don't care, done)
		
	// (B.2.4.2) Handle the delta for nIndex_BK_P2, part 2:

	// (Contribution from folding or checking and losing == 0!)
	
	nH_P1--;

// (B.2.5) P1's folding range	
	
	for (; nH_P1 >= 0; --nH_P1) {
		pdHandValues_P1[nH_P1] = 0.0; // zero equity since P2 bets all weaker hands
		
	}  // (end loop over P1 hands, folding with *no* equity against checks)
	
	if (bDisplay) { 
		printf("(Thresholds: K/B=%.1f B/K=%.1f F/C=%.1f)\n", dExactIndex_KB_P2, dExactIndex_BK_P2, dExactIndex_FC_P1);
		for (int nH = 0; nH < HandStrengths::nHANDS; ++nH) {
			printf("EV(hand=% 3d, freqs=% 3d%%/=% 3d%%) .... P1: %.2f .... P2: %.2f", 
					nH,
					(int)(pdHandRange_P1[nH]*100.00), (int)(pdHandRange_P2[nH]*100.0),  
					pdHandValues_P1[nH], pdHandValues_P2[nH]);
			if ((nH == nIndex_FC_P1)||((nH == (HandStrengths::nHANDS - 1))&&(nIndex_FC_P1 == HandStrengths::nHANDS))) {
				printf(" .... (F/C % 2.2f = % 3.1f%%)", (nIndex_FC_P1 == HandStrengths::nHANDS)?(delta_FCP1 - 1.0):(delta_FCP1), 100.0*dCumProb_FCP1_high_P1);
			}
			if ((nH == nIndex_KB_P2)||((nH == (HandStrengths::nHANDS - 1))&&(nIndex_KB_P2 == HandStrengths::nHANDS))) {
				printf(" .... (K/B % 2.2f = % 3.1f%%)", (nIndex_KB_P2 == HandStrengths::nHANDS)?(delta_KBP2 - 1.0):(delta_KBP2), 100.0*dCumProb_KBP2_high_P2);				
			}
			if ((nH == nIndex_BK_P2)||((nH == (HandStrengths::nHANDS - 1))&&(nIndex_BK_P2 == HandStrengths::nHANDS))) {
				printf(" .... (B/K % 2.2f = % 3.1f%%)", (nIndex_BK_P2 == HandStrengths::nHANDS)?(delta_BKP2 - 1.0):(delta_BKP2), 100.0*dCumProb_low_BKP2_P2);								
			}
			printf("\n");
		}
		printf("\n");
	}
/**/
printf("%d (%.1f): %.2f %.2f (freqs=% 3d%%/=% 3d%%)\n", 
		nIndex_KB_P2, dExactIndex_KB_P2, pdHandValues_P1[12], pdHandValues_P2[12],(int)(pdHandRange_P1[12]*100.00), (int)(pdHandRange_P2[12]*100.0));
//TODO: ok something odd is happening, since at some point, these join a 2-cycle even though the underlying data must be converging??! 
// (also see some stupid indices early, so maybe I do
// need my robustness check->noddy interpolation code for deltas outside of +-2 or indices outside of range...)
	
#endif //__SIMPLESTGAME_NOLEADING
}

//______________________________________________________________________________________________
//______________________________________________________________________________________________

int main(int argc, char** argv)
{
#ifdef __VIRTUAL_STREET_TEST
	double *pdRiverRange_P1 = new double[HandStrengths::nHANDS];
	double *pdRiverRange_P2 = new double[HandStrengths::nHANDS];
	double *pdRiverValue_P1 = new double[HandStrengths::nHANDS];
	double *pdRiverValue_P2 = new double[HandStrengths::nHANDS];
	for (int i = 0; i < HandStrengths::nHANDS; ++i) {
		pdRiverRange_P1[i] = 1.0/HandStrengths::nHANDS;
		pdRiverRange_P2[i] = 1.0/HandStrengths::nHANDS;
	}
	int nItsV3;
	clock_t nTv3 = ::clock();
	for (nItsV3 = 0; nItsV3 < 1; ++nItsV3) {
		::calculateCompleteSingleStreetSolution(pdRiverRange_P1, pdRiverRange_P2, pdRiverValue_P1, pdRiverValue_P2);
	}
	nTv3 = ::clock() - nTv3;
	printf("Iterations = %d, cycles = %d (values: P1 = %.2f, P2 = %.2f)\n\n", nItsV3, (int)nTv3, 0.0, 0.0);
		// (Performance on 1.6GHz, 1000000 iterations with 100 hand buckets in 4686 cycles ~= 4.7s)
		// (Performance on 1.6GHz, 1000000 iterations with *20* hand buckets in 1331 cycles ~= 1.3s)
		// (Performance on 1.6GHz, 1000000 iterations with *10* hand buckets in 1021 cycles ~= 1.0s)
	return 0;
#endif //__VIRTUAL_STREET_TEST
	
	// 0. Stop annoying compile warning
	const char* sz = GameActions::szACTIONS[0];
	*sz++;
	
	// 1. Create a hand distributation (P1 acts first, and in simplest game just KC/KF)
 	
// 13 uniform buckets	
	double pdPreflop_P1[] = { 0.0769, 0.0769, 0.0769, 0.0769, 0.0769, 0.0769, 0.0769, 0.0769, 0.0769, 0.0769, 0.0769, 0.0769, 0.0769 }; 
	double pdPreflop_P2[] = { 0.0769, 0.0769, 0.0769, 0.0769, 0.0769, 0.0769, 0.0769, 0.0769, 0.0769, 0.0769, 0.0769, 0.0769, 0.0769 }; 
// 5 uniform buckets for simplicity
//	double pdPreflop_P1[] = { 0.2, 0.2, 0.2, 0.2, 0.2, 0, 0, 0, 0, 0, 0, 0, 0 }; 
//	double pdPreflop_P2[] = { 0.2, 0.2, 0.2, 0.2, 0.2, 0, 0, 0, 0, 0, 0, 0, 0 };
// 10 uniform buckets: reasonable approximation to [0,1]
//	double pdPreflop_P1[] = { 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0, 0, 0 }; 
//	double pdPreflop_P2[] = { 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1, 0, 0, 0 }; 
// Asymmetric caller/raiser
//	double pdPreflop_P1[] = { 0.156, 0.173, 0.104, 0.000, 0.182, 0.026, 0.039, 0.091, 0.182, 0.026, 0.000, 0.090, 0.013 }; 
//	double pdPreflop_P2[] = { 0.129, 0.138, 0.108, 0.053, 0.152, 0.021, 0.043, 0.093, 0.144, 0.080, 0.000, 0.070, 0.031 };
// Another odd-shaped distro	
//	double pdPreflop_P1[] = { 0.0, 0.2, 0.2, 0.2, 0.2, 0.0, 0.0, 0.0, 0.0, 0.2, 0, 0, 0 }; 
//	double pdPreflop_P2[] = { 0.2, 0.0, 0.0, 0.0, 0.0, 0.2, 0.2, 0.2, 0.2, 0.0, 0, 0, 0 }; 

	P1Strategy *pHandRange_P1 = new P1Strategy[HandStrengths::nHANDS];
	P1Strategy::initializePreflop(pHandRange_P1, pdPreflop_P1);

	P2Strategy *pHandRange_P2 = new P2Strategy[HandStrengths::nHANDS];
	P2Strategy::initializePreflop(pHandRange_P2, pdPreflop_P2);

	// 2. Solve game by iterating through strategies (AFP):

	double dEV_P1, dEV_P2;	
	
	clock_t nT = ::clock();
	int nIts = 1;
	double dAlpha = 0.0;
	static const int nMaxIts = 1000; //100002; //10002
	for (; nIts < nMaxIts; ++nIts) {

		dAlpha = 1.0/(double)nIts;
		
		// Pre-calculations (my stupid "St1 freq" calculations mean need to do it all in one place in the right order)
		Strategy::equityCache.fillEquityCache_P1_St1(pHandRange_P1);
		Strategy::equityCache.fillEquityCache_P2_St1(pHandRange_P2);
		
#ifndef __SINGLE_STREET		
#ifdef __VIRTUAL_TWO_STREET
		for (int i = GameActions::ACTIONS_P1FOLD_ACT + 1; i < GameActions::ACTIONS_NUMACTS_ACT; ++i) {
			::calculateCompleteSingleStreetSolution(
					(const double*)Strategy::equityCache.dHandRange_P1_St2[i], (const double*)Strategy::equityCache.dHandRange_P2_St2[i],
					Strategy::equityCache.dHandValueCache_P1_St2[i], Strategy::equityCache.dHandValueCache_P2_St2[i]);
		}
#else		
		Strategy::equityCache.fillEquityCache_P1_St2(pHandRange_P1);
		Strategy::equityCache.fillEquityCache_P2_St2(pHandRange_P2);
#endif //__VIRTUAL_TWO_STREET
#endif //__SINGLE_STREET		

		// 3. Solve game for P1 given prev P2
		dEV_P1 = Strategy::calculateBRandEV_P1_St1(pHandRange_P1, pHandRange_P2, dAlpha);	
			
		// 4. Solve game for P2 given new P1
		dEV_P2 = Strategy::calculateBRandEV_P2_St1(pHandRange_P2, pHandRange_P1, dAlpha);
		
#ifdef __LOG_CONVERGENCE
		if (0 == (++_nConvIts % _nConvItsPeriodicity)) {
			if (0 == _fpConvergenceLog) {
				_fpConvergenceLog = fopen("conv.log", "w");
			}
			fprintf(_fpConvergenceLog, "%d %f %f\n", _nConvIts, dEV_P1, dEV_P2);
		}		
#endif //__LOG_CONVERGENCE
		
		/**/
		//if (0 == (nIts % 1000))
		if (0)
		{		
			printf("Iterations = %d\n", nIts);	
			P1Strategy::displayStrategy_St1(pHandRange_P1);
			P2Strategy::displayStrategy_St1(pHandRange_P2);
#ifndef __SINGLE_STREET			
			if (0)
			{
				for (int i = GameActions::ACTIONS_KBRF + 1; i < GameActions::ACTIONS_NUMACTS_ACT; ++i) {
					P1Strategy::displayStrategy_St2(pHandRange_P1, i);			
					P2Strategy::displayStrategy_St2(pHandRange_P2, i);
				}
			}
#endif //__SINGLE_STREET			
			int nDummy = scanf("%d", &nDummy);
		}
	} // end loop over iterations
	
	nT = ::clock() - nT;
	
	dEV_P1 = Strategy::calculateBRandEV_P1_St1(pHandRange_P1, pHandRange_P2, dAlpha);	
	dEV_P2 = Strategy::calculateBRandEV_P2_St1(pHandRange_P2, pHandRange_P1, dAlpha);	

	printf("\nIterations = %d, cycles = %d (values: P1 = %.2f, P2 = %.2f)\n\n", nIts, (int)nT, dEV_P1, dEV_P2);	

	P1Strategy::displayStrategy_St1(pHandRange_P1);
	P2Strategy::displayStrategy_St1(pHandRange_P2);

#ifndef __SINGLE_STREET
#ifdef __VIRTUAL_TWO_STREET
	for (int i = GameActions::ACTIONS_P1FOLD_ACT + 1; i < GameActions::ACTIONS_NUMACTS_ACT; ++i) {
		printf("Action: %s\n", GameActions::szACTIONS[i]);
		::calculateCompleteSingleStreetSolution(
				(const double*)Strategy::equityCache.dHandRange_P1_St2[i], (const double*)Strategy::equityCache.dHandRange_P2_St2[i],
				Strategy::equityCache.dHandValueCache_P1_St2[i], Strategy::equityCache.dHandValueCache_P2_St2[i], true);
	}

#else //!__VIRTUAL_TWO_STREET
	if (0)
	{
		for (int i = GameActions::ACTIONS_KBF; i < GameActions::ACTIONS_NUMACTS_ACT; ++i) {
			//if ((i == GameActions::ACTIONS_BC)||(i == GameActions::ACTIONS_BRC))
			{
				if (i > GameActions::ACTIONS_P1FOLD_ACT) {
					P1Strategy::displayStrategy_St2(pHandRange_P1, i);			
					P2Strategy::displayStrategy_St2(pHandRange_P2, i);
				}
			}
		}
	}
#endif //!__VIRTUAL_TWO_STREET
#endif //__SINGLE_STREET
	
#ifdef __LOG_CONVERGENCE
	if (0 != _fpConvergenceLog) {
		fclose(_fpConvergenceLog);
	}
#endif //__LOG_CONVERGENCE

	return 0;
}
