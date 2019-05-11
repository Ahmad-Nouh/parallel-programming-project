#include "Player.h"
#include "Record.h"
#include <iostream>
#include <mpi.h>

Player::Player()
{
}


Player::~Player()
{
}

int Player::play() {
	int min = 0;
	int max = 10;
	int randNum = rand() % (max - min + 1) + min;
	return randNum;
}


void Player::sendMessage(int &dataToSend, int destRank, int tag, MPI_Request &req) {
	MPI_Isend(&dataToSend, 1, MPI_INT, destRank, tag, MPI_COMM_WORLD, &req);
}

void Player::receiveMesssage(int &buf, int srcRank, int tag, MPI_Request &req) {
	MPI_Irecv(&buf, 1, MPI_INT, srcRank, tag, MPI_COMM_WORLD, &req);
}

void Player::addRecord(Record newRecord) {
	// add new record
	records[recordsSize] = newRecord;
	recordsSize++;
}