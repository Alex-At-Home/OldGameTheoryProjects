///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
//
// GTO_demo_1
//
///////////////////////////////////////////////////////////////////////////////////////////
//
// Some Game Theory Optimal solutions
//
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

#include <deque>
#include <string>

#include "CBoard.h"
#include "CBoardAnalyzer.h"

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

//TODO: this needs to have 2 modes, one where the flop is specified, and it
// calculates the correct play; and the other where it iterates over the different
// flops in order to calculate the correct pre-flop play

//TODO: talking of pre-flop play, there's also some interesting 3b/4b pre-flop calculations
// to be done here

// In this model, the following scenario occurs:

// 1. Hero raises the pot with "lHeroRange" {POT=3, RAISE=7}
// 2. Villain re-raises with some unknown % of his opening range ("lVillainRange") {POT=10, RAISE=24}
	// (2.1. We'll make it a simple "top N" for the moment, need to polarize it at some point)
// 3. Hero calls with a TBD subset of his range, and checks every flop {CALL=17, POT=51}
	// (3.1. Here, conversely, Hero gets to select bits of his range based on the EV of [4]-[6])
	// (3.2. It is assumed the 4-bet range of Hero has already been removed at the command line)
// 4. Villain continuation pots his entire 3-bet range (obv an approximation) {POT=51, BET=51}
// 5. Hero either folds or shoves with some TBD subset of his range {POT=102,RAISE=204} 
	// (5.1. Initially we're going to assume the shove is a pot-size bet)
// 6. Villain calls ~ optimally (given the [5] range, again an approximation) {CALL=153, POT=306}

