#include "Manager.h"


Manager::Manager()
{
}


Manager::~Manager()
{
}


void Manager::addPlayer(int rank) {
	round[playersNum] = rank;
	playersNum++;
}


void Manager::addRound(int* newRound) {
	allRounds[roundsNum] = newRound;
	roundsNum++;
}