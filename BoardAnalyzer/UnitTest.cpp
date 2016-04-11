//---------------------------------------------------------------
//---------------------------------------------------------------
// UNIT TEST CASES
//---------------------------------------------------------------
//---------------------------------------------------------------

#include <list>
#include <string.h> 

#include "CBoardAnalyzer.h"

//---------------------------------------------------------------

class CTestScenario {
public:
	CTestScenario() {}

	CTestScenario(const std::string& sHand, const std::string& sBoard, 
					const std::string& sSimpleContains, const std::string sSimpleExcludes,
					const std::string sComplexContains, const std::string sComplexExcludes)
	{
		this->set(sHand, sBoard, sSimpleContains, sSimpleExcludes, sComplexContains, sComplexExcludes);
	}

	void set(const std::string& sHand, const std::string& sBoard, 
					const std::string& sSimpleContains, const std::string sSimpleExcludes,
					const std::string sComplexContains, const std::string sComplexExcludes)
	{
		_sHand = sHand; 
		_sBoard = sBoard;
		_sSimpleContains = sSimpleContains;
		_sSimpleExcludes = sSimpleExcludes;
		_sComplexContains = sComplexContains;
		_sComplexExcludes = sComplexExcludes;
	}
	std::string _sHand, _sBoard, _sSimpleContains, _sSimpleExcludes, _sComplexContains, _sComplexExcludes; 
};

//---------------------------------------------------------------