void GTO_demo1(std::deque<CCard::Hole>& lHeroRange, std::deque<CCard::Hole>& lVillainRange, int *nBoardTable, int nBoardCards)
{
	// Special values, and corresponding thresholds (unnecessarily taking double inaccuracy into account)
	static const double CONFLICT_ = -2.0;
	static const double CONFLICT_THR_ = -1.5;
	static const double FOLD_HAND_ = -1.0;
	static const double FOLD_HAND_THR_ = -0.5;

	CBoard theBoard;

	double dEquityTable_V[lHeroRange.size()][lVillainRange.size()];
	double dJammingEV_H[lHeroRange.size()];
	double dJammingPct_H[lHeroRange.size()];
	double dCallingPct_V[lVillainRange.size()];
	double dCallingEquity_V[lVillainRange.size()];
	double dJammingPct_sum_H = 0.0, dCallingPct_sum_V = 0.0;
	
	std::deque<CCard::Hole>::iterator itV, itH;
	// Generate a bunch of different flops
	int nFlops = 1; // TODO: calculate this properly
	for (int j = 0; j < nFlops; ++j) {
		
		time_t nCpu = ::clock();
		int nHands = 0, nHandsProc = 0;

		if (0 == nBoardCards) {
			theBoard.generateRandomBoard(nBoardTable, nBoardCards);
		}
		else theBoard.setBoard(nBoardTable, nBoardCards);
		
		CBoardAnalyzer::resetEquityCache();
#if 1					
		printf("BOARD = %s %s %s %s %s\n", 
				CCard(nBoardTable[0]).convertToString().c_str(),
				CCard(nBoardTable[1]).convertToString().c_str(),
				CCard(nBoardTable[2]).convertToString().c_str(),						
				(nBoardCards==4)?",":"",						
				(nBoardCards==4)?(CCard(nBoardTable[3]).convertToString().c_str()):"");						
#endif //0
		
		CBoardAnalyzer theAnalyzer_H, theAnalyzer_V;
		for (int k = 0; k < nBoardCards; ++k) {
			theAnalyzer_H.addCardToBoard(CCard(nBoardTable[k]));
			theAnalyzer_V.addCardToBoard(CCard(nBoardTable[k]));
			//TODO: speed: from addHand (700c/30Kh), assume about 1c/30flops~=330cycles/1Kflops
		}

// Generate equity table		
		
		int nH, nV = 0;
		for (itV = lVillainRange.begin(); itV != lVillainRange.end(); ++itV, ++nV) {
			
			if (!theBoard.isBoardAllowed(*itV)) {
				dCallingEquity_V[nV] = CONFLICT_; // (ignore)
				continue;
			}
			dCallingEquity_V[nV] = 0.0;
			dJammingPct_sum_H = 0.0;
			nH = 0;
			for (itH = lHeroRange.begin(); itH != lHeroRange.end(); ++itH, ++nH) {
			
				if (!theBoard.isBoardAllowed(*itH)) {
					dEquityTable_V[nH][nV] = CONFLICT_; // (ignore)
					// (note: you can't set dJammingEV_H[nH] because it might be V's cards we conflict with)
					continue;
				}

				//TODO: for 30K hand combos, all this "addCardToHand" takes 700 cycles (so need to improve)
				theAnalyzer_H.addHand(*itH);
				theAnalyzer_V.addHand(*itV);
				
				// Now given the flop let's calculate the equity and classify the hands

				++nHands;
				theAnalyzer_H.analyzeBoard();
				theAnalyzer_V.analyzeBoard();
				//TODO: hmmm, don't really need to analyze etc every time, just save off the info
				int nFeatures_H = theAnalyzer_H.getFeatures();
				int nFeatures_V = theAnalyzer_V.getFeatures();
				int nHandVal_H = CBoardAnalyzer::categorizeHand(nFeatures_H);
				int nHandVal_V = CBoardAnalyzer::categorizeHand(nFeatures_V);
				//TODO: for 30K hand combos, takes ~100 cycles

				if ((nHandVal_H <= CBoardAnalyzer::RUBBISH) && (nHandVal_V <= CBoardAnalyzer::RUBBISH)) 
				{
					//TODO: (else no point calculating equity, need to categorize better though)				

					nHandsProc++;
					double dEq_V = CBoardAnalyzer::calculateEquity(*itV, *itH, nBoardTable, nBoardCards);
					//TODO: for 30K hand combos, takes 4.7K cycles (sample mode, 100 samples: 950 cycles) (so need to improve)
					//double dEq_V = CBoardAnalyzer::approximateEquity(theAnalyzer_H, theAnalyzer_V, *itV, *itH, nBoardTable, nBoardCards);

					dEquityTable_V[nH][nV] = dEq_V;

					if (nHandVal_H <= CBoardAnalyzer::MEDIOCRE_MADE) {
						// (give H some sort of respectable initial jamming range)
						dJammingPct_H[nH] = 1.0;
						dCallingEquity_V[nV] += dEq_V;
						dJammingPct_sum_H += 1.0; 
					}
					else dEquityTable_V[nH][nV] = FOLD_HAND_; 
						// prevents dominated strategy of jamming with no hand at all
						// (just jam with better hands more often, duh!)
					
#if 0					
						printf("% 6d/% 6d: %s%s (%s) vs %s%s (%s) on %s%s%s: %.2g\n", nPlayed, nIterations,
								itH->first.convertToString().c_str(),itH->second.convertToString().c_str(),
								CBoardAnalyzer::_pcHandSummaryTable[nHandVal_H],
								itV->first.convertToString().c_str(),itV->second.convertToString().c_str(),
								CBoardAnalyzer::_pcHandSummaryTable[nHandVal_V],
								CCard(nBoardTable[0]).convertToString().c_str(),
								CCard(nBoardTable[1]).convertToString().c_str(),
								CCard(nBoardTable[2]).convertToString().c_str(),
								100.0*dEq);
#endif //0					
						
				}				
				else {
					if (nHandVal_V > CBoardAnalyzer::RUBBISH) { // V has a weak hand, so will always fold to a craise
				
						dCallingPct_V[nV] = 0.0;
						dCallingEquity_V[nV] = FOLD_HAND_; // (V fold due to hand strength)
						//TODO: this means you don't need to calculate equity for other {H, this V} pairs! 
					}
					if (nHandVal_H > CBoardAnalyzer::RUBBISH) { // H has a weak hand, make sure he'll never jam

						dJammingPct_H[nH] = 0.0;
						dEquityTable_V[nH][nV] = FOLD_HAND_; // (H fold due to hand strength)
						//TODO: this means you don't need to calculate equity for other {this H, V} pairs!
					}
				}
								
				theBoard.resetBoard(*itH);
			} // (end loop over H range)

			if (dCallingPct_V[nV] > FOLD_HAND_THR_) { // (not a conflict/hand strength)
				if (dJammingPct_sum_H > 0.0) {
					dCallingEquity_V[nV] /= dJammingPct_sum_H;
					if (dCallingEquity_V[nV] > 0.333) {
						dCallingPct_V[nV] = 1.0;
					}
					else dCallingPct_V[nV] = 0.0;
				}
				else { //(doesn't matter either way, H never jams)
					dCallingPct_V[nV] = 0.0;
					dCallingEquity_V[nV] = 0.0;
				}
			}
						
			theBoard.resetBoard(*itV);
		} // (end loop over V range)				

		nCpu = ::clock() - nCpu;
		printf("(took %d cycles for %d hands (%d processed))\n", 
				(int)nCpu, nHands, nHandsProc);
		
// Calculate optimal ranges for Hero and Villain		

		int nIterations = 0;
		static const int nMaxIts = 101;
		double dMeanAbsChange = 0.0; 
		double dMaxAbsChange = 0.0;
		// When Hero raises....
		// Hero is putting 204 units in to win 102 [5]
		// Villain is faced with a call of 153 to win 306 [4/5] 
		// V defense starts with all hands with >= 0.33 equity vs H's starting range
		for (;nIterations < nMaxIts; ++nIterations) {

			double dAlpha = 1.0/(1 + nIterations);
			dMeanAbsChange = 0.0; 
			dMaxAbsChange = 0.0;
			
			// So, which hands should H jam with against V's defense

			double dJamEv_H = 0.0;
			int nValidCombos_H = 0;
			int nTotalHands_H = 0;
			dJammingPct_sum_H = 0.0;
			
			// Work out which hands to jam with H
			for (nH = 0; nH < (int)lHeroRange.size(); ++nH) {

				// The cases are:
				// (i)   H has a bad hand, so will fold it. (H eq 0.0)
				// (ii)  hand conflicts, just don't count (dEq_HV==-2.0 || dEq_V==-2.0)
				// (iii) H raises, V folds (dEq_V==-1.0, H eq 102)
				// (iv)  H raises, V calls, H gets his equity (H eq 204 - dEq_V*408)
				
				double dJamEv_hand_H = 0.0;
				int nValidCombos_hand_H = 0;
				
				dJammingEV_H[nH] = 0.0;

				for (nV = 0; nV < (int)lVillainRange.size(); ++nV) {
					
					if (dCallingEquity_V[nV] < CONFLICT_THR_) continue; // (ii) conflict

					double dEq_V = dEquityTable_V[nH][nV];

					if (dEq_V < CONFLICT_THR_) continue; // (ii) conflict

					nValidCombos_hand_H++;

					if (dEq_V < FOLD_HAND_THR_) { // (i) H c-folds
						//dJamEv_H_hand += 0;
					}
					else if (dCallingEquity_V[nV] < FOLD_HAND_THR_) { // (iii) V b-folds (hand strength)
						dJamEv_hand_H += 102;
					}
					else { // (iv) V b-calls some %
						dJamEv_hand_H += 102.0*(1.0 - dCallingPct_V[nV]); // V folds
						dJamEv_hand_H += dCallingPct_V[nV]*(-204.0 + (1.0 - dEq_V)*459.0);
							// (when called, put in $204, get back equity in full pot size of $459)
					}
					
				} // end loop over V range

				if (nValidCombos_hand_H > 0) {
					
					nValidCombos_H += nValidCombos_hand_H;
					double dNewJamPct_H;
					if (dJamEv_hand_H > 0.0) {								
						dNewJamPct_H = dAlpha + (1.0 - dAlpha)*dJammingPct_H[nH]; // (add a weighted jam)
						dMeanAbsChange += dNewJamPct_H - dJammingPct_H[nH]; 
						if ((dNewJamPct_H - dJammingPct_H[nH]) > dMaxAbsChange) {
							dMaxAbsChange = dNewJamPct_H - dJammingPct_H[nH];							
						}
					}
					else {
						dNewJamPct_H = (1.0 - dAlpha)*dJammingPct_H[nH]; // (add a weighted fold)
						dMeanAbsChange -= dNewJamPct_H - dJammingPct_H[nH]; 
						if ((dJammingPct_H[nH] - dNewJamPct_H) > dMaxAbsChange) {
							dMaxAbsChange = dJammingPct_H[nH] - dNewJamPct_H;							
						}
					}
					dJammingPct_H[nH] = dNewJamPct_H;
					dJammingEV_H[nH] = dJamEv_hand_H/nValidCombos_hand_H; // (make this the EV of when we jam, ignore c/folds)
					dJamEv_H += dJamEv_hand_H*dJammingPct_H[nH]; // (don't need to worry about freq % as it's 0EV if H c/folds)
					dJammingPct_sum_H += dJammingPct_H[nH];

					nTotalHands_H++;
				}
				else dJammingPct_H[nH] = CONFLICT_; // conflict
				
			} // end loop over H range
			
			dJamEv_H /= nValidCombos_H; // (don't need to worry about freq % as it's 0EV if H c/folds) 

			// Modify V's defense range in light of H's new range

			dCallingPct_sum_V = 0.0;
			int nTotalHands_V = 0;
			int nV = 0;
			double dCallingEV_V = 0.0;
			for (itV = lVillainRange.begin(); itV != lVillainRange.end(); ++itV, ++nV) {
				
				if (dCallingEquity_V[nV] > CONFLICT_THR_) { // not a conflict 

					nTotalHands_V++;
					dCallingEV_V -= 51.0; // V's bet

					dCallingEquity_V[nV] = 0.0; // (reset)
					dJammingPct_sum_H = 0.0;
					int nJamHands_H_nV = 0;
					
					for (int nH = 0; nH < (int)lHeroRange.size(); ++nH) {
						
						double dEq_V = dEquityTable_V[nH][nV];

						if (dEq_V > CONFLICT_THR_) { // not a conflict
							dCallingEquity_V[nV] += dEq_V*dJammingPct_H[nH]; // (this might be meaningless if H always folds...)
							dJammingPct_sum_H += dJammingPct_H[nH]; // (... but this isn't)
							nJamHands_H_nV++;
						} // end not a conflict
					} // end loop over H range				
					
					if (dCallingEquity_V[nV] < FOLD_HAND_THR_) {

						dCallingPct_V[nV] = 0.0; // (don't need to worry about alpha here, it's a hand strength thing)
						dCallingEquity_V[nV] = 0.0;

						double dJamPct_H_nV = dJammingPct_sum_H/nJamHands_H_nV;						
						dCallingEV_V += (1.0 - dJamPct_H_nV)*102.0; // (always folds if H jams)
					}
					if (dJammingPct_sum_H > 0.0) {
						dCallingEquity_V[nV] /= dJammingPct_sum_H; 
						double dNewCallPct_V;
						if (dCallingEquity_V[nV] > 0.333) {
							dNewCallPct_V = dAlpha + (1.0 - dAlpha)*dCallingPct_V[nV]; // (add a weighted call)
							dMeanAbsChange += dNewCallPct_V - dCallingPct_V[nV]; 
							if ((dNewCallPct_V - dCallingPct_V[nV]) > dMaxAbsChange) {
								dMaxAbsChange = dNewCallPct_V - dCallingPct_V[nV];							
							}
						}
						else {
							dNewCallPct_V = (1.0 - dAlpha)*dCallingPct_V[nV]; // (add a weighted fold)
							dMeanAbsChange -= dNewCallPct_V - dCallingPct_V[nV]; 
							if ((dCallingPct_V[nV] - dNewCallPct_V) > dMaxAbsChange) {
								dMaxAbsChange = dCallingPct_V[nV] - dNewCallPct_V;							
							}
						}
						dCallingPct_V[nV] = dNewCallPct_V;
						dCallingPct_sum_V += dCallingPct_V[nV]; 
						
						double dJamPct_H_nV = dJammingPct_sum_H/nJamHands_H_nV;						
						dCallingEV_V += dJamPct_H_nV*dCallingPct_V[nV]*(-153.0 + dCallingEquity_V[nV]*459.0);
						dCallingEV_V += (1.0 - dJamPct_H_nV)*102.0;
					}
					else { // (doesn't really matter, H never jams)
						dCallingPct_V[nV] = 0.0;
						dCallingEquity_V[nV] = 0.0;
						dCallingEV_V += 102.0; // V gets the pot 
					}
				}
								
			} // end loop over V range		
			
			dCallingEV_V /= nTotalHands_V;
			dMeanAbsChange /= (nTotalHands_V + nTotalHands_H);			
#if 1
			if (nMaxIts == (nIterations + 1)) {
				// Let's print some stuff out			
				nH = 0;
				printf("Hero range (%d): %.2f EV (%.0f%%)\n", nIterations, dJamEv_H, 100.0*dJammingPct_sum_H/nTotalHands_H);
				
				for (itH = lHeroRange.begin(); itH != lHeroRange.end(); ++itH, ++nH) {
					if (0 == (nH % 4)) printf("\n");
					
					if (dJammingPct_H[nH] >= 0.001) { // jam some % of the time
						printf("%s%s: %.1f EV %.0f%%\t",
								itH->first.convertToString().c_str(),itH->second.convertToString().c_str(),
								dJammingEV_H[nH], 100.0*dJammingPct_H[nH]);
					}
					else if (dJammingPct_H[nH] > CONFLICT_THR_) { // fold, not conflict
						printf("(%s%s: %.1f EV)%s\t", itH->first.convertToString().c_str(),itH->second.convertToString().c_str(),
								dJammingEV_H[nH], (dJammingEV_H[nH]>-10.0)?"\t":"");
					}
					else { // conflict
						printf("(----)\t\t\t");					
					}
				} // end loop over H range
				printf("\n\n");
			}
#endif //1

#if 1
			if (nMaxIts == (nIterations + 1)) {
				// Let's print some stuff out			
				nV = 0;
				printf("Villain range (%d): %.2f EV (%.0f%%)\n", nIterations, dCallingEV_V, 100.0*dCallingPct_sum_V/nTotalHands_V);
				for (itV = lVillainRange.begin(); itV != lVillainRange.end(); ++itV, ++nV) {
					if (0 == (nV % 4)) printf("\n");
	
					if (dCallingPct_V[nV] > 0.001) { // call
						printf("%s%s: %.2f Eq %.0f%%\t",
								itV->first.convertToString().c_str(),itV->second.convertToString().c_str(),
								dCallingEquity_V[nV], 100.0*dCallingPct_V[nV]);
					}
					else if (dCallingEquity_V[nV] > CONFLICT_THR_) { // fold, not conflict
						printf("(%s%s: %.2f)\t\t", itV->first.convertToString().c_str(),itV->second.convertToString().c_str(),
								dCallingEquity_V[nV]);
					}
					else { // conflict
						printf("(----)\t\t\t");					
					}
				} // end loop over V range
				printf("\n\n");
			}
#endif //1
			
		} // end loop over iterative optimal play algorithm
		
		nCpu = ::clock() - nCpu;
		printf("(took %d cycles for %d iterations, (deltas = %.2f%% mean %.2f%% max))\n", 
				(int)nCpu, nIterations, 100.0*dMeanAbsChange, 100.0*dMaxAbsChange);

	} // (end loop over random flops)

	//TODO: hmmm, seems to take 5 secs/flop (all in calculateEquity).
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

