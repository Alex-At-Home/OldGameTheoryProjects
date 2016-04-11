////////////////////////////////////////////////////

#include <string>
#include <map>
#include <list>
#include <deque>

#include <string.h>

//#define __UNIT_TEST

#include "CBoardAnalyzer.h"

//TODO: can I tidy this "main()" up a bit, eg into a few functions?

////////////////////////////////////////////////////
////////////////////////////////////////////////////

// Some other uses

// 3b GTO simulation assuming 3-bettor always c-bets
extern void GTO_demo1(std::deque<CCard::Hole>& lHeroRange, std::deque<CCard::Hole>& lVillainRange, int *nBoardTable, int nBoardCards);
// GTO [KC|KF]v[B|K] simulation from the turn
extern void GTO_demo2(std::deque<CCard::Hole>& lHeroRange, std::deque<CCard::Hole>& lVillainRange, int *nBoardTable);
// GTO [4b/F|4b/C|4bai|F] v [5b|F] pre-flop simulation after 3b
extern void GTO_demo_preflop(std::deque<CCard::Hole>& lHeroRange, std::deque<CCard::Hole>& lVillainRange);

////////////////////////////////////////////////////
////////////////////////////////////////////////////

#ifdef __UNIT_TEST
	extern void do_unit_test(); 
#endif //__UNIT_TEST

static void print_usage()
{
	printf("\nBoardAnalyzer\n\t"
				"[--myrank <min-hand-%%>[,<max-hand-%%>]|--myrange <hand-range>}]\n\t"
				"[--flop <3-or-4-card-board>|--numflops <number-of-random-flops>]\n\t"
				" --vsrank <min-hand-%%>[,<max-hand-%%>]|--vsrange <hand-range>}]\n\t"
				"[--faction <flop-range-name>[,<my-flop-range-name>]\n\t"
				"[--GTO 3bet|turn|preflop]\n\t"
				"[--olddisp]\n\t\n\n"
			);
	printf("Pre-allocated pre-flop ranges:\t%s\n", CBoardAnalyzer::DisplayPreFlopRanges().c_str());
	printf("Pre-allocated flop ranges:\t%s\n\n", CBoardAnalyzer::DisplayPostFlopRanges().c_str());
}
	
