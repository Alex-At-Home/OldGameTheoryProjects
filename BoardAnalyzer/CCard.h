#ifndef CCARD_H_
#define CCARD_H_

////////////////////////////////////////////////////

#include <string>
#include <deque>

////////////////////////////////////////////////////
////////////////////////////////////////////////////

class CCard_Hole;

class CCard {

public:
	enum { HEARTS = 1, DIAMONDS = 2, CLUBS = 3, SPADES = 4, SUITS = 5 }; 
	
	typedef CCard_Hole Hole;
	
	CCard() { _nVal = 0; }
	CCard(int nVal) { _nVal = nVal; }
	CCard(int nVal, int nDummy) { this->setFromPokerEnum(nVal); }
	CCard(const std::string& s) { this->set(s); }
	operator int() const { return _nVal; }
	
	int set(const std::string& s);
	void set(int nVal) { _nVal = nVal; }
	CCard setFromPokerEnum(int nPokerEnum) { _nVal = (1+(nPokerEnum/13)) + 10*(2+(nPokerEnum%13)); return *this; } 
	
	int getSuit() const { return (_nVal % 10); }
	int getRank() const { return _nVal/10; }
	CCard changeSuit(int nNewSuit) { _nVal = (_nVal/10)*10 + nNewSuit; return *this; }  

	std::string convertToString() const 
		{ std::string s("xx"); if (_nVal) { s[0] = _rankTable[this->getRank()]; s[1] = _suitTable[this->getSuit()]; } return s; } 
	std::string s() const { return this->convertToString(); } // (for use in printfs)
	
	int convertToPokerEnum() const { return ((this->getSuit()-1)*13) + (this->getRank()-2); }

	static int addHand(const std::string& s, std::deque<CCard::Hole>& l, double& dRunningFreq);
	static int addHand(const std::string& s, std::deque<CCard::Hole>& l) { double d; return CCard::addHand(s, l, d); }
protected:
	static bool isRank(char c, int& nRank);
	static bool isSuit(char c, int& nSuit);
	int _nVal;

	static const char *_suitTable; 
	static const char *_rankTable; 
};
class CCard_Hole {
public:
	CCard first;
	CCard second;
	double dFreq;
	CCard_Hole() { first = CCard(0); second = CCard(0); dFreq = 1.0; }
	CCard_Hole(CCard c1, CCard c2, double df = 1.0) { first = c1; second = c2; dFreq = df; }
};

////////////////////////////////////////////////////
////////////////////////////////////////////////////

#endif /*CCARD_H_*/
