//______________________________________________________________________________________________
//
// ZeroOneGames
//______________________________________________________________________________________________
//
// Some GTO solutions to [0,1] games I'm inventing
//
// Alex Piggott April 2009
//______________________________________________________________________________________________

#include <exception>
#include <string>
#include <vector>

#define __DEBUG_PRINT

//______________________________________________________________________________________________
//
// Game structure
//______________________________________________________________________________________________

// Generic structure:

struct CGameBase {
	//TODO: some nicer way of doing this?!
	//TODO: also I need to have different action threshold types for different draw types?
	enum { DRAW_DRY = 0, NUM_DRAWS };
//	enum { DRAW_DRY = 0, DRAW_OPEN, NUM_DRAWS };
//	enum { DRAW_DRY = 0, DRAW_OPEN, DRAW_SEMIOPEN, NUM_DRAWS };
	static const double dDrawProbs[1+NUM_DRAWS];
	static const int nHandBins = 10;
	static const double dPctPerHandBin = 1.0/nHandBins;
};
const double CGameBase::dDrawProbs[1+NUM_DRAWS] = { 0.80, -1.0 };
//const double CGameBase::dDrawProbs[1+NUM_DRAWS] = { 0.80 , 0.20, -1.0 };
//const double CGameBase::dDrawProbs[1+NUM_DRAWS] = { 0.64 , 0.20, 0.16, -1.0 };

template<int nNOA_H, int nNOA_V, int nMS_H, int nMS_V, int nMNT_H, int nPn> class CGameStructure : public CGameBase
{
public:
	static const int nPlayer = nPn;
	
	// Player actions/strategies
	
	static const int nNumObsActions_H = nNOA_H; // for P1, K or B; for P2, just "."
	static const int nNumObsActions_V = nNOA_V;  
		// ^^(these values can be restricted to 1 or 2, which can make processing a bit more efficient)

	static const int nStratToObsActionThresh_H; // (just need 1/0 threshold because of above restriction)
	static const int nStratToObsActionThresh_V;
	
	static const int nNumStrategies_H[nNumObsActions_V]; // more complete, eg KRC (vs K) or BC (vs B)
	static const int nMaxStrategies_H = nMS_H; // (the max across all observable actions)
	static const int nNumStrategies_V[nNumObsActions_H];  
	static const int nMaxStrategies_V = nMS_V; 
	
	// Decision points
	static const int nMaxThresholds_H = nMNT_H;
	static const int nNumThresholds_H[nNumObsActions_V];
	static const int nThresh_LowerAction[nNumObsActions_V][nMaxThresholds_H];
	static const int nThresh_UpperAction[nNumObsActions_V][nMaxThresholds_H];

	// Results of actions
	// For performance reasons, all the "fold" actions need to follow strictly "nofold" actions
	static const int nCombinedActions_noFold; 
	static const int nCombinedActions;

	enum { FOLD_H, FOLD_V, NOFOLD };
	struct CombinedAction {
		int nCombinedAction;
		int nOutcome_H;
		double dCost_H;
		double dCost_V; 
	}; // (24B)
	static const struct CombinedAction combinedActionTable[nMaxStrategies_H][nMaxStrategies_V];

	//TODO: don't we want to make this configurable per game?
	static const double dBetSize_H = 1.0; // (x the pot)
	static const double dBetSize_V = 1.0; // (x the pot)

	//TODO: don't we want to make these next three configurable per game?
	static const int nHandBins = 100; //TODO 10 normally?
};

//______________________________________________________________________________________________

// Very simple game (P1 has to K, can KF or KC, P2 can B or K):

namespace GSS {
	// Enums:
	enum { P1STRAT_KF = 0, P1STRAT_KC, P1STRAT_NUM };
	enum { P2STRAT_P1K_K = 0, P2STRAT_P1K_B, P2STRAT_P1K_NUM };
	enum { ACTIONS_KK = 0, ACTIONS_KBC, ACTIONS_KBF, ACTIONS_NUM };
	static const int ACTIONS_NUM_NOFOLD = ACTIONS_KBF;
};
typedef CGameStructure<1, 1, 2, 2, 1, 1> CGameStructure_Simple_P1;
typedef CGameStructure<1, 1, 2, 2, 2, 2> CGameStructure_Simple_P2;

template<>
const int CGameStructure_Simple_P1::nStratToObsActionThresh_H = GSS::P1STRAT_NUM; 
template<>
const int CGameStructure_Simple_P1::nStratToObsActionThresh_V = GSS::P2STRAT_P1K_NUM; 
template<>
const int CGameStructure_Simple_P2::nStratToObsActionThresh_H = GSS::P2STRAT_P1K_NUM; 
template<>
const int CGameStructure_Simple_P2::nStratToObsActionThresh_V = GSS::P1STRAT_NUM; 
template<>
const int CGameStructure_Simple_P1::nNumStrategies_H[] = { GSS::P1STRAT_NUM };
template<>
const int CGameStructure_Simple_P1::nNumStrategies_V[] = { GSS::P2STRAT_P1K_NUM };
template<>
const int CGameStructure_Simple_P2::nNumStrategies_H[] = { GSS::P2STRAT_P1K_NUM };
template<>
const int CGameStructure_Simple_P2::nNumStrategies_V[] = { GSS::P1STRAT_NUM };
template<>
const int CGameStructure_Simple_P1::nNumThresholds_H[] = { 1 };
template<>
const int CGameStructure_Simple_P1::nThresh_LowerAction[1][1] = { GSS::P1STRAT_KF };
template<>
const int CGameStructure_Simple_P1::nThresh_UpperAction[1][1] = { GSS::P1STRAT_KC };
template<>
const int CGameStructure_Simple_P2::nNumThresholds_H[] = { 2 };
template<>
const int CGameStructure_Simple_P2::nThresh_LowerAction[1][2] = { GSS::P2STRAT_P1K_B, GSS::P2STRAT_P1K_K };
template<>
const int CGameStructure_Simple_P2::nThresh_UpperAction[1][2] = { GSS::P2STRAT_P1K_K, GSS::P2STRAT_P1K_B };
template<>
const int CGameStructure_Simple_P1::nCombinedActions_noFold = GSS::ACTIONS_NUM_NOFOLD;
template<>
const int CGameStructure_Simple_P1::nCombinedActions = GSS::ACTIONS_NUM;
template<>
const int CGameStructure_Simple_P2::nCombinedActions_noFold = GSS::ACTIONS_NUM_NOFOLD;
template<>
const int CGameStructure_Simple_P2::nCombinedActions = GSS::ACTIONS_NUM;
template<>
const struct CGameStructure_Simple_P1::CombinedAction 
	CGameStructure_Simple_P1::combinedActionTable[GSS::P1STRAT_NUM][GSS::P2STRAT_P1K_NUM] = 
{			
		// P2STRAT_P1K_K						, P2STRAT_P1K_B
		{ {GSS::ACTIONS_KK, NOFOLD, 0.0, 0.0}	, {GSS::ACTIONS_KBF, FOLD_H, 0.0, dBetSize_V} },		// P1STRAT_KF
		{ {GSS::ACTIONS_KK, NOFOLD, 0.0, 0.0}	, {GSS::ACTIONS_KBC, NOFOLD, dBetSize_V, dBetSize_V} }	// P1STRAT_KC
};
template<>
const struct CGameStructure_Simple_P2::CombinedAction 
	CGameStructure_Simple_P2::combinedActionTable[GSS::P2STRAT_P1K_NUM][GSS::P1STRAT_NUM] = 
{			
		// P1STRAT_KF									, P1STRAT_KC
		{ {GSS::ACTIONS_KK, NOFOLD, 0.0, 0.0}			, {GSS::ACTIONS_KK, NOFOLD, 0.0, 0.0} },				// P2STRAT_P1K_K
		{ {GSS::ACTIONS_KBF, FOLD_V, dBetSize_H, 0.0}	, {GSS::ACTIONS_KBC, NOFOLD, dBetSize_H, dBetSize_H} }	// P2STRAT_P1K_B
};
		
