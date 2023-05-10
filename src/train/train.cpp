#include "train.h"

namespace ticket {

int TrainManager::add_train(Train const train) {
	if (trainID2int.find(train.trainID) != trainID2int.end())
		return -1;
	// there should be some checks
	int id = trains.insert(train);
	if (id <= 0)
		return -2;
	trainID2int.insert({train.trainID, id});
	return id;
}

int TrainManager::find(char const *trainID) {
	auto res = trainID2int.find(trainID);
	if (res == trainID2int.end()) return -1;
	return res->second;
}

inline std::pair<int, bool> TrainManager::get_id_and_released(char const *trainID) {
	int id = find(trainID);
	if (id <= 0) return {0, false};
	bool isReleased = false;
	trains.read(id, isReleased, offsetof(Train, released));
	return {id, isReleased};
}

int TrainManager::release_train(char const *trainID) {
	auto [id, isReleased] = get_id_and_released(trainID);
	if (id <= 0) return -1;
	if (isReleased) return -2;
	isReleased = true;
	trains.write(id, isReleased, offsetof(Train, released));

	// do some other works

	return 0;
}

int TrainManager::delete_train(char const *trainID) {
	auto [id, isReleased] = get_id_and_released(trainID);
	if (id <= 0) return -1;
	if (isReleased) return -2;
	trainID2int.erase(trainID);
	return 0;
}

TicketsOnPath TrainManager::avaliable_ticket(int id, Date d) {
	TicketsOnPath ret;
	tickets.read(id, ret, sizeof(TicketsOnPath) * (int(d)));
	return ret;
}


}// namespace ticket