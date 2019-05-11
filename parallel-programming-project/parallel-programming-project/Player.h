#pragma once
class Player
{
public:
	// Constants
	const static int MAX_SIZE = 100; // max number of matches
	// Variables
	std::string name;
	int rank;

	int recordsSize = 0;
	Record *records = new Record[MAX_SIZE];

	// Methods
	Player();
	~Player();

	int play();
	void sendMessage(int &dataToSend, int destRank, int tag, MPI_Request &req);
	void receiveMesssage(int &buf, int srcRank, int tag, MPI_Request &req);
	void addRecord(Record newRecord);
};