//______________________________________________________________________________________________

// Less simple game (P1 can KF, KC, or B, P2 can (K)B, (K)K, (B)F, or (B)C):

namespace GSM {
	// Enums:
	enum { P1STRAT_KF = 0, P1STRAT_KC, P1STRAT_B, P1STRAT_NUM };
	enum { P1ACT_K = 0, P1ACT_B };
	enum { P2STRAT_P1K_K = 0, P2STRAT_P1K_B, P2STRAT_P1K_NUM };
	enum { P2STRAT_P1B_F = 0, P2STRAT_P1B_C, P2STRAT_P1B_NUM };
	static const int P2STRAT_MAX = P2STRAT_P1K_NUM; //==P2STRAT_P1B_NUM
	enum { ACTIONS_KK = 0, ACTIONS_KBC, ACTIONS_BC, ACTIONS_KBF, ACTIONS_BF, ACTIONS_NUM };
	static const int ACTIONS_NUM_NOFOLD = ACTIONS_KBF;
};

typedef CGameStructure<2, 1, 3, 2, 3, 1> CGameStructure_Medium_P1;
typedef CGameStructure<1, 2, 2, 3, 2, 2> CGameStructure_Medium_P2;

template<>
const int CGameStructure_Medium_P1::nStratToObsActionThresh_H = GSM::P1STRAT_B; 
template<>
const int CGameStructure_Medium_P1::nStratToObsActionThresh_V = GSM::P2STRAT_MAX; 
template<>
const int CGameStructure_Medium_P2::nStratToObsActionThresh_H = GSM::P2STRAT_MAX; 
template<>
const int CGameStructure_Medium_P2::nStratToObsActionThresh_V = GSM::P1STRAT_B; 
template<>
const int CGameStructure_Medium_P1::nNumStrategies_H[] = { GSM::P1STRAT_NUM };
template<>
const int CGameStructure_Medium_P1::nNumStrategies_V[] = { GSM::P2STRAT_P1K_NUM, GSM::P2STRAT_P1B_NUM };
template<>
const int CGameStructure_Medium_P2::nNumStrategies_H[] = { GSM::P2STRAT_P1K_NUM, GSM::P2STRAT_P1B_NUM };
template<>
const int CGameStructure_Medium_P2::nNumStrategies_V[] = { GSM::P1STRAT_NUM };
template<>
const int CGameStructure_Medium_P1::nNumThresholds_H[] = { 3 };
template<>
const int CGameStructure_Medium_P1::nThresh_LowerAction[1][3] = { GSM::P1STRAT_B,  GSM::P1STRAT_KF, GSM::P1STRAT_KC };
template<>
const int CGameStructure_Medium_P1::nThresh_UpperAction[1][3] = { GSM::P1STRAT_KF, GSM::P1STRAT_KC, GSM::P1STRAT_B };
template<>
const int CGameStructure_Medium_P2::nNumThresholds_H[] = { 2, 1 };
template<>
const int CGameStructure_Medium_P2::nThresh_LowerAction[2][2] = { { GSM::P2STRAT_P1K_B, GSM::P2STRAT_P1K_K }, { GSM::P2STRAT_P1B_F, -1 } };
template<>
const int CGameStructure_Medium_P2::nThresh_UpperAction[2][2] = { { GSM::P2STRAT_P1K_K, GSM::P2STRAT_P1K_B }, { GSM::P2STRAT_P1B_C, -1 } };
template<>
const int CGameStructure_Medium_P1::nCombinedActions_noFold = GSM::ACTIONS_NUM_NOFOLD;
template<>
const int CGameStructure_Medium_P1::nCombinedActions = GSM::ACTIONS_NUM;
template<>
const int CGameStructure_Medium_P2::nCombinedActions_noFold = GSM::ACTIONS_NUM_NOFOLD;
template<>
const int CGameStructure_Medium_P2::nCombinedActions = GSM::ACTIONS_NUM;
template<>
const struct CGameStructure_Medium_P1::CombinedAction 
	CGameStructure_Medium_P1::combinedActionTable[GSM::P1STRAT_NUM][GSM::P2STRAT_MAX] = 
{
		// P2STRAT_P1K_K/P2STRAT_P1B_F			, P2STRAT_P1K_B/P2STRAT_P1B_C
		{ {GSM::ACTIONS_KK, NOFOLD, 0.0, 0.0}	, {GSM::ACTIONS_KBF, FOLD_H, 0.0, 1.0} },	// P1STRAT_KF
		{ {GSM::ACTIONS_KK, NOFOLD, 0.0, 0.0}	, {GSM::ACTIONS_KBC, NOFOLD, 1.0, 1.0} },	// P1STRAT_KC
		{ {GSM::ACTIONS_BF, FOLD_V, 1.0, 0.0}	, {GSM::ACTIONS_BC, NOFOLD, 1.0, 1.0} }		// P1STRAT_B
};
template<>
const struct CGameStructure_Medium_P2::CombinedAction 
	CGameStructure_Medium_P2::combinedActionTable[GSM::P2STRAT_MAX][GSM::P1STRAT_NUM] = 
{			
	// P1STRAT_KF							, P1STRAT_KC							, P1STRAT_B
	{ {GSM::ACTIONS_KK, NOFOLD, 0.0, 0.0}	, {GSM::ACTIONS_KK, NOFOLD, 0.0, 0.0}	, {GSM::ACTIONS_BF, FOLD_H, 0.0, 1.0} },// P2STRAT_P1K_K/P2STRAT_P1B_F
	{ {GSM::ACTIONS_KBF, FOLD_V, 1.0, 0.0}	, {GSM::ACTIONS_KBC, NOFOLD, 1.0, 1.0}	, {GSM::ACTIONS_BC, NOFOLD, 1.0, 1.0} }	// P2STRAT_P1K_B/P2STRAT_P1B_C
};
		