static const char* ppcTestTable_[] = 
{ 
	// Straight flushes
	"Ac 2c", "3c 4c 5c", "Straight Flush;", "", "Straight Flush;", "",
	"2c 3d", "4d 5d 6d 7d", "One card Bottom end Straight Flush;", "", "One card Bottom end Straight Flush;", "",		
	"3c 3d", "4d 5d 6d 7d", "One card Bottom end Straight Flush;", "", "One card Bottom end Straight Flush;", "",
	"8c 8d", "4d 5d 6d 7d", "Straight Flush;", "", "Straight Flush;", "",
	"2d 3d", "4c 5d 6d 7d", "Flush;", "Straight;", "Weak Flush;", "Straight;",
	"2c 3d", "4d 5d 6d 9d", "Flush;", "Straight;", "Weak Flush;", "Straight;",		
	//Quads
	"2c 2d", "2h 2s 3d 4d", "Quads;", "", "Quads;", "Fake",
	"Ac 2h", "2c 2s 2d Kh", "Quads;", "", "Quads;", "Fake",
	"Ac Ah", "2h 2s 2c 2d", "Quads;", "", "Fake Quads Top kicker;", "",
	//Full house
	"8c 8h", "6c 6h 6d", "Full house", "", "Full house", "",
	"8c 9h", "8h Tc Th Td", "Full house", "", "Underfill Full house", "",
	"8c 9h", "8h 6c 6h 6d", "Full house", "", "Full house", "Underfill",
	// Flushes:
	"2d 5d", "6d Ad 8d", "Flush;", "", "Weak Flush;", "",
	"2d 7d", "6d Ad 8d", "Flush;", "", "Medium Flush;", "",
	"2d Kd", "6d Ad 8d", "Flush;", "", "Strong Flush;", "",
	"2h 3c", "4c 5c 6c Ac", "Flush;", "Straight;", "Weak Flush;", "Straight;",
	"5h 6h", "3h 4c Qh Kh", "Flush;", "Straight", "Weak Flush;", "Straight",
	// Flush draws:
	"2d 3d", "6d Ad 8h", "Flush Draw;", "", "Weak Flush Draw;", "",
	"2d Jd", "6d Ad 8h", "Flush Draw;", "", "Medium Flush Draw;", "",
	"2d Kd", "6d Ad 8h", "Flush Draw;", "", "Strong Flush Draw;", "",
	// Straights:
	"Ac 9h", "2s 3s 4d 5s", "Straight;", "", "One card Bottom end Straight;", "",
	"4c Jd", "5s 6s 7h 8h", "Straight;", "", "One card Bottom end Straight;", "",
	"2c 3d", "4c 5d 6d 7d", "Straight;", "", "One card Bottom end Straight;", "",
	"9c Jd", "5s 6s 7h 8h", "Straight;", "", "One card Straight;", "",
	"Qh Kh", "9c Th Jh", "Straight;", "", "Two card Straight;", "",
	"4h 5h", "6c 7h 8h", "Straight;", "", "Two card Straight;", "",
	// Straight draws:
	"Tc Jd", "5s 6s 7h 8h", "Gutshot Straight Draw;", "", "Two card Gutshot Straight Draw;", "",
	"Tc Kd", "5s 6s 7h 8h", "Gutshot Straight Draw;", "", "One card Gutshot Straight Draw;", "",
	"3c Jd", "5s 6s 7h 8h", "", "Straight Draw;", "", "Straight Draw;",
	"Qh Kh", "8c Th Jh", "Open ended Straight Draw;", "", "Two card Open ended Straight Draw;", "",
	"4h 5h", "6c 7h 9h", "Open ended Straight Draw;", "", "Two card Open ended Straight Draw;", "",
	"Ac 9h", "3s 4s 5c 6c", "", "Straight Draw;", "", "Straight Draw;",
	"Kc 7h", "9s Ts Jc 2c", "Open ended Straight Draw;", "", "Two card Bellybuster Straight Draw;", "",
	"Ac 9h", "3s 4s 5d", "Gutshot Straight Draw;", "", "One card Bottom end Gutshot Straight Draw;", "",
	"Ac 9h", "2s 3s 4d", "Gutshot Straight Draw;", "", "One card Bottom end Gutshot Straight Draw;", "",
	"Tc Qd", "5s 6s 7h 8h", "Gutshot Straight Draw;", "", "One card Gutshot Straight Draw;", "",
	"Ac Kd", "Qs Js 6d", "Gutshot Straight Draw;", "", "Two card Gutshot Straight Draw;", "",
	"Ac Kd", "Qs Js 9d", "Gutshot Straight Draw;", "", "Two card Gutshot Straight Draw;", "",
	"Tc Td", "7h 8h 9h", "Open ended Straight Draw;", "", "One card Open ended Straight Draw;", "",
	"Tc Td", "7h 8h 6h", "Gutshot Straight Draw;", "", "One card Gutshot Straight Draw;", "",
	"Jh 9h", "6d Qc Tc", "Open ended Straight Draw;", "", "Two card Open ended Straight Draw;", "",
	"Ah 3h", "3c 4s 5d 6h", "", "Straight Draw;", "", "Straight Draw;",
	"Kh 9h", "Jc Tc 8s 6d", "Open ended Straight Draw;", "", "Two card Open ended Straight Draw;", "",
	"Qc Th", "6c Kh Jd", "Open ended Straight Draw;", "", "Two card Open ended Straight Draw;", "",
	"2c Td", "7h 8h 9h Tc", "Weak kicker;", "Straight Draw;", "Weak kicker;", "Straight Draw;",
	//Sets
	"3c 3d", "2h 3h 4h 5h", "Set;","","Set;","",
	// Trips
	"Ad 2h", "2d 2s 7d 6d", "Trips Top kicker; Flush Draw;", "", "Trips Top kicker; Strong Flush Draw;", "",
	"8d Kh", "Kd Ks 7d 6d", "Trips Weak kicker;", "", "Trips Weak kicker;", "",
	"Ad 9s", "4c 4d 4s", "Fake Trips;", "", "Fake Trips;", "",
	// 2 pair
	"4c 5d", "4h 5h Ks", "Middle 2 Pair;", "", "Middle 2 Pair;", "",
	"4c Kd", "4h 5h Ks", "Top and middle 2 Pair;", "", "Top and middle 2 Pair;", "",
	"5c Kd", "4h 5h Ks", "Top 2 Pair;", "", "Top 2 Pair;", "",
	"Ac Ad", "2c 2d 7s", "Top and middle 2 Pair;", "", "Top and middle 2 Pair;", "",
	"Ac Ad", "2c 7d 7s", "Top 2 Pair;", "", "Top 2 Pair;", "",
	"3c 3d", "8c 7d 7s", "Middle 2 Pair;", "", "Middle 2 Pair;", "",
	"Ah 3s", "3d 7h 7d 4s", "One card Top and middle 2 Pair Top kicker;", "", "One card Top and middle 2 Pair Top kicker;", "",
	"2h 2d", "5h 5d 8s 8h", "Fake 2 Pair;", "", "Fake 2 Pair;", "",
	"8c 9c", "Th Td 8s", "One card Top 2 Pair Weak kicker;", "","One card Top 2 Pair Weak kicker;", "", 
	"8c 6c", "Th Td 8s", "One card Top 2 Pair Weak kicker;", "","One card Top 2 Pair Weak kicker;", "", 
	"8c 9c", "3h 3d 8s", "One card Top 2 Pair Weak kicker;", "","One card Top 2 Pair Weak kicker;", "", 
	"8c 6c", "3h 3d 8s", "One card Top 2 Pair Weak kicker;", "","One card Top 2 Pair Weak kicker;", "", 
	// 1 pair
	"Ah Ad", "Jh Ts 2c", "Overpair;", "", "Overpair;", "",
	"Th Td", "Jh 9s 2c", "2nd Pair;", "", "2nd Pair;", "",
	"3h 3d", "Jh 9s 2c", "Middle Pair;", "", "Middle Pair;", "",
	"2h 2d", "Jh 9s 3c", "Bottom Pair;", "", "Bottom Pair;", "",
	"Ah Kd", "Ks 8h 7d", "Top Pair Top kicker;", "", "Top Pair Top kicker;", "",
	"Qh 8d", "Ks 8h 7d", "2nd Pair Good kicker;", "", "2nd Pair Good kicker;", "",
	"Qh 7d", "Ks 8h 7h 2c", "Middle Pair Good kicker;", "", "Middle Pair Good kicker;", "",
	"7c 6d", "Ks 8h 7d", "Bottom Pair Weak kicker;", "", "Bottom Pair Weak kicker;", "",
	"7d 6d", "Kd 8d 7s", "Bottom Pair Weak kicker; Flush Draw;", "", "Bottom Pair Weak kicker; Medium Flush Draw;", "",
	"Ah Ks", "2d 2c 6d", "Two Overcards;", "Pair", "Two Overcards;", "Pair",
	// Overcards
	"Ah Ks", "Jh Td 2c", "Two Overcards; Gutshot Straight Draw;", "", "Two Overcards; Two card Gutshot Straight Draw;", "",
	"Ad 9s", "4c 4d 4s", "Fake Trips; Two Overcards", "", "Fake Trips; Two Overcards;", "",
	"Ad 9s", "Jc Jd Js", "Fake Trips; One Overcard", "", "Fake Trips; One Overcard;", "",
	"Td 9s", "Jc Jd Js", "Fake Trips;", "Overcard", "Fake Trips;", "Overcard",
	0
};

