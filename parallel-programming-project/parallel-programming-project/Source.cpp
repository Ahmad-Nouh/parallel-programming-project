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

int main(int argc, char** argv) {
	// Initialize the MPI environment
	MPI_Init(&argc, &argv);
	MPI_Status stat;
	vector<int> toPlay, temp;
	// Get the number of processes
	int numberOfPlayers, rc, myRank, incomingScore = -1, maxScore = -1, maxScoreIndex, maxScoreRank, asyncCount = 0;
	bool hasLost = false, put_back = false;
	MPI_Comm_size(MPI_COMM_WORLD, &numberOfPlayers);
	
	// Get the rank of the process
	MPI_Comm_rank(MPI_COMM_WORLD, &myRank);
	srand(time(nullptr) + myRank);
	int myScore = generateScore();
    // Initializing toPlay array for first round
	for (int i = 0; i < numberOfPlayers; i++)
	{
		toPlay.push_back(i);
	}

	//cout << "size before " << toPlay.size() << endl;

	while (toPlay.size() > 1 && !hasLost)
	{
		// cout << "rank " << myRank << " size " << toPlay.size() << endl;
		put_back = false;

		if (toPlay.size() % 2 != 0)
		{
			put_back = true;
			// send my score to other players (except me)
			for (auto it = toPlay.begin(); it != toPlay.end(); ++it)
			{
				int i = distance(toPlay.begin(), it);

				if (toPlay[i] != myRank)
				{
					MPI_Send(&myScore, 1, MPI_INT, toPlay[i], 0, MPI_COMM_WORLD);
				}
			}

			// receive other scores (except me)
			// delete the player of the highest score from toPlay
			for (auto it = toPlay.begin(); it != toPlay.end(); ++it)
			{
				int i = distance(toPlay.begin(), it);

				if (toPlay[i] != myRank)
				{
					MPI_Recv(&incomingScore, 1, MPI_INT, toPlay[i], 0, MPI_COMM_WORLD, &stat);
				}

				if (incomingScore > maxScore) {
					maxScore = incomingScore;
					maxScoreIndex = i;
					maxScoreRank = toPlay[i];
				}
			}

			toPlay.erase(toPlay.begin() + maxScoreIndex);
		}
		cout << "maxScore " << maxScore << endl;
		cout << "maxScoreRank " << maxScoreRank << endl;

		
		// find my index
		vector<int>::iterator it = find(toPlay.begin(), toPlay.end(), myRank);
		int index = distance(toPlay.begin(), it);
		bool opponentLost = false;

		if (index % 2 == 0)
		{
			// if my index is even
			// then i will send then receive
			while (!hasLost && !opponentLost)
			{
				int myGuessedNumber = play(), opponentGuessedNumber;
				MPI_Send(&myGuessedNumber, 1, MPI_INT, toPlay[index + 1], 1, MPI_COMM_WORLD);
				MPI_Recv(&opponentGuessedNumber, 1, MPI_INT, toPlay[index + 1], 1, MPI_COMM_WORLD, &stat);

				if (myGuessedNumber <= opponentGuessedNumber)
				{
					myScore--;
					hasLost = myScore == 0;
				}
				else 
				{
					myScore++;
				}

				//if (myRank == 0) {
				//	cout << "My score: " << myScore << endl;
				//}

				// send game result to other player
				MPI_Send(&hasLost, 1, MPI_C_BOOL, toPlay[index + 1], 2, MPI_COMM_WORLD);
				MPI_Recv(&opponentLost, 1, MPI_C_BOOL, toPlay[index + 1], 2, MPI_COMM_WORLD, &stat);

				if (hasLost) {
					cout << "Player No. " << myRank << " has lost to player No. " << toPlay[index + 1] << endl;
				}

				if (opponentLost)
				{
					//cout << "before " << toPlay.size() << endl;
					toPlay.erase(toPlay.begin() + (index + 1));
					//cout << "after " << toPlay.size() << endl;
				}
			}
		}
		else
		{
			// if my index is odd
			// then i will receive then send
			while (!hasLost && !opponentLost)
			{
				int myGuessedNumber = play(), opponentGuessedNumber;
				MPI_Recv(&opponentGuessedNumber, 1, MPI_INT, toPlay[index - 1], 1, MPI_COMM_WORLD, &stat);
				MPI_Send(&myGuessedNumber, 1, MPI_INT, toPlay[index - 1], 1, MPI_COMM_WORLD);

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
				MPI_Recv(&opponentLost, 1, MPI_C_BOOL, toPlay[index - 1], 2, MPI_COMM_WORLD, &stat);
				MPI_Send(&hasLost, 1, MPI_C_BOOL, toPlay[index - 1], 2, MPI_COMM_WORLD);
				
				if (hasLost) {
					cout << "Player No. " << myRank << " has lost to player No. " << toPlay[index - 1] << endl;
				}

				if (opponentLost)
				{
					//cout << "before " << toPlay.size() << endl;
					toPlay.erase(toPlay.begin() + (index - 1));
					//cout << "after " << toPlay.size() << endl;
				}
			}
		}


		// send match result to other players
		for (vector<int>::iterator it = toPlay.begin(); it != toPlay.end(); ++it)
		{
			int i = distance(toPlay.begin(), it);

			if (*it != myRank)
			{
				MPI_Send(&hasLost, 1, MPI_C_BOOL, *it, 3, MPI_COMM_WORLD);
			}
		}

		/*cout << "before my rank = " << myRank << endl;
		for (vector<int>::iterator it = toPlay.begin(); it != toPlay.end(); ++it) {
			cout << " " << *it;
		}
		cout << endl;*/

		// receive match result from other players
		for (vector<int>:: iterator it = toPlay.begin(); it != toPlay.end(); ++it) {
			if (*it != myRank)
			{
				MPI_Recv(&opponentLost, 1, MPI_C_BOOL, *it, 3, MPI_COMM_WORLD, &stat);

				// if opponentLost delete him from toPlay array
				if (opponentLost)
				{
					it = toPlay.erase(it);

					if (it == toPlay.end()) {
						break;
					}
					
				}

			}
		}
		
		
		/*cout << "after my rank = " << myRank << endl;
		for (vector<int>::iterator it = toPlay.begin(); it != toPlay.end(); ++it) {
			cout << " " << *it;
		}
		cout << endl;*/


		/*
		it = toPlay.begin();
		while (it != toPlay.end())
		{
			int i = distance(toPlay.begin(), it);

			if (toPlay[i] != myRank)
			{
				MPI_Recv(&opponentLost, 1, MPI_C_BOOL, toPlay[i], 3, MPI_COMM_WORLD, &stat);

				if (!opponentLost)
				{
					temp.push_back(toPlay[i]);
				}
				
			}
			
			it++;
		}
		toPlay = temp;
		*/
		//if (put_back) {
		//	toPlay.insert(toPlay.begin() + maxScoreIndex + 1, maxScoreRank);
		//}
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