//______________________________________________________________________________________________
//______________________________________________________________________________________________

// Equity calculations:

template<class GS> class CEquityInfo {
public:	
	CEquityInfo()
	{ 
		_dFinalWeight = 0.0;
		_nStartIndex[0] = INT_MAX; // (ensures if getEquity is called for an unused object returns 0.0)
		_writeCache.nBlock = 0; 
	} 
	void resetEquity() {
		_writeCache.nBlock = 0; _writeCache.dHalfProb = 0.0; }
	
	// Lower threshold 
	// (this and "setEnd..." take doubles to allow for future handling of "hand block" fragments - maybe change later) 
	void setStartOfCumProb(const double& dHandIndex)
	{
		int nHandIndex = (int)(dHandIndex + 0.5);
		_nStartIndex[_writeCache.nBlock] = nHandIndex;
		
		_writeCache.pdCumProb = &_dCumProbTable[_writeCache.nBlock][0];
	}
	// Upper threshold (see above re "dHandIndex" being double)
	void setEndOfCumProb(const double& dHandIndex, const double& dEndProb)
	{
		int nHandIndex = (int)(dHandIndex + 0.5);
		_nEndIndex[_writeCache.nBlock] = nHandIndex;
		
		if (0 == _writeCache.nBlock) {
			_dIntermediateWeight = dEndProb;
			_writeCache.nBlock = 1;
		}
		else {
			_dFinalWeight = dEndProb;
			_writeCache.nBlock = 0; // reset ready for next time
		}		
#ifdef __DEBUG_PRINT
//printf(">>>>P%d (%p): EoCP @ %f = cum %f (%d)\n", GS::nPlayer, this, dHandIndex, dEndProb, _writeCache.nBlock);		
#endif //__DEBUG_PRINT
	}
	// Fill in info in between the upper and lower threshold
	void accumulateProb(const double &dProb) 
	{
/**/
//Explain plz		
		double d1 = dProb - _writeCache.dHalfProb; 
		*_writeCache.pdCumProb++ = dProb - 0.5*d1;
		_writeCache.dHalfProb = dProb;
	}
	double getEquityTimesWeight(int nH) const;
	std::string getSummaryDisplay() const;
private:
	double _dIntermediateWeight; // When there are two blocks, the upper limit of the lower one
	double _dFinalWeight;
private:
	int _nStartIndex[2]; // Each action has either a single block (eg call ranges) or two blocks...
	int _nEndIndex[2];   // ... eg a bluff range and a value range (for betting)
private:
	double _dCumProbTable[2][GS::nHandBins]; // first=index (see above), second=weighted prob
private:
	struct {
		int nBlock;
		double *pdCumProb;
		double dHalfProb; // (when comparing a block vs a block range, assume we're half way into the given block)
	} _writeCache ;
};

//______________________________________________________________________________________________

template<class GS> std::string CEquityInfo<GS>::getSummaryDisplay() const
{
	static char sz_[256];
	std::string s;
	sprintf(sz_, "P%d: (%.2f -> %.2f: %.2f) [", GS::nPlayer, (double)_nStartIndex[0], (double)_nEndIndex[0], _dIntermediateWeight);
	s += sz_;
	for (int i = _nStartIndex[0]; i <= _nEndIndex[0]; ++i) {
		sprintf(sz_, "%d, %.2f / ", i, _dCumProbTable[0][i - _nStartIndex[0]]);
		s += sz_;
	}
	s += "] ";
	if (0 == _writeCache.nBlock) {
		sprintf(sz_, "\n(%.2f -> %.2f: %.2f) [", (double)_nStartIndex[1], (double)_nEndIndex[1], _dFinalWeight);		
		s += sz_;
		for (int i = _nStartIndex[1]; i <= _nEndIndex[1]; ++i) {
			sprintf(sz_, "%d, %.2f / ", i, _dCumProbTable[1][i - _nStartIndex[1]]);
			s += sz_;
		}
		s += "]";
	}
	return s;
}
//______________________________________________________________________________________________

template<class GS> double CEquityInfo<GS>::getEquityTimesWeight(int nH) const
{
	//TODO: Worry about "non-integer" overlaps - ie whether they "point" up or down
	//TODO: Also shouldn't I only have 0.5*equity in the bin I occupy (ugh)
	
	if (1 == _writeCache.nBlock) { // This action only appears once
			
		if (nH < _nStartIndex[0]) { 
			return 0.0;
		}
		else if (nH > _nEndIndex[0]) { 
			return _dIntermediateWeight; // (==final weight for single block)
		}
		else { // this is the worst case, we're somewhere in the middle
			return _dCumProbTable[0][nH - _nStartIndex[0]];
		} 
	}
	else { // This action has two blocks (a bluff block and a value block)
		if (nH < _nStartIndex[0]) { // (note this has to be the first clause - see c'tor)
			return 0.0;
		}
		else if (nH > _nEndIndex[1]) { // (approx as above)
			return _dFinalWeight;
		}
		else if ((nH > _nEndIndex[0]) && nH < _nStartIndex[1]) { // In the middle
			return _dIntermediateWeight;
		}
		else if (nH > _nEndIndex[0]) { // Overlapping second block
			return _dCumProbTable[1][nH - _nStartIndex[1]];
		}
		else { // Overlapping first block
			return _dCumProbTable[0][nH - _nStartIndex[0]];
		}
	}
}

//______________________________________________________________________________________________
//______________________________________________________________________________________________
//
// Player strategy encapsulations
//______________________________________________________________________________________________

