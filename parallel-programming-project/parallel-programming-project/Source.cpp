#include <mpi.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <ctime>

using namespace std;

int generateRandNumber(int, int);
int generateScore();
int play();
int findPlayerWithMaxScore(vector<int>);
void startRound(int, vector<int>&);
void endRoundAndRemoveLosers(vector<int>&);
void printVec(vector<int>);
int findIndex(vector<int>, int);


// global variables
int myRank, myScore;
bool hasLost = false, opponentLost = false, put_back = false;
// constants
const int MAX_NUMBER_PLAYER = 10, MAX_REQ = 100000, MAX_STAT = 100000;

int main(int argc, char** argv) {
	// Initialize the MPI environment
	MPI_Init(&argc, &argv);
	MPI_Status stat;
	vector<int> toPlay, temp;
	// Get the number of processes
	int numberOfPlayers, rc, incomingScore = -1, asyncCount = 0;
	
	MPI_Comm_size(MPI_COMM_WORLD, &numberOfPlayers);
	
	// Get the rank of the process
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	srand(time(nullptr) + myRank);
	myScore = generateScore();
    // Initializing toPlay array for first round
	for (int i = 0; i < numberOfPlayers; i++)
	{
		toPlay.push_back(i);
	}

	while (toPlay.size() > 1 && !hasLost)
	{
		put_back = false;

		if (toPlay.size() % 2 != 0)
		{
			put_back = true;
			// find index the player with max score
			int maxScoreIndex = findPlayerWithMaxScore(toPlay);
			// delete the player with max score from toPlay array
			toPlay.erase(toPlay.begin() + maxScoreIndex);
		}

		// find my index
		int myIndex = findIndex(toPlay, myRank);
		// start round
		startRound(myIndex, toPlay);

		// end round and remove losers
		endRoundAndRemoveLosers(toPlay);

	}

	// Finalize the MPI environment.
	MPI_Finalize();
}


int generateRandNumber(int min, int max) {
	int randNum = rand() % (max - min + 1) + min;
	return randNum;
}

int generateScore() {
	return generateRandNumber(10, 20);
}

int play() {
	return generateRandNumber(5, 10);
}


int findPlayerWithMaxScore(vector<int> toPlay) {
	const int n = MAX_NUMBER_PLAYER * 2;
	MPI_Request reqs[n];
	MPI_Status stats[n];
	int incomingScores[MAX_NUMBER_PLAYER];
	int counter = 0;

	for (auto it = toPlay.begin(); it != toPlay.end(); ++it)
	{
		int i = distance(toPlay.begin(), it);
		// receive async
		if (toPlay[i] != myRank)
		{
			MPI_Irecv(&incomingScores[counter], 1, MPI_INT, toPlay[i], 0, MPI_COMM_WORLD, &reqs[counter]);
			counter++;
		}
	}

	for (auto it = toPlay.begin(); it != toPlay.end(); ++it)
	{
		int i = distance(toPlay.begin(), it);
		// send async
		if (toPlay[i] != myRank)
		{
			MPI_Isend(&myScore, 1, MPI_INT, toPlay[i], 0, MPI_COMM_WORLD, &reqs[counter]);
			counter++;
		}
	}

	MPI_Waitall(counter, reqs, stats);


	int maxScore = -1, maxScoreIndex = -1, maxScoreRank = -1;

	for (int i = 0; i < counter / 2; i++)
		cout << incomingScores[i] << " ";
	cout << endl;
	// loop on incoming scores and find the max
	for (int i = 0; i < counter / 2; i++) {
		if (maxScore < incomingScores[i]) {
			maxScore = incomingScores[i];
			maxScoreIndex = i;
			maxScoreRank = stats[i].MPI_SOURCE;
		}
	}

	cout << "max rank is " << maxScoreRank << " with score " << maxScore << " at index " << maxScoreIndex << endl;

	return maxScoreIndex;
}

