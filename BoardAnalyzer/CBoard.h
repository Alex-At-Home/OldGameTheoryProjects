#ifndef CBOARD_H_
#define CBOARD_H_

#include "CCard.h"

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

// BOARD CALCULATIONS

class CBoard {
	
public:
	CBoard();
	
	void generateRandomBoard(int nBoardTable[], int nBoardCards);
	bool isBoardAllowed(const CCard::Hole& hand);
	void resetBoard(const CCard::Hole& hand);
	void setBoard(int nBoardTable[], int nBoardCards);
	
protected:
	int _nCardTable[52];
	int _nDeadTable[9]; // 2 players' hands + 3/4/5 board cards
	int _nDeadLookup[160];
	int _nDeads;
};

///////////////////////////////////////////////////////////////////////////////////////////

#endif /*CBOARD_H_*/