template <class GS, class GS_V> class CStreetStrategy
{
public:
	// C'tor and d'tor
	CStreetStrategy();
	~CStreetStrategy();

	// Set-up game information
	void setPotSize(double dPotSize) { _dStartingPot = dPotSize; }
	const double &getPotSize() const { return _dStartingPot; }
	
	// Set-up links
	void setHalfStreet(const CStreetStrategy<GS_V, GS>* pAction_V) { _pAction_V = pAction_V; }
	void setLocation(const CStreetStrategy<GS, GS_V>* pParent_H, int nPrevAction) { _pParent_H = pParent_H; _nPrevAction = nPrevAction; }
	void setNextStreetArray(CStreetStrategy<GS, GS_V>* pNextAction_H);
	void setAllIn() { _bAllIn = true; } 
//TODO: not even sure what this does any more?!
//	void setRange();
	
	// Set initial strategy
	void setInitialStrategy(const std::vector<double> vdPrevStRange[GS::NUM_DRAWS]);
	
	// Calculate best response (modifies internal state)
	void calcBestResponse(double dAlpha);
	
	// Diagnostic information about this object
	std::string getStateDisplay() const;
	std::string getSummaryDisplay() const;
	
public:
	const double* getWeights(int nObsAction_V) const { return _dWeightTable[nObsAction_V]; }
	const CEquityInfo<GS>* getEquityInfo(int nObsAction_V) const { return _equityTable[nObsAction_V]; }
protected:
	double calcValue(int nHand_H, int nAction_H);
	void calcWeightsAndEquities_Slow();
	
protected:
	// Initial hand ranges:
	std::vector<double> _vdPrevStRange[GS::NUM_DRAWS];	

	// Values:
	std::vector<double> _vdEv[GS::nNumObsActions_V];	
		//TODO: can't calc these up-front because don't know prev st's ranges??

	// Weights, ie the cum prob of each H action given V's observable action
	double _dWeightTable[GS::nNumObsActions_V][GS::nMaxStrategies_H]; 
		//TODO: can't calc these up-front because don't know prev st's ranges??
	
	// How actions (from an unknown set) are divided up between hand bins: 
	std::vector<double> _vdThresholds[GS::nNumObsActions_V][GS::NUM_DRAWS];
		//^^^(note that these thresholds measure absolute strength, ie not adjusted by previous actions)

	// Pot at the start of this street
	double _dStartingPot;
	
	// Link to associated half street:
	const CStreetStrategy<GS_V, GS> *_pAction_V;
	
	// Link to next street
	bool _bLastStreet;
	bool _bAllIn;

	CStreetStrategy<GS, GS_V> *(_ppNextAction_H[GS::NUM_DRAWS]); // each is an array of size [GS::nCombinedActions_noFold];
	
	// Links to previous street (useful for display only?)
	int _nPrevAction; 
	const CStreetStrategy<GS, GS_V> *_pParent_H;

	// Equity information:
	CEquityInfo<GS> _equityTable[GS::nNumObsActions_V][GS::nMaxStrategies_H];
		//TODO: only need this on the final streets
};

//______________________________________________________________________________________________

// Constructor

template <class GS, class GS_V> CStreetStrategy<GS, GS_V>::CStreetStrategy()
{
	_bAllIn = false;
	_bLastStreet = true;
	for (int i = 0; i < GS::nNumObsActions_V; ++i) {
		int nActions = GS::nNumThresholds_H[i];
		_vdEv[i].resize(nActions, 0);
		for (int j = 0; j < GS::NUM_DRAWS; ++j) {
			_vdThresholds[i][j].resize(nActions);
		}
	}
	//TODO: zero the weight table?
	// _dStartingPot set by setPotSize() 
	// _pAction, _ppNextAction set by setHalfStreet() and setNextStreetArray()
	// _nPrevAction and _pParent_H set by setLocation()
	// TODO: _vdPrevStRange set by setRange() ... err which doesn't exist at the moment
}

//______________________________________________________________________________________________

// Destructor

template <class GS, class GS_V> CStreetStrategy<GS, GS_V>::~CStreetStrategy()
{
	// All default d'tors
}

//______________________________________________________________________________________________

// Given hand distributions etc calculate the weights and equities "from scratch"

template <class GS, class GS_V> void CStreetStrategy<GS, GS_V>::calcWeightsAndEquities_Slow()
{
	for (int nOA_V = 0; nOA_V < GS::nNumObsActions_V; ++nOA_V) {
		
		double *pdW = &_dWeightTable[nOA_V][0];
		CEquityInfo<GS> *pEqInfos = &_equityTable[nOA_V][0]; //(TODO: only final street) 
		CEquityInfo<GS> *pEqInfo = pEqInfos;
		
		for (int i = 0; i < GS::nMaxStrategies_H; ++i) {
			*pdW++ = 0;
			(pEqInfo++)->resetEquity();			
		}
		pdW = &_dWeightTable[nOA_V][0]; // (reset)

		for (int nD = 0; nD < GS::NUM_DRAWS; ++nD) {

			// Various temporary counts, grouped together
			int nThresh = _vdThresholds[nOA_V][nD].size();
			const double *pdT = &_vdThresholds[nOA_V][nD][0];
			double dThresh = *pdT++;
			
			const double *pdH = &_vdPrevStRange[nD][0];
			double dCumProb = 0.0;
			
			int nActTrans = 0;
			int nAction = GS::nThresh_LowerAction[nOA_V][nActTrans];			
			double dW = 0.0;

			pEqInfo = &(pEqInfos[nAction]); //(TODO: only final street)
			
			for (int nH = 0; nH < GS::nHandBins; ++nH) {
				double d = *pdH++;
				dCumProb += d;
				if (dCumProb < dThresh) { // Still inside the current action range
					dW += d; 
					if (0 == nH) {
						pEqInfo->setStartOfCumProb((double)0);
					}
					pEqInfo->accumulateProb(dW);
				}
				else { // New action, (dCumProb - d) < dThresh <= dCumProb

					// So (dThresh - (dCumProb - d)) is old and (dCumProb - dThresh) is new (summing to d) 					
/**/
// More explanation plz					
					dW += (dThresh - (dCumProb - d));
					pdW[nAction] = dW;
					
					pEqInfo->accumulateProb(dW);
					pEqInfo->setEndOfCumProb((double)nH, dW); //(TODO: handle overlap)

/**/
// More explanation plz
					dW = (dCumProb - dThresh); // The other side of the above overlap
					
					if (++nActTrans >= nThresh) { // No more transitions
						nAction = GS::nThresh_UpperAction[nOA_V][nActTrans - 1];
						dThresh = 10.0; // (ie 1.0 + safety "delta"!)
					}
					else {
						nAction = GS::nThresh_LowerAction[nOA_V][nActTrans];
							//^^^(==GS::nThresh_UpperAction[nOA_V][nActTrans-1])
						dThresh = *pdT++;
						
					} // end if looped over all transitions
/**/
// More explanation plz
					dW += pdW[nAction];

#ifdef __DEBUG_PRINT
printf("(P%d Setting new transition: %p to %p and %d: %.2f (%d)\n", GS::nPlayer, pEqInfo, &(pEqInfos[nAction]), nAction, dW, nH);	
printf("%s\n", pEqInfo->getSummaryDisplay().c_str());
#endif //__DEBUG_PRINT
					pEqInfo = &(pEqInfos[nAction]);

					pEqInfo->setStartOfCumProb((double)nH); //(TODO: handle overlap)
					pEqInfo->accumulateProb(dW);
					
				} // end if reached range transition

			} // end loop over hand bins

			pdW[nAction] = dW; // (set final threshold)
			pEqInfo->setEndOfCumProb((double)(GS::nHandBins - 1), dW);

//printf("%s\n", pEqInfo->getSummaryDisplay().c_str());
				
		} // end loop over draws		
	} // (end loop over V's observable actions)	

}

//______________________________________________________________________________________________

// Set the initial strategy with uniform thresholds

