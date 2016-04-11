#include "CCard.h"

////////////////////////////////////////////////////

const char *CCard::_suitTable = "xhdcs"; 
const char *CCard::_rankTable = "xx23456789TJQKA"; 

////////////////////////////////////////////////////
////////////////////////////////////////////////////

bool CCard::isRank(char c, int& nRank)
{
	if (('a' == c)||('A' == c)) {
		nRank = 140; return true;
	}		
	else if (('k' == c)||('K' == c)) {
		nRank = 130; return true;
	}		
	else if (('q' == c)||('Q' == c)) {
		nRank = 120; return true;
	}		
	else if (('j' == c)||('J' == c)) {
		nRank = 110; return true;
	}		
	else if (('t' == c)||('T' == c)) {
		nRank = 100; return true;
	}		
	else if (isdigit(c)) {
		int nVal = c - 0x30; nRank = nVal*10; return true;
	}
	return false;
}

////////////////////////////////////////////////////

bool CCard::isSuit(char c, int& nSuit)
{
	if (('h' == c)||('H' == c)) {
		nSuit = HEARTS; return true;
	}
	else if (('d' == c)||('D' == c)) {
		nSuit = DIAMONDS; return true;
	}
	else if (('s' == c)||('S' == c)) {
		nSuit = SPADES; return true;
	}
	else if (('c' == c)||('C' == c)) {
		nSuit = CLUBS; return true;
	}
	return false;
}

////////////////////////////////////////////////////

int CCard::set(const std::string& s)
{
	_nVal = 0;
	int n = 0;
	int nRank, nSuit;
	for (; n < (int)s.length(); ++n) {

		if (this->getSuit() && this->getRank()) break; // (done)

		if ((' ' == s[n])||(',' == s[n])||(',' == s[n])) {
			
			if (!this->getSuit() || !this->getRank()) {
				continue;
			}
			else break; // already have a card value
		}
		else if (isRank(s[n], nRank)) {
			
			if (this->getRank()) { _nVal = 0; break; }
			else _nVal += nRank;
		}
		else if (isSuit(s[n], nSuit)) {

			if (this->getSuit()) { _nVal = 0; break; }
			else _nVal += nSuit;
		}
		// else Ignore (eg '.' '[' etc)
		
	} // end loop over input string

	if (!this->getSuit() || !this->getRank()) {
		_nVal = 0; // problem
	}
	return n;
}

////////////////////////////////////////////////////

int CCard::addHand(const std::string& s, std::deque<CCard::Hole>& l, double& dRunningFreq)
{
	bool bStdFmt = false;
	const char *sz = s.c_str();
	CCard c1;
	int n = c1.set(sz);
	if (0 != (int)c1) {
		CCard c2;
		n += c2.set(sz + n);
		if (0 != (int)c2) {
			// (Allow for :freq format)
			double dFreq = 1.0;
			if (((n + 1) < (int)s.length()) && (':' == s[n])) {
				double d;
				int nLen;
				if (1 == sscanf(s.c_str() + n, ":%lf%n", &d, &nLen)) {
					dFreq = d;
				}
				n += nLen;
			}
			l.push_back(CCard::Hole(c1, c2, dFreq));
			dRunningFreq += dFreq;
			bStdFmt = true;
		}
	}
	if (!bStdFmt) {
		n = 0; // reset
		double dFreq = 1.0;
		
		// Allowed (eg): KK, KK+, AT, ATs, ATo, ATs+, AT+, ATo+

		int nRank1 = 0, nRank2;
		enum { OFFSUIT, SUITED, BOTH } nSuitedness = BOTH; 
		if (2 == s.length()) { // must be AA or AT
			
			if (!isRank(s[0], nRank1)) return 1;
			if (!isRank(s[1], nRank2)) return 2;
			n = 2;
		}
		else { // Could be anything:
			
			// Skip to the characters
			for (; n < (int)s.length(); ++n) {
				if (isRank(s[n], nRank1)) break;
			}
			if ((0 == nRank1)||((s.length() - n) < 2)) { 
				return s.length(); // end of the string...
			}
			if (!isRank(s[n+1], nRank2)) {
				return n + 1;
			}
			n += 2;
			if ((s.length() - n) >= 1) {
				if (('o' == s[n])||'O' == s[n]) {
					nSuitedness = OFFSUIT;
					n++;
				}
				else if (('s' == s[n])||'S' == s[n]) {
					nSuitedness = SUITED;
					n++;
				}
			}
		}
		// Handle all the "-" and "+" cases
		int nRank2_max = nRank2 + 10;
		if ((n < (int)s.length()) && ('+' == s[n])) {
			n++;
			nRank2_max = (nRank2 == nRank1) ? 150 : nRank1;
		}
		else if ((n < (int)s.length()) && ('-' == s[n])) {
			
			std::deque<CCard::Hole> l2;
			n += CCard::addHand(s.c_str() + n + 1, l2) + 1;
			if (l2.size() > 0) {
				CCard::Hole h = *l2.begin();
				
				int nRmax = h.second.getRank()*10;
				if (nRmax < nRank2) { // AK-AJ==AJ-AK; 88-22==22-88
					nRmax = nRank2;
					nRank2 = h.second.getRank()*10;
					if (nRank1 == nRmax) nRank1 = nRank2; 
				}				
				nRank2_max = nRmax + 10;
				dFreq = h.dFreq;
			}
			else return n;
		}
		
		// Finally, if there's a ':' then can assign a relative probability to the hand
		if (((n + 1) < (int)s.length()) && (':' == s[n])) {
			double d;
			int nLen;
			if (1 == sscanf(s.c_str() + n, ":%lf%n", &d, &nLen)) {
				dFreq = d;
			}
			n += nLen;
		}
		
		// If we're here, we have all the parameters, so create some cards
		if (nRank1 == nRank2) nSuitedness = OFFSUIT;
		
		for (int nR2 = nRank2; nR2 < nRank2_max; nR2 += 10) {
			int nR1 = (nRank2 == nRank1) ? nR2 : nRank1;			
			
			if (OFFSUIT != nSuitedness) { // add all the suited cards in one go, to make compression easier				
				for (int n1 = HEARTS; n1 <= SPADES; ++n1) {
					CCard c1(nR1 + n1);
					CCard c2(nR2 + n1);
					l.push_back(CCard::Hole(c1, c2, dFreq));
					dRunningFreq += dFreq;
				}
			}
			if (SUITED != nSuitedness) {
				for (int n1 = HEARTS; n1 <= SPADES; ++n1) {
					CCard c1(nR1 + n1);
	
					int n2_start = (nRank1 == nRank2) ? (n1 + 1) : HEARTS;
					for (int n2 = n2_start; n2 <= SPADES; ++n2) {
						CCard c2(nR2 + n2);
						if (n1 != n2) {
							l.push_back(CCard::Hole(c1, c2, dFreq));
							dRunningFreq += dFreq;
						}
					}
				}
			}
		}
	} // end if aggregated format
	return n;
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////

