#pragma once
class Manager
{
public:
	// Constants
	const static int MAX_SIZE = 20; // max number of players
	const static int MAX_ROUND_NUMBER = 10; // max number of rounds
	// Variables
	int playersNum = 0;
	int* round = new int[MAX_SIZE];

	int roundsNum = 0;
	int** allRounds = new int*[MAX_ROUND_NUMBER];

	bool* results = new bool[MAX_SIZE]; // to store who wins and who loses

	// Methods
	Manager();
	~Manager();
	
	void addPlayer(int rank);

	void addRound(int* newRound);
};

