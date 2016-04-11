#include "CBoard.h"

#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////////////////

CBoard::CBoard()
{
	CCard c1;
	for (int i = 0; i < 160; ++i) {
		_nDeadLookup[i] = 0;
	}
	for (int i = 0; i < 52; ++i) {
		_nCardTable[i] = (int)c1.setFromPokerEnum(i);
	}
	_nDeads = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////

void CBoard::generateRandomBoard(int nBoardTable[], int nBoardCards)
{
	// Reset previous board:
	for (int i = 0; i < _nDeads; ++i)
	{
		_nDeadLookup[_nDeadTable[i]] = 0;
	}
	_nDeads = 0;
	
	for (int i = 0; i < nBoardCards; ++i) {		
		unsigned int nRand, nCard;
		do {
			
			nRand = (unsigned int)(52.0*(::rand()/(double)RAND_MAX));
			nCard = _nCardTable[nRand];
			
		} while (1 == _nDeadLookup[nCard]);

		nBoardTable[i] = nCard;
		_nDeadLookup[nCard] = 1;
		_nDeadTable[i] = nCard;
	}
	_nDeads = nBoardCards;
}

///////////////////////////////////////////////////////////////////////////////////////////

void CBoard::setBoard(int nBoardTable[], int nBoardCards)
{
	// Reset previous board:
	for (int i = 0; i < _nDeads; ++i)
	{
		_nDeadLookup[_nDeadTable[i]] = 0;
	}
	_nDeads = 0;
	
	for (int i = 0; i < nBoardCards; ++i) {		
		unsigned int nCard;
		nCard = nBoardTable[i];
		_nDeadLookup[nCard] = 1;
		_nDeadTable[i] = nCard;
	}
	_nDeads = nBoardCards;	
}

///////////////////////////////////////////////////////////////////////////////////////////

bool CBoard::isBoardAllowed(const CCard::Hole& hand)
{
	if ((0 == _nDeadLookup[(int)hand.first])&&(0 == _nDeadLookup[(int)hand.second]))
	{
		_nDeadTable[_nDeads++] = (int)hand.first;
		_nDeadLookup[(int)hand.first] = 1;
		_nDeadTable[_nDeads++] = (int)hand.second;
		_nDeadLookup[(int)hand.second] = 1;
		
		return true;
	}
	else {
		return false;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////

void CBoard::resetBoard(const CCard::Hole& hand)
{
	// Just remove board cards

	_nDeadLookup[(int)hand.first] = 0;
	_nDeadLookup[(int)hand.second] = 0;
	
	_nDeads -= 2;
}