void startRound(int myIndex, vector<int> &toPlay) {
	hasLost = false;
	opponentLost = false;
	MPI_Request reqs[2];
	MPI_Status stats[2];

	if (myIndex % 2 == 0)
	{
		// if my index is even
		// then i will send then receive
		while (!hasLost && !opponentLost)
		{
			int myGuessedNumber = play(), opponentGuessedNumber;
			MPI_Irecv(&opponentGuessedNumber, 1, MPI_INT, toPlay[myIndex + 1], 1, MPI_COMM_WORLD, &reqs[0]);
			MPI_Isend(&myGuessedNumber, 1, MPI_INT, toPlay[myIndex + 1], 1, MPI_COMM_WORLD, &reqs[1]);

			MPI_Waitall(2, reqs, stats);

			if (myGuessedNumber <= opponentGuessedNumber)
			{
				myScore--;
				hasLost = myScore == 0;
			}
			else
			{
				myScore++;
			}

			// send game result to other player
			MPI_Irecv(&opponentLost, 1, MPI_C_BOOL, toPlay[myIndex + 1], 2, MPI_COMM_WORLD, &reqs[0]);
			MPI_Isend(&hasLost, 1, MPI_C_BOOL, toPlay[myIndex + 1], 2, MPI_COMM_WORLD, &reqs[1]);

			MPI_Waitall(2, reqs, stats);

			if (hasLost) {
				cout << "Player No. " << myRank << " has lost to player No. " << toPlay[myIndex + 1] << endl;
			}

			/*if (opponentLost)
			{
				//cout << "before " << toPlay.size() << endl;
				toPlay.erase(toPlay.begin() + (myIndex + 1));
				//cout << "after " << toPlay.size() << endl;
			}*/
		}
	}
	else
	{
		// if my index is odd
		// then i will receive then send
		while (!hasLost && !opponentLost)
		{
			int myGuessedNumber = play(), opponentGuessedNumber;
			MPI_Irecv(&opponentGuessedNumber, 1, MPI_INT, toPlay[myIndex - 1], 1, MPI_COMM_WORLD, &reqs[0]);
			MPI_Isend(&myGuessedNumber, 1, MPI_INT, toPlay[myIndex - 1], 1, MPI_COMM_WORLD, &reqs[1]);

			MPI_Waitall(2, reqs, stats);

			if (myGuessedNumber <= opponentGuessedNumber)
			{
				myScore--;
				hasLost = myScore == 0;
			}
			else
			{
				myScore++;
			}

			// send game result to other player
			MPI_Irecv(&opponentLost, 1, MPI_C_BOOL, toPlay[myIndex - 1], 2, MPI_COMM_WORLD, &reqs[0]);
			MPI_Isend(&hasLost, 1, MPI_C_BOOL, toPlay[myIndex - 1], 2, MPI_COMM_WORLD, &reqs[1]);

			MPI_Waitall(2, reqs, stats);

			if (hasLost) {
				cout << "Player No. " << myRank << " has lost to player No. " << toPlay[myIndex - 1] << endl;
			}

			/*if (opponentLost)
			{
				//cout << "before " << toPlay.size() << endl;
				toPlay.erase(toPlay.begin() + (myIndex - 1));
				//cout << "after " << toPlay.size() << endl;
			}*/
		}
	}
}

void endRoundAndRemoveLosers(vector<int> &toPlay) {
	const int n = MAX_NUMBER_PLAYER * 2;
	MPI_Request reqs[n];
	MPI_Status stats[n];
	bool playerLost[MAX_NUMBER_PLAYER] = { 0 };
	int counter1 = 0, counter2 = 0;

	// receive match result from other players
	for (vector<int>::iterator it = toPlay.begin(); it != toPlay.end(); ++it) {
		if (*it != myRank)
		{
			MPI_Irecv(&playerLost[counter2], 1, MPI_C_BOOL, *it, 3, MPI_COMM_WORLD, &reqs[counter1]);
			counter1++;
			counter2++;
		}
	}

	// send match result to other players
	for (vector<int>::iterator it = toPlay.begin(); it != toPlay.end(); ++it)
	{
		if (*it != myRank)
		{
			MPI_Isend(&hasLost, 1, MPI_C_BOOL, *it, 3, MPI_COMM_WORLD, &reqs[counter1]);
			counter1++;
		}
	}

	MPI_Waitall(counter1, reqs, stats);

	// if player lost delete him from toPlay array
	for (int i = 0; i < counter1/2; i++) {
		if (playerLost[i]) {

			int rank = stats[i].MPI_SOURCE;
			// find player by rank
			int index = findIndex(toPlay, rank);

			// remove player
			toPlay.erase(toPlay.begin() + index);
		}
	}
}

void printVec(vector<int> myVec) {
	for (auto it = myVec.begin(); it != myVec.end(); ++it) {
		cout << *it << " ";
	}
	cout << endl;
}

int findIndex(vector<int> vec, int value) {
	vector<int>::iterator it = find(vec.begin(), vec.end(), value);
	return distance(vec.begin(), it);
}