//---------------------------------------------------------------

void do_unit_test()
{
	printf("Running unit test.....\n");
	std::list<CTestScenario> lTests;
	
	const char** psz = &ppcTestTable_[0]; 
	for (; *psz; psz += 6) {
		lTests.push_back(CTestScenario(psz[0], psz[1], psz[2], psz[3], psz[4], psz[5]));
	}
			
	int nTest = 0;
	int nFailed = 0;
	std::list<CTestScenario>::iterator it;
	for (it = lTests.begin(); it != lTests.end(); ++it, nTest++) {
		CTestScenario& ts = *it;
		CBoardAnalyzer theAnalyzer;

		std::string sHB = ts._sHand + " " + ts._sBoard;
		
		int nCards = 0;
		int nlen = sHB.length();	
		int ncurr = 0;
		CCard::Hole hand;
		while (nlen > 0) {
	
			CCard newCard;
			int npos = newCard.set(sHB.c_str() + ncurr);
			ncurr += npos;
			nlen -= npos;

			if (0 == newCard) {
				printf("Error! at %d %d\n", nTest, ncurr);
				nFailed++;
				continue;
			}
			nCards++;
			
			if (1 == nCards) {
				hand.first = newCard;
			}
			else if (nCards == 2) {
				hand.second = newCard;
				if (!theAnalyzer.addHand(hand))
				{
					printf("Error! at %d %d = %d\n", nTest, ncurr, (int)newCard);
					nFailed++;
					continue;
				}
			}
			else {
				if (!theAnalyzer.addCardToBoard(newCard)) {
					printf("Error! at %d %d = %d\n", nTest, ncurr, (int)newCard);
					nFailed++;
					continue;
				}
			}
		} // end loop over cards within args 

		theAnalyzer.analyzeBoard();

		if (ts._sSimpleContains.length() > 0) {
			if (!strstr(theAnalyzer.getFeatures(false).c_str(), ts._sSimpleContains.c_str())) {
				printf("Failed! at %d, simple %s vs actual %s (%s)\n", 1+nTest, ts._sSimpleContains.c_str(), theAnalyzer.getFeatures(false).c_str(), sHB.c_str());
				nFailed++;
				continue;
			}
			if (!strstr(theAnalyzer.getFeatures(true).c_str(), ts._sComplexContains.c_str())) {
				printf("Failed! at %d, complex %s vs actual %s (%s)\n", 1+nTest, ts._sComplexContains.c_str(), theAnalyzer.getFeatures(true).c_str(), sHB.c_str());
				nFailed++;
				continue;
			}
		}
		if (ts._sSimpleExcludes.length() > 0) {
			if (strstr(theAnalyzer.getFeatures(false).c_str(), ts._sSimpleExcludes.c_str())) {
				printf("Failed! at %d, !simple %s vs actual %s (%s)\n", 1+nTest, ts._sSimpleExcludes.c_str(), theAnalyzer.getFeatures(false).c_str(), sHB.c_str());
				nFailed++;
				continue;
			}
		}
		if (ts._sComplexExcludes.length() > 0) {
			if (strstr(theAnalyzer.getFeatures(true).c_str(), ts._sComplexExcludes.c_str())) {
				printf("Failed! at %d, !complex %s vs actual %s (%s)\n", 1+nTest, ts._sComplexExcludes.c_str(), theAnalyzer.getFeatures(true).c_str(), sHB.c_str());
				nFailed++;
				continue;
			}
		}
	}
	printf("Completed unit test failed %d of %d\n", nFailed, nTest);
}