template <class GS, class GS_V> void CStreetStrategy<GS, GS_V>::setInitialStrategy(const std::vector<double> vdPrevStRange[GS::NUM_DRAWS])
{
	// Set the thresholds
		
	for (int i = 0; i < GS::nNumObsActions_V; ++i) {
		int nActions = GS::nNumThresholds_H[i];
		for (int ii = 0; ii < GS::NUM_DRAWS; ++ii) {
			double *pd = &_vdThresholds[i][ii][0];
			double d1 = 1.0/(1 + nActions); 
			for (int j = 0; j < nActions; ++j) {
				*pd++ = (1 + j)*d1;
			}
		}
	}

/**/
if (2 == GS::nPlayer) {
	_vdThresholds[0][0][0] = 0.11;
	_vdThresholds[0][0][1] = 0.78;
}
	
	// Set the hands
	//TODO: not like this?!
	for (int nD = 0; nD < GS::NUM_DRAWS; ++nD) {
		_vdPrevStRange[nD] = vdPrevStRange[nD];
	}
	
	// Set the weights and equities 

	this->calcWeightsAndEquities_Slow();
		
	// Set the hand ranges

	//TODO: how to modify these and pass them on?!
	
	// Recurse (TBD obv change hand range)
	
	if (!_bAllIn && !_bLastStreet) {
		for (int i = 0; i < GS::nCombinedActions_noFold; ++i) {
			for (int j = 0; j < GS::NUM_DRAWS; ++j) {
				_ppNextAction_H[j][i].setInitialStrategy(vdPrevStRange);
			}
		}
	}
}

//______________________________________________________________________________________________

template <class GS, class GS_V> std::string CStreetStrategy<GS, GS_V>::getStateDisplay() const
{
	static char szLine_[256];
	std::string sDisplay;
	
	sprintf(szLine_, "ptr=%p,pot=$%.1f,act=%d/", this, _dStartingPot, _nPrevAction);
	sDisplay += szLine_;
	
	return sDisplay;
}
//______________________________________________________________________________________________

template <class GS, class GS_V> std::string CStreetStrategy<GS, GS_V>::getSummaryDisplay() const
{
	static char szLine_[256];
	
	sprintf(szLine_, "P%d CStreetStrategy(%p): last=%d allin=%d\n", GS::nPlayer, this, _bLastStreet, _bAllIn);
	std::string sDisplay(szLine_);
	
	sDisplay += "State to now, reverse order: ";
	for (const CStreetStrategy<GS, GS_V>* pIt = this; pIt; pIt = pIt->_pParent_H) {
		sDisplay += pIt->getStateDisplay();
	}
	sDisplay += "\nHalf-street state: ";
	if (_pAction_V) {
		sDisplay += _pAction_V->getStateDisplay();
	}
	else {
		sDisplay += "NULL";
	}
	sDisplay += "\n";
	for (int i = 0; i < GS::nNumObsActions_V; ++i) {
		sprintf(szLine_, "Weights (obs=%d): ", i);
		sDisplay += szLine_;
		for (int k = 0; k < GS::nMaxStrategies_H; ++k) {
			sprintf(szLine_, "%.2f , ", this->_dWeightTable[i][k]);
			sDisplay += szLine_;			
		}
		sDisplay += "\n";
		for (int ii = 0; ii < GS::NUM_DRAWS; ++ii) {
			sprintf(szLine_, "Thresholds (draw=%d obs=%d): ", ii, i);
			sDisplay += szLine_;
			for (int k = 0; k < (int)_vdThresholds[i][ii].size(); ++k) {
				sprintf(szLine_, "%.2f , ", _vdThresholds[i][ii][k]);
				sDisplay += szLine_;
			}
			sDisplay += "\n";
		}
	}
	sDisplay += "\n";

	//TODO relative weights
	//TODO equities?
	//TODO starting hand range?
	
	return sDisplay;	
}

//______________________________________________________________________________________________

template <class GS, class GS_V> void CStreetStrategy<GS, GS_V>::setNextStreetArray(CStreetStrategy<GS, GS_V>* pNextAction_H) 
{ 
	_bLastStreet = false; 
	for (int nDraw = 0; nDraw < GS::NUM_DRAWS; ++nDraw) {
		_ppNextAction_H[nDraw] = pNextAction_H;
		pNextAction_H += GS::nCombinedActions_noFold;
	}
}

//______________________________________________________________________________________________

//TODO: somewhere, make sure each hand bin is >0 (say 0.001) to avoid weakly dominated solutions

template <class GS, class GS_V> double CStreetStrategy<GS, GS_V>::calcValue(int nHand_H, int nAction_H)
{
	int nObsAction_H = (nAction_H < GS::nStratToObsActionThresh_H) ? 0 : 1;
		//^^^(ie if H acts first, he must check or bet, regardless of what his final intentions are eg KC v KF)		
	int nNumStrats_V = GS::nNumStrategies_V[nObsAction_H];
	
	double dTotalWeight = 0.0, dTotalEv = 0.0;
	
	const double *pdWeights = _pAction_V->getWeights(nObsAction_H);
		//^^^(cum probabilities of V's actions)

	const CEquityInfo<GS_V>* pEqInfo = _pAction_V->getEquityInfo(nObsAction_H);
	
	const struct GS::CombinedAction *pAction = GS::combinedActionTable[nAction_H]; 
	
	for (int nS_V = 0; nS_V < nNumStrats_V; ++nS_V, ++pEqInfo) {

		double dWeight = *pdWeights++;		
		dTotalWeight += dWeight;
		
		//int nAction = pAction->nCombinedAction; // (unused)
		int nOutcome_H = pAction->nOutcome_H;
		double dCost_H = pAction->dCost_H; 
		double dCost_V = pAction->dCost_V;
		pAction++;
		
		if (GS::FOLD_H == nOutcome_H) {
			dTotalEv -= dWeight*dCost_H*_dStartingPot;

#ifdef __DEBUG_PRINT
/**/
printf("%d: strat H=%d(%d) v V=%d: HF    f(%.2f , %.2f) =-%.2f\n", nHand_H, nAction_H, nObsAction_H, nS_V, dWeight, dCost_H*_dStartingPot,dWeight*dCost_H*_dStartingPot);			
#endif //__DEBUG_PRINT
		}
		else if (GS::FOLD_V == nOutcome_H) {
			dTotalEv += dWeight*_dStartingPot*(1.0 + dCost_V); // (ie V can put money in and then fold)

#ifdef __DEBUG_PRINT
/**/
printf("%d: strat H=%d(%d) v V=%d: VF    f(%.2f , %.2f) = %.2f\n", nHand_H, nAction_H, nObsAction_H, nS_V, dWeight, dCost_V,dWeight*_dStartingPot*(1.0 + dCost_V));			
#endif //__DEBUG_PRINT
		}
		else if (_bAllIn) {
			//TODO: Something??? This one is quite complicated, let's not worry about it for the moment
		}
		else if (_bLastStreet) {
			double dEquityTimesWeight_V = pEqInfo->getEquityTimesWeight(nHand_H);
			dTotalEv += _dStartingPot*(dEquityTimesWeight_V*(1.0 + dCost_H + dCost_V) - dWeight*dCost_H);

#ifdef __DEBUG_PRINT
/**/			
printf("%d: strat H=%d(%d) v V=%d: SD eq f(%.2f , %.2f) = %.2f [H=%.2f V=%.2f]\n", 
		nHand_H, nAction_H, nObsAction_H, nS_V, dWeight, dEquityTimesWeight_V/dWeight, //=
		_dStartingPot*(dEquityTimesWeight_V*(1.0 + dCost_H + dCost_V) - dWeight*dCost_H), dCost_H, dCost_V);			
#endif //__DEBUG_PRINT

		}
		else { // Note, implies (nAction < GS::nCombinedActions_noFold)
			//TODO: something??? Not quite this:
			//for (int nDraw = 0; nDraw < GS::NUM_DRAWS; ++nDraw) {
			//	dTotalEv += GS::dDrawProbs[nDraw]*_ppNextAction_H[nDraw][nAction].getCachedValue(nHand_H)*dWeight;
			//}
		}
				
	} // end loop over V counter-strategies
	
	if (dTotalWeight > 0.0) {
		dTotalEv /= dTotalWeight;
	}
	return dTotalEv;
}