int main(int argc, char** argv)
{
#ifdef __UNIT_TEST
	::do_unit_test();
	return 0;
#endif //__UNIT_TEST

	if (argc < 2) {
		::print_usage();
		return 0;
	}
	bool bNewDisplay = true;
	
// 1. Command line parsing:
	
	std::map<std::string, std::string> mParameters;	
	std::string sParamName;
	std::string sParamValue;
	
	for (int arg = 1; arg < argc; ++arg) {
		if ((strlen(argv[arg]) > 2) && (argv[arg][0] == '-') && (argv[arg][1] == '-')) {
			if (sParamName.length() > 0) {
				mParameters[sParamName] = sParamValue;
			}
			sParamValue.clear();
			sParamName = argv[arg];
		}
		else {
			if (sParamValue.length() > 0) sParamValue += " , ";
			sParamValue += argv[arg]; 
		}
	}
	if (sParamName.length() > 0) {		
		mParameters[sParamName] = sParamValue;
	}

// 1.1 Application-specific command line arg parsing	
	
	std::deque<CCard::Hole> lMyHandRange;
	if (mParameters["--myrange"].length() > 0) {
		std::string sRange = mParameters["--myrange"]; 
		
		CBoardAnalyzer::assignHandFromRange(sRange, sRange);

		double dRunningFreq = 0.0;
		for (int n = 0; n < (int)sRange.length(); ) {
			n+= CCard::addHand(sRange.c_str() + n, lMyHandRange, dRunningFreq);
		}
		int nPct = (int)(dRunningFreq*100.0)/(26*51);
		printf("BoardAnalyzer: MY pre-flop hand range (%d%%) = %s\n\n", nPct, sRange.c_str());
	}
	else if (mParameters["--myrank"].length() > 0) {
		int nPct, nPct_max = 0;
		std::string s = mParameters["--myrank"];
		if (2 != sscanf(s.c_str(), " %d , %d ", &nPct, &nPct_max)) {
			if (0 == sscanf(s.c_str(), " %d ", &nPct)) {
				printf("BoardAnalyzer: Failed to parse --myrank %s\n", mParameters["--myrank"].c_str());
				::print_usage();
				return -1;
			}
			nPct_max = 0; // (just in case)
		}
		std::string sHandList;
		int nActualPct = CBoardAnalyzer::assignHandFromRange(nPct, nPct_max, lMyHandRange, sHandList);
		printf("BoardAnalyzer: MY pre-flop hand range (%d%%) = %s\n\n", nActualPct, sHandList.c_str());
	}
	std::deque<CCard::Hole> lVsHandRange;
	if (mParameters["--vsrange"].length() > 0) {
		std::string sRange = mParameters["--vsrange"]; 

		CBoardAnalyzer::assignHandFromRange(sRange, sRange);

		double dRunningFreq = 0.0;
		for (int n = 0; n < (int)sRange.length(); ) {
			n+= CCard::addHand(sRange.c_str() + n, lVsHandRange, dRunningFreq);
		}
		int nPct = (int)(dRunningFreq*100.0)/(26*51);
		printf("BoardAnalyzer: VS pre-flop hand range (%d%%) = %s\n\n", nPct, sRange.c_str());
	}
	else if (mParameters["--vsrank"].length() > 0) {
		int nPct, nPct_max = 0;
		std::string s = mParameters["--vsrank"];
		if (2 != sscanf(s.c_str(), " %d , %d ", &nPct, &nPct_max)) {
			if (0 == sscanf(s.c_str(), " %d ", &nPct)) {
				printf("BoardAnalyzer: Failed to parse --vsrank %s\n", mParameters["--vsrank"].c_str());
				::print_usage();
				return -1;
			}
			nPct_max = 0; // (just in case)
		}
		std::string sHandList;
		int nActualPct = CBoardAnalyzer::assignHandFromRange(nPct, nPct_max, lVsHandRange, sHandList);
		printf("BoardAnalyzer: VS pre-flop hand range (%d%%) = %s\n\n", nActualPct, sHandList.c_str());
	}
	else {
		printf("BoardAnalyzer: Currently requires \"vs range\" or \"vs rank\" to be specified");
		::print_usage();
		return -1;
	}
	
	int nFlopLen = 0;
	int nBoardTable[5] = { -1 };
	int nBoardCards = 0;
	const char *szFlop;
	if (mParameters["--flop"].length() > 0) {
		std::string sFlop = mParameters["--flop"];
		szFlop = sFlop.c_str();
		nFlopLen = sFlop.length();

		int nf = 0;
		for (; nf < nFlopLen;) {
			
			CCard newCard;
			int n = newCard.set(szFlop + nf);
			if (0 == newCard) {
				printf("BoardAnalyzer: Error 1 at %d = %d (%s)\n", nf, (int)newCard, szFlop+nf);					
				::print_usage();
				return -1;
			}
			nBoardTable[nBoardCards++] = (int)newCard;
			nf += n;
		}			
	}

	std::string sAction;
	std::string sMyAction;
	if (mParameters["--faction"].length() > 0) {
		char szBuff1[256], szBuff2[256];
		
		std::string s = mParameters["--faction"];

		if (2 == sscanf(s.c_str(), "%s , %s", szBuff1, szBuff2)) {
			sAction = szBuff1;
			sMyAction = szBuff2;
		}
		else {
			sAction = s;
		}
	}
	if (mParameters.end() != mParameters.find("--olddisp")) {
		bNewDisplay = false;
	}

// 2. GTO demos
	
	if (mParameters["--GTO"].length() > 0) {
		std::string s = mParameters["--GTO"];
		if (s == "3bet") {
			::GTO_demo1(lMyHandRange, lVsHandRange, nBoardTable, nBoardCards);			
			return 0;
		}
		else if (s == "turn") {
			if (nBoardCards >= 4) {
				::GTO_demo2(lMyHandRange, lVsHandRange, nBoardTable);			
				return 0;
			}
			else {
				printf("BoardAnalyzer: GTO 2 requires flop");
				return -1;
			}
		}
		else if (s == "preflop") {
			::GTO_demo_preflop(lMyHandRange, lVsHandRange);			
			return 0;
		}
		else {
			::print_usage();
			printf("BoardAnalyzer: toy game not supported yet.\n");
			return -1;
		}
	}
	
	// (Some more error checking)
	
	if (0 == nFlopLen) {

		//TODO: allow distribution of flops
		printf("BoardAnalyzer: Currently require flop to be specified");
		::print_usage();
		return -1;
	}

// (Some initialization)	
	
	std::deque<CCard::Hole>::iterator it, itMe;	
	double nBoardsAnalyzed = 0, nBoardsAnalyzed_me = 0;
	std::map<int, double> mFeatures[1 + CBoardAnalyzer::RUBBISH];
	std::map<int, double> mFeatures_me[1 + CBoardAnalyzer::RUBBISH];
	std::map<int, double> dmEquity[1 + CBoardAnalyzer::RUBBISH];
	double dEquityTable[1 + CBoardAnalyzer::RUBBISH] = { 0.0 };

	std::map<int, double> mStrengthThenDynamism[CBoardAnalyzer::STATIC_MAX];
	std::map<int, double> mStrengthThenDynamism_me[CBoardAnalyzer::STATIC_MAX];
	
	bool bCalcEquity = false;
	bool bRangeVsRange = false;
	if (lMyHandRange.empty()) {
		lMyHandRange.push_back(CCard::Hole(0, 0));
	}
	else if (lMyHandRange.size() > 1){ // Going to process a range for "my" hands
		bRangeVsRange = true;
	}
	else {
		bCalcEquity = true;
	}
	
// (Loop over "my" hands)	

	for (itMe = lMyHandRange.begin(); itMe != lMyHandRange.end(); ++itMe) {

		if (0 != itMe->first) { // Calculate parameters for "my" hand...

			double dFreq = itMe->dFreq;
			CBoardAnalyzer meAnalyzer;
			int nBoardCards_touse = nBoardCards;
			if  (0 == sMyAction.length()) nBoardCards_touse = 3;

			for (int nf = 0; nf < nBoardCards_touse; ++nf) {
				if (!meAnalyzer.addCardToBoard(nBoardTable[nf])) { 
					printf("BoardAnalyzer: Error with board card %d\n", (int)nBoardTable[nf]);
					return -1;
				}
			}
			if (meAnalyzer.addHand(*itMe)) {
				
				meAnalyzer.analyzeBoard();
				int nFeatures = meAnalyzer.getFeatures();
				int nIndex = CBoardAnalyzer::categorizeHand(nFeatures);
				
// 4. (Turn action) Eliminate hands on flop, deal turn, etc

				if  (sMyAction.length() > 0) {
					double dProb = CBoardAnalyzer::GetHandRangeWeighting(sMyAction, nIndex);

					if (dProb < -0.5) continue;
					else dFreq *= dProb;

					// OK if we're here, then deal the turn
					if (nBoardCards_touse < nBoardCards) {
						
						// Put the hands back in again (call to analyze removes them)
						if (!meAnalyzer.addHand(*itMe)) {
							printf("BoardAnalyzer: Error with hole cards %d %d\n", (int)itMe->first, (int)itMe->second);
							return -1;
						}
						
						if (!meAnalyzer.addCardToBoard(nBoardTable[nBoardCards_touse])) { // That's OK, this is a combo we don't need to analyse
							continue;
						}
						meAnalyzer.analyzeBoard();
						nFeatures = meAnalyzer.getFeatures();
						nIndex = CBoardAnalyzer::categorizeHand(nFeatures);
					}				
				} // end if flop action specified					
				
				nBoardsAnalyzed_me += dFreq;
				mFeatures_me[nIndex][nFeatures] += dFreq;
				
				// Better display categorization, separating out draws and hands
				mStrengthThenDynamism_me[meAnalyzer.calculateHandStrength(nFeatures)][nFeatures] += dFreq;
					// (classed by feature)
				mStrengthThenDynamism_me[meAnalyzer.calculateHandStrength(nFeatures)][-1] += dFreq;										
					// (across all features)
					
			} // (else that's OK, it's a non-existent combo)
		} // my hand exists

// (Loop over "villain" hands)		
		
		for (it = lVsHandRange.begin(); it != lVsHandRange.end(); ++it) {

			CBoardAnalyzer theAnalyzer;
			if ((itMe->first != it->first)&&(itMe->first != it->second)&&(itMe->second != it->first)&&(itMe->second != it->second)) {
				
				if (!theAnalyzer.addHand(*it)) {
					printf("BoardAnalyzer: Error with hole cards %d %d\n", (int)it->first, (int)it->second);
					return -1;
				}

// 3. Sort out flop/turn
				
				int nf = 0;
				int nBoardCards_touse = nBoardCards;
				if  (0 == sAction.length()) nBoardCards_touse = 3;
				
				for (; nf < nBoardCards_touse; ++nf) {
					if (!theAnalyzer.addCardToBoard(nBoardTable[nf])) { // That's OK, this is a combo we don't need to analyse
						break;
					}
				}				
				if (nf == nBoardCards_touse) {
					theAnalyzer.analyzeBoard();
					int nFeatures = theAnalyzer.getFeatures();
					int nIndex = CBoardAnalyzer::categorizeHand(nFeatures);
					
// 4. (Turn action) Eliminate hands on flop, deal turn, etc

					double dFreq = it->dFreq;
					if  (sAction.length() > 0) {
						double dProb = CBoardAnalyzer::GetHandRangeWeighting(sAction, nIndex);

						if (dProb < -0.5) continue;
						else dFreq *= dProb;

						// OK if we're here, then deal the turn
						if (nBoardCards_touse < nBoardCards) {
							
							// Put the hands back in again (call to analyze removes them)
							if (!theAnalyzer.addHand(*it)) {
								printf("BoardAnalyzer: Error with hole cards %d %d\n", (int)it->first, (int)it->second);
								return -1;
							}
							
							if (!theAnalyzer.addCardToBoard(nBoardTable[nBoardCards_touse])) { // That's OK, this is a combo we don't need to analyse
								continue;
							}
							theAnalyzer.analyzeBoard();
							nFeatures = theAnalyzer.getFeatures();
							nIndex = CBoardAnalyzer::categorizeHand(nFeatures);
						}				
					} // end if flop action specified					

// 5. Save off hand for later analysis					
					
					nBoardsAnalyzed += dFreq;
					mFeatures[nIndex][nFeatures] += dFreq;
					
					// Better display categorization, separating out draws and hands
					mStrengthThenDynamism[theAnalyzer.calculateHandStrength(nFeatures)][nFeatures] += dFreq;
						// (classed by feature)
					mStrengthThenDynamism[theAnalyzer.calculateHandStrength(nFeatures)][-1] += dFreq;										
						// (across all features)
					
					// (Calculate and save the equity)
					
					if (bCalcEquity) {
						double dEV = CBoardAnalyzer::calculateEquity(*it, *itMe, nBoardTable, nBoardCards);
						dmEquity[nIndex][nFeatures] += dEV*dFreq;
						dEquityTable[nIndex] += dEV*dFreq;
					}
					//TODO: calculate equity when you're against a range.
					
				} // end if dealt the flop (or turn if no flop action specified)
			} // end if there's no conflict between hands 
		} // end loop over villain hands
	} // end loop over my hands

// 6. Display hand	

	if (bNewDisplay) { // This new display orders first by hand strength, then adds draw information		
		int nCurrCategory = 0;
		double nTotalCount_cat = 0, nTotalCount_cat_dry = 0, nTotalCount_cat_fl = 0, nTotalCount_cat_str4 = 0, nTotalCount_cat_str8 = 0,
				nTotalCount_cat_str4fl = 0, nTotalCount_cat_str8fl = 0;
		double nTotalCount_cat_me = 0, nTotalCount_cat_dry_me = 0, nTotalCount_cat_fl_me = 0, nTotalCount_cat_str4_me = 0, nTotalCount_cat_str8_me = 0,
				nTotalCount_cat_str4fl_me = 0, nTotalCount_cat_str8fl_me = 0;
		
		printf("VS\t DRY  /  4SD   8SD  /   FD   4SFD  8SFD = TOT)\t\t");
		if (bRangeVsRange) {
			printf("ME\t DRY  /  4SD   8SD  /   FD   4SFD  8SFD = TOT");
		}
		printf("\n\n");
		for (int i = 0; i < CBoardAnalyzer::STATIC_MAX; ++i) {

			if (i == CBoardAnalyzer::_nHandCategoryTable[nCurrCategory]) {
				if (i > 0) {
					printf("(T:\t%4.1f%% / %4.1f%% %4.1f%% / %4.1f%% %4.1f%% %4.1f%% = %4.1f%%)\t", 
							(100.0*nTotalCount_cat_dry)/nBoardsAnalyzed, 
							(100.0*nTotalCount_cat_str4)/nBoardsAnalyzed, (100.0*nTotalCount_cat_str8)/nBoardsAnalyzed,
							(100.0*nTotalCount_cat_fl)/nBoardsAnalyzed, (100.0*nTotalCount_cat_str4fl)/nBoardsAnalyzed, 
							(100.0*nTotalCount_cat_str8fl)/nBoardsAnalyzed, (100.0*nTotalCount_cat)/nBoardsAnalyzed);
				}
				if (bRangeVsRange) {
					if (i > 0) {
						printf("(T:\t%4.1f%% / %4.1f%% %4.1f%% / %4.1f%% %4.1f%% %4.1f%% = %4.1f%%)", 
								(100.0*nTotalCount_cat_dry_me)/nBoardsAnalyzed_me, 
								(100.0*nTotalCount_cat_str4_me)/nBoardsAnalyzed_me, (100.0*nTotalCount_cat_str8_me)/nBoardsAnalyzed_me,
								(100.0*nTotalCount_cat_fl_me)/nBoardsAnalyzed_me, (100.0*nTotalCount_cat_str4fl_me)/nBoardsAnalyzed_me, 
								(100.0*nTotalCount_cat_str8fl_me)/nBoardsAnalyzed_me, (100.0*nTotalCount_cat_me)/nBoardsAnalyzed_me);
					}
					nTotalCount_cat_me = 0; nTotalCount_cat_dry_me = 0; nTotalCount_cat_fl_me = 0; nTotalCount_cat_str4_me = 0;
					nTotalCount_cat_str8_me = 0; nTotalCount_cat_str4fl_me = 0; nTotalCount_cat_str8fl_me = 0;
				}
				if (i > 0) {
					printf("\n\n");
					printf("------%s\n", CBoardAnalyzer::_szHandCategoryTable[nCurrCategory]);
				}
				nCurrCategory++;
				nTotalCount_cat = 0; nTotalCount_cat_dry = 0; nTotalCount_cat_fl = 0; nTotalCount_cat_str4 = 0;
				nTotalCount_cat_str8 = 0; nTotalCount_cat_str4fl = 0; nTotalCount_cat_str8fl = 0;
			}			
			std::map<int, double>::iterator mit;
			std::map<int, double>& m = mStrengthThenDynamism[i];
			double nTotHands = 0, nDryHands, nFlHands = 0, nStr8FlHands = 0, nStr4FlHands = 0, nStr8Hands = 0, nStr4Hands = 0;
			for (mit = m.begin(); mit != m.end(); ++mit) {
				int n = mit->first;
				if (-1 == n) {
					nTotHands = mit->second;
				}
				else if (n & CBoardAnalyzer::FLUSH_DRAW){
					if (n & CBoardAnalyzer::GUTSHOT) nStr4FlHands += mit->second;
					else if (n & CBoardAnalyzer::ANY_STR8_POSSIBILITIES) nStr8FlHands += mit->second;
					else nFlHands += mit->second;					
				}
				else if (n & CBoardAnalyzer::GUTSHOT) nStr4Hands += mit->second;					
				else if (n & CBoardAnalyzer::ANY_STR8_POSSIBILITIES) nStr8Hands += mit->second;
			}
			nDryHands = nTotHands - (nFlHands + nStr8FlHands + nStr4FlHands + nStr4Hands + nStr8Hands);
			nTotalCount_cat += nTotHands;
			nTotalCount_cat_dry += nDryHands;
			nTotalCount_cat_fl += nFlHands; 
			nTotalCount_cat_str8fl += nStr8FlHands; 
			nTotalCount_cat_str4fl += nStr4FlHands; 
			nTotalCount_cat_str8 += nStr8Hands; 
			nTotalCount_cat_str4 += nStr4Hands; 
			
			printf("% 2d\t%4.1f%% / %4.1f%% %4.1f%% / %4.1f%% %4.1f%% %4.1f%% = %4.1f%%\t\t", i, 
					(100.0*nDryHands)/nBoardsAnalyzed, 
					(100.0*nStr4Hands)/nBoardsAnalyzed, (100.0*nStr8Hands)/nBoardsAnalyzed,
					(100.0*nFlHands)/nBoardsAnalyzed, (100.0*nStr4FlHands)/nBoardsAnalyzed, (100.0*nStr8FlHands)/nBoardsAnalyzed
					,(100.0*nTotHands)/nBoardsAnalyzed
					);
			
			if (bRangeVsRange) {
				std::map<int, double>::iterator mit_me;
				std::map<int, double>& m = mStrengthThenDynamism_me[i];
				double nTotHands_me = 0, nDryHands_me, nFlHands_me = 0, nStr8FlHands_me = 0, nStr4FlHands_me = 0, nStr8Hands_me = 0, nStr4Hands_me = 0;
				for (mit_me = m.begin(); mit_me != m.end(); ++mit_me) {
					int n = mit_me->first;
					if (-1 == n) {
						nTotHands_me = mit_me->second;
					}
					else if (n & CBoardAnalyzer::FLUSH_DRAW){
						if (n & CBoardAnalyzer::GUTSHOT) nStr4FlHands_me += mit_me->second;
						else if (n & CBoardAnalyzer::ANY_STR8_POSSIBILITIES) nStr8FlHands_me += mit_me->second;
						else nFlHands_me += mit_me->second;					
					}
					else {
						if (n & CBoardAnalyzer::GUTSHOT) nStr4Hands_me += mit_me->second;					
						else if (n & CBoardAnalyzer::ANY_STR8_POSSIBILITIES) nStr8Hands_me += mit_me->second;
					}
				}
				nDryHands_me = nTotHands_me - (nFlHands_me + nStr8FlHands_me + nStr4FlHands_me + nStr4Hands_me + nStr8Hands_me);
				nTotalCount_cat_me += nTotHands_me;
				nTotalCount_cat_dry_me += nDryHands_me;
				nTotalCount_cat_fl_me += nFlHands_me; 
				nTotalCount_cat_str8fl_me += nStr8FlHands_me; 
				nTotalCount_cat_str4fl_me += nStr4FlHands_me; 
				nTotalCount_cat_str8_me += nStr8Hands_me; 
				nTotalCount_cat_str4_me += nStr4Hands_me; 
				
				printf("% 2d\t%4.1f%% / %4.1f%% %4.1f%% / %4.1f%% %4.1f%% %4.1f%% = %4.1f%%", i, 
						(100.0*nDryHands_me)/nBoardsAnalyzed_me, 
						(100.0*nStr4Hands_me)/nBoardsAnalyzed_me, (100.0*nStr8Hands_me)/nBoardsAnalyzed_me,
						(100.0*nFlHands_me)/nBoardsAnalyzed_me, (100.0*nStr4FlHands_me)/nBoardsAnalyzed_me, (100.0*nStr8FlHands_me)/nBoardsAnalyzed_me
						,(100.0*nTotHands_me)/nBoardsAnalyzed_me 
						);
			} // end range over range
			printf("\n");
		}
		printf("(T:\t%4.1f%% / %4.1f%% %4.1f%% / %4.1f%% %4.1f%% %4.1f%% = %4.1f%%)\t", 
				(100.0*nTotalCount_cat_dry)/nBoardsAnalyzed, 
				(100.0*nTotalCount_cat_str4)/nBoardsAnalyzed, (100.0*nTotalCount_cat_str8)/nBoardsAnalyzed,
				(100.0*nTotalCount_cat_fl)/nBoardsAnalyzed, (100.0*nTotalCount_cat_str4fl)/nBoardsAnalyzed, 
				(100.0*nTotalCount_cat_str8fl)/nBoardsAnalyzed, (100.0*nTotalCount_cat)/nBoardsAnalyzed);
						
		if (bRangeVsRange) {
			printf("(T:\t%4.1f%% / %4.1f%% %4.1f%% / %4.1f%% %4.1f%% %4.1f%% = %4.1f%%)", 
					(100.0*nTotalCount_cat_dry_me)/nBoardsAnalyzed_me, 
					(100.0*nTotalCount_cat_str4_me)/nBoardsAnalyzed_me, (100.0*nTotalCount_cat_str8_me)/nBoardsAnalyzed_me,
					(100.0*nTotalCount_cat_fl_me)/nBoardsAnalyzed_me, (100.0*nTotalCount_cat_str4fl_me)/nBoardsAnalyzed_me, 
					(100.0*nTotalCount_cat_str8fl_me)/nBoardsAnalyzed_me, (100.0*nTotalCount_cat_me)/nBoardsAnalyzed_me);
		}
		printf("\n\n");
	}
	else // Old legacy display
	{
		double dTotalEqFreq = 0.0, dTotalFreq = 0.0;
		std::map<int, double>::iterator mit;
		for (int i = CBoardAnalyzer::MONSTERS; i <= CBoardAnalyzer::RUBBISH; ++i) {
			double nHands = 0;
			for (mit = mFeatures[i].begin(); mit != mFeatures[i].end(); ++mit) nHands += mit->second;
	
			printf("%5.1f%%: %s (EQ=%.1f%%)\n", (100.0*nHands)/nBoardsAnalyzed, CBoardAnalyzer::_pcHandSummaryTable[i], 100.0*dEquityTable[i]/nHands);
			for (mit = mFeatures[i].begin(); mit != mFeatures[i].end(); ++mit) {
				printf("\t%5.1f%%:\t %s (EQ=%.1f%%)\n", (100.0*mit->second)/nBoardsAnalyzed, CBoardAnalyzer::getHandFeatures(mit->first, true).c_str(), 100.0*dmEquity[i][mit->first]/mit->second);
				dTotalEqFreq += 100.0*dmEquity[i][mit->first];
				dTotalFreq += mit->second;
			}	
		}
		printf("(Total equity vs range = %.1f%%)\n", dTotalEqFreq/dTotalFreq);
	}
	return 0;
}
