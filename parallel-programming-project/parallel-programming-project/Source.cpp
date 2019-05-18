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

	const int MAX_NUMBER_PLAYER = 10;
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

	cout << "size " << toPlay.size() << endl;
	while (toPlay.size() > 1 && !hasLost)
	{
		put_back = false;

		if (toPlay.size() % 2 != 0)
		{
			put_back = true;
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


			maxScore = -1; maxScoreIndex = -1; maxScoreRank = -1;
			for (int i = 0; i < counter/2; i++)
				cout << incomingScores[i] << " ";
			cout << endl;
			// loop on incoming scores and find the max
			for (int i = 0; i < counter/2; i++) {
				if (maxScore < incomingScores[i]) {
					maxScore = incomingScores[i];
					maxScoreIndex = i;
					maxScoreRank = stats[i].MPI_SOURCE;
				}
			}

			cout << "max rank is " << maxScoreRank << " with score " << maxScore << " at index " << maxScoreIndex << endl;
		}
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