//______________________________________________________________________________________________

template <class GS, class GS_V> void CStreetStrategy<GS, GS_V>::calcBestResponse(double dAlpha)
{	
//	double dNegAlpha = 1.0 - dAlpha;
	
	static double dNewThreshTable_[GS::nNumObsActions_V][GS::nMaxThresholds_H];
	
	//TODO: first attempt, not updating anything, just calculating BR
	//TODO: also not handling ranges of <1 bin
	//TODO: handle draws..
	int nDraw = 0;
	
	for (int nObsAction_V = 0; nObsAction_V < GS::nNumObsActions_V; ++nObsAction_V)
	{
		// Per "V obs action":
		int nThresh = 0;
		const int *pnCandidateActions1 = GS::nThresh_LowerAction[nObsAction_V];
		const int *pnCandidateActions2 = GS::nThresh_UpperAction[nObsAction_V];

		int nNumThresholds_H = GS::nNumThresholds_H[nObsAction_V];
		double *pdThresh = &dNewThreshTable_[nObsAction_V][0];
		
		// Per decision:
		int nCandidateAction1 = *pnCandidateActions1++;
		int nCandidateAction2 = *pnCandidateActions2++;
		
		// Loop over hands
		double dEvCandidate1_prev = 0.0, dEvCandidate2_prev = 0.0;
		double dCumProb = 0.0, dCumProb_diff = 0.0, dCumProb_diff_prev;
		for  (int nH = 0; nH < GS::nHandBins; ++nH) {
			dCumProb_diff_prev = dCumProb_diff; 
			dCumProb_diff = this->_vdPrevStRange[nDraw][nH];

			double dEvCandidate1 = this->calcValue(nH, nCandidateAction1); 
			double dEvCandidate2 = this->calcValue(nH, nCandidateAction2);
			
#ifdef __DEBUG_PRINT
/**/			
printf("P%d %d: >>>ACTION %d v %d: %.2f v %.2f\n", GS::nPlayer, nH, nCandidateAction1, nCandidateAction2, dEvCandidate1, dEvCandidate2);			
#endif //__DEBUG_PRINT
			
			if (dEvCandidate2 > dEvCandidate1) { // Change in decision

				// Calculate exact transition point using linear interpolation
				// EV1p + t*(EV1n - EV1p) = EV2p + t*(EV2n - EV2p) [0 < t <= 1, t==0.5 is the boundary]
				// t*((EV1n - EV1p) - (EV2n - EV2p)) = EV2p - EV1p  
				double dThresh = (dEvCandidate1 - dEvCandidate1_prev) - (dEvCandidate2 - dEvCandidate2_prev); 
				if ((dThresh < 1.0e-6) && (dThresh > -1.0e-6)) { // denominator is 0 so it doesn't matter where the threshold is
					dThresh = 0.5; // (ie make it on the boundary)					
				}
				else {
					dThresh = (dEvCandidate2_prev - dEvCandidate1_prev)/dThresh;
				}
				dThresh -= 0.5;
				//*pdThresh++ = dThresh;
				/**/
				//TODO: actually want to update this separately
				if (dThresh < 0.0) {
					*pdThresh++ = dCumProb + dThresh*dCumProb_diff_prev;					

#ifdef __DEBUG_PRINT
/**/
printf("TRANSITION: \"index\" %.1f and cum %.2f -> %.2f\n", dThresh, dCumProb, dCumProb + dThresh*dCumProb_diff_prev); 
#endif //__DEBUG_PRINT
				}
				else {
					*pdThresh++ = dCumProb + dThresh*dCumProb_diff;

#ifdef __DEBUG_PRINT
/**/
printf("TRANSITION: \"index\" %.1f and cum %.2f -> %.2f\n", dThresh, dCumProb, dCumProb + dThresh*dCumProb_diff); 
#endif //__DEBUG_PRINT
				}
				//TODO: this isn't quite right ... dCumProb is at the boundary so at dThresh==0.5 not dThresh==0.0
				dCumProb += dCumProb_diff;
				if (++nThresh < nNumThresholds_H) {
					nCandidateAction1 = *pnCandidateActions1++;
					nCandidateAction2 = *pnCandidateActions2++;
					
					dEvCandidate1_prev = dEvCandidate2;
					dEvCandidate2_prev = this->calcValue(nH, nCandidateAction2);					
				}
				else { // No more decisions, just calculate values and weights
					break;
				}			
			} // end if reached indifference point
			else {
				dCumProb += dCumProb_diff;
				dEvCandidate1_prev = dEvCandidate1;
				dEvCandidate2_prev = dEvCandidate2;

			} // end if status quo
						
		} // end loop over H hands		

		/**/// Do this better....
		for (int nThresh = 0; nThresh < nNumThresholds_H; ++nThresh) {
//			_vdThresholds[nObsAction_V][0][nThresh] = dNewThreshTable_[nObsAction_V][nThresh];		
			_vdThresholds[nObsAction_V][0][nThresh] =					
				dAlpha*dNewThreshTable_[nObsAction_V][nThresh]
				+ (1.0 - dAlpha)*_vdThresholds[nObsAction_V][0][nThresh];
#ifdef __DEBUG_PRINT
/**/
printf("P%d: new thresh @ %d = %.2f\n", GS::nPlayer, nThresh, dNewThreshTable_[nObsAction_V][nThresh]);			
#endif //__DEBUG_PRINT
		}
		
	} // end loop over V obs actions

	/**/
	//TODO: can I do this incrementally?
	this->calcWeightsAndEquities_Slow();
	
}

//______________________________________________________________________________________________
//______________________________________________________________________________________________
//
// Gluing the indiviudal streets together to make a complete game
//______________________________________________________________________________________________

//TODO make the number of streets a template array to help with loop unrolling etc?

template <class GSP1, class GSP2> class CMultiStreetTwoPlayerGame
{
public:
	CMultiStreetTwoPlayerGame(double dStartPot, double dEffectiveStack, int nStreets);
	~CMultiStreetTwoPlayerGame();
	
	// Main processing function
	void calculateCoOptimalStrategies(const std::vector<double> vdP1Preflop[GSP1::NUM_DRAWS], 
										const std::vector<double> vdP2Preflop[GSP1::NUM_DRAWS]);

	// Diagnostic display
	std::string getSummaryDisplay() const;
	
protected:
	// Sets all the "next street" pointers up correctly
	void recurseSetup(int nStreet,
			CStreetStrategy<GSP1, GSP2> *pCurrAction_P1,
			CStreetStrategy<GSP2, GSP1> *pCurrAction_P2);

protected:	
	// Utility: allocate contiguous-ish strategy blocks
	CStreetStrategy<GSP1, GSP2> *getNextBlock_P1(int nStreet);
	CStreetStrategy<GSP2, GSP1> *getNextBlock_P2(int nStreet);

protected:	
	double _dCostTable[GSP1::nCombinedActions];
	double _dEffectiveStack;
	int _nStreets;

protected:	
	// Ordered in reverse 	
	std::vector<std::vector<CStreetStrategy<GSP1, GSP2> > > _vvP1Strategies;
	std::vector<std::vector<CStreetStrategy<GSP2, GSP1> > > _vvP2Strategies;
};

//______________________________________________________________________________________________

// Constructor

template <class GSP1, class GSP2> CMultiStreetTwoPlayerGame<GSP1, GSP2>::CMultiStreetTwoPlayerGame(double dStartPot, double dEffectiveStack, int nStreets)
{
	_nStreets = nStreets;
	_dEffectiveStack = dEffectiveStack;
	// Check that the game is "logical"  
	if (GSP1::nCombinedActions_noFold != GSP2::nCombinedActions_noFold) {
		throw std::exception();
	}
	if (GSP1::NUM_DRAWS != GSP2::NUM_DRAWS) {
		throw std::exception();		
	}
	if ((-1.0 != GSP1::dDrawProbs[GSP1::NUM_DRAWS])
			||(-1.0 != GSP2::dDrawProbs[GSP2::NUM_DRAWS]))
	{
		throw std::exception();				
	}
	
	_vvP1Strategies.resize(nStreets);
	_vvP2Strategies.resize(nStreets);
	
	_vvP1Strategies[0].resize(1);
	_vvP2Strategies[0].resize(1);

	// Calculate the number of different strategy objects
	int nTemp = 1;
	for (int i = 1; i < nStreets; ++i) {
		nTemp *= (GSP1::nCombinedActions_noFold*GSP1::NUM_DRAWS);
		_vvP1Strategies[i].reserve(nTemp);
		_vvP2Strategies[i].reserve(nTemp);
	}	
	// (So, stepping back through the strategies ... you have
	//  1, draws*actions, (draws*actions)^2, ..., (draws*actions)^n)  

	// For the calculations below, set-up an action->cost table
	//TODO: do I care about folds here? Also seem to be visiting "nCombAct" entries > once?
	for (int i = 0; i < GSP1::nMaxStrategies_H; ++i) {
		for (int j = 0; j < GSP1::nMaxStrategies_V; ++j) {
			const struct GSP1::CombinedAction *pCombAct = &(GSP1::combinedActionTable[i][j]);
			_dCostTable[pCombAct->nCombinedAction] = (1.0 + pCombAct->dCost_H + pCombAct->dCost_V);				
		}
	} // (end loop over P1/P2 actions)

	// And sanity check the game structure one final time...
	for (int i = 0; i < GSP2::nMaxStrategies_H; ++i) {
		for (int j = 0; j < GSP2::nMaxStrategies_V; ++j) {
			const struct GSP2::CombinedAction *pCombAct = &(GSP2::combinedActionTable[i][j]);
			double dCost = (1.0 + pCombAct->dCost_H + pCombAct->dCost_V);
			if (_dCostTable[pCombAct->nCombinedAction] != dCost) {
				throw std::exception();				 
			}
		}
	} // (end loop over P1/P2 actions)
	
	// Only fiddly bit: set-up all the links between streets
	CStreetStrategy<GSP1, GSP2> *pCurrAction_P1 = &_vvP1Strategies[0][0];
	CStreetStrategy<GSP2, GSP1> *pCurrAction_P2 = &_vvP2Strategies[0][0];
	
	pCurrAction_P1->setLocation(0, -1);
	pCurrAction_P1->setPotSize(dStartPot);
	pCurrAction_P2->setLocation(0, -1);
	pCurrAction_P2->setPotSize(dStartPot);
	pCurrAction_P1->setHalfStreet(pCurrAction_P2);
	pCurrAction_P2->setHalfStreet(pCurrAction_P1);
	
	this->recurseSetup(1, pCurrAction_P1, pCurrAction_P2);
}

//______________________________________________________________________________________________

// Destructor

template <class GSP1, class GSP2> CMultiStreetTwoPlayerGame<GSP1, GSP2>::~CMultiStreetTwoPlayerGame()
{
	// All just happens by default d'tors
}

//______________________________________________________________________________________________

template <class GSP1, class GSP2> CStreetStrategy<GSP1, GSP2> *CMultiStreetTwoPlayerGame<GSP1, GSP2>::getNextBlock_P1(int nStreet)
{ 
	std::vector<CStreetStrategy<GSP1, GSP2> > *pv = &(_vvP1Strategies[nStreet]); 
	int nCurrPtr = pv->size();
	pv->resize(nCurrPtr + (GSP1::nCombinedActions_noFold*GSP1::NUM_DRAWS));
	return &(pv->at(nCurrPtr)); 
}	

//______________________________________________________

template <class GSP1, class GSP2> CStreetStrategy<GSP2, GSP1> *CMultiStreetTwoPlayerGame<GSP1, GSP2>::getNextBlock_P2(int nStreet)
{ 
	std::vector<CStreetStrategy<GSP2, GSP1> > *pv = &(_vvP2Strategies[nStreet]); 
	int nCurrPtr = pv->size();
	pv->resize(nCurrPtr + (GSP1::nCombinedActions_noFold*GSP1::NUM_DRAWS));
	return &(pv->at(nCurrPtr)); 
}	

//______________________________________________________________________________________________

template <class GSP1, class GSP2> void CMultiStreetTwoPlayerGame<GSP1, GSP2>::
										recurseSetup(int nStreet,
														CStreetStrategy<GSP1, GSP2> *pCurrAction_P1,
														CStreetStrategy<GSP2, GSP1> *pCurrAction_P2)
{
	if (nStreet >= _nStreets) { // (this test isn't exact, but it should work anyway, eg 3^3 > 1+3+3^2 etc)
		return;
	}
	CStreetStrategy<GSP1, GSP2> *pNextAction_P1;
	CStreetStrategy<GSP2, GSP1> *pNextAction_P2;
	
	pCurrAction_P1->setNextStreetArray((pNextAction_P1 = this->getNextBlock_P1(nStreet)));
	pCurrAction_P2->setNextStreetArray((pNextAction_P2 = this->getNextBlock_P2(nStreet)));

	for (int nAct = 0; nAct < GSP1::nCombinedActions_noFold; ++nAct) {
		for (int nDraw = 0; nDraw < GSP1::NUM_DRAWS; ++nDraw) {
			pNextAction_P1->setHalfStreet(pNextAction_P2);
			pNextAction_P1->setLocation(pCurrAction_P1, nAct);
			pNextAction_P1->setPotSize(pCurrAction_P1->getPotSize()*_dCostTable[nAct]);
			pNextAction_P2->setHalfStreet(pNextAction_P1);
			pNextAction_P2->setLocation(pCurrAction_P2, nAct);
			pNextAction_P2->setPotSize(pCurrAction_P2->getPotSize()*_dCostTable[nAct]);

			if ((pNextAction_P1->getPotSize() > _dEffectiveStack)
					||(pNextAction_P2->getPotSize() > _dEffectiveStack))
			{
				pNextAction_P1->setAllIn();					
				pNextAction_P2->setAllIn();					
			}
			else {
				this->recurseSetup(nStreet + 1, pNextAction_P1++, pNextAction_P2++);				
			}
		} // end loop over draws
	} // end loop over combined actions
}

//______________________________________________________________________________________________

template <class GSP1, class GSP2> void CMultiStreetTwoPlayerGame<GSP1, GSP2>::
						calculateCoOptimalStrategies(const std::vector<double> vdP1Preflop[GSP1::NUM_DRAWS], 
														const std::vector<double> vdP2Preflop[GSP1::NUM_DRAWS])
{
	static const int nMaxIts_ = 1; // TODO: work out how to calculate this automatically...
	
	_vvP1Strategies[0][0].setInitialStrategy(vdP1Preflop);
	_vvP2Strategies[0][0].setInitialStrategy(vdP2Preflop);
	
/**/
for (int nIts = 1;; nIts++) {
	std::string s;
	s = _vvP1Strategies[0][0].getSummaryDisplay();
	s += _vvP2Strategies[0][0].getSummaryDisplay();
	printf("%s\n", s.c_str());
	int nD;
	scanf("%d", &nD);

	_vvP1Strategies[0][0].calcBestResponse(1.0/nIts);
	_vvP2Strategies[0][0].calcBestResponse(1.0/nIts);
}
return;	
	
	for (int nIts = 1; nIts < nMaxIts_; ++nIts) {
		
		double dAlpha = 1.0/nIts;

		for (int i = _nStreets - 1; i > 1; ++i) {
			// (Needs to be in reverse order so that the EV of future streets can be used by earlier ones)
			//TODO: actually v strong argument to do it forwards so can only cache values for action/street combos that
			//are actually used! Do need some clever way of working out what's been cached and what hasn't though...
			
			CStreetStrategy<GSP1, GSP2>* pP1 = &_vvP1Strategies[i][0];
			int nStratsP1 = _vvP1Strategies[i].size();
			for (int j = 0; j < nStratsP1; ++j) {
				(pP1++)->calcBestResponse(dAlpha);
			}
			CStreetStrategy<GSP2, GSP1>* pP2 = &_vvP2Strategies[i][0];
			int nStratsP2 = _vvP2Strategies[i].size();
			for (int j = 0; j < nStratsP2; ++j) {
				(pP2++)->calcBestResponse(dAlpha);
			}
		}
		_vvP1Strategies[0][0].calcBestResponse(dAlpha);
		_vvP2Strategies[0][0].calcBestResponse(dAlpha);
		
	} // (end loop over iterations)
}

//______________________________________________________________________________________________

template <class GSP1, class GSP2> std::string CMultiStreetTwoPlayerGame<GSP1, GSP2>::getSummaryDisplay() const
{
	std::string sDisplay;
	static char szLine_[256];
	
	for (int i = 0; i < _nStreets; ++i) {
		
		sprintf(szLine_, "Street = %d\n\n", i);
		sDisplay += szLine_;
		sDisplay += "PLAYER 1:\n\n";
		const CStreetStrategy<GSP1, GSP2>* pP1 = &_vvP1Strategies[i][0];
		int nStratsP1 = _vvP1Strategies[i].size();
		for (int j = 0; j < nStratsP1; ++j) {
			sDisplay += (pP1++)->getSummaryDisplay();
		}
		sDisplay += "PLAYER 2:\n\n";
		const CStreetStrategy<GSP2, GSP1>* pP2 = &_vvP2Strategies[i][0];
		int nStratsP2 = _vvP2Strategies[i].size();
		for (int j = 0; j < nStratsP2; ++j) {
			sDisplay += (pP2++)->getSummaryDisplay();
		}
	}
	return sDisplay;
}

//______________________________________________________________________________________________
//______________________________________________________________________________________________

int main(int argc, char** argv)
{
	// (10 hand bins,) Uniform distribution
	
	std::vector<double> vdP1[CGameStructure_Simple_P1::NUM_DRAWS];
	std::vector<double> vdP2[CGameStructure_Simple_P2::NUM_DRAWS];
	
	for (int i = 0; i < CGameStructure_Simple_P1::NUM_DRAWS; ++i) {
		vdP1[i].resize(CGameStructure_Simple_P1::nHandBins, 1.0/CGameStructure_Simple_P1::nHandBins);
	}
	for (int i = 0; i < CGameStructure_Simple_P2::NUM_DRAWS; ++i) {
		vdP2[i].resize(CGameStructure_Simple_P1::nHandBins, 1.0/CGameStructure_Simple_P1::nHandBins);
	}
	
	// Game
	
	CMultiStreetTwoPlayerGame<CGameStructure_Simple_P1, CGameStructure_Simple_P2> multiStGame(12.0, 200.0, 1);
		// 1 street, $200 effective stacks, $12 in the pot
		
	multiStGame.calculateCoOptimalStrategies(vdP1, vdP2);
	
	std::string s = multiStGame.getSummaryDisplay();
	printf("%s\n", s.c_str());

	return 0;
}

//______________________________________________________________________________________________
//______________________________________________________________________________________________
