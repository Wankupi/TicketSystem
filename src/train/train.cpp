#include "train.h"

namespace ticket {

int TrainManager::add_train(Train const &train) {
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
	int id = find(trainID);
	if (id <= 0) return -1;
	Train train = get_train(id);
	if (train.released) return -2;
	trains.write(id, train.released = true, offsetof(Train, released));

	// do some other works
	DateTime first{train.saleDateBegin, train.startTime}, last{train.saleDateEnd, train.startTime};

	train_info_in_station info{id, first.d, last.d, first.t, 0, 0, 0, 0};
	for (int i = 0; i < train.stationNum - 1; ++i) {
		stations.insert({train.stations[i], info});
		++info.kth;
		info.running_time += train.travelTimes[i] + info.stop;
		info.stop = train.stopoverTimes[i];
		info.price += train.prices[i];
		first += train.travelTimes[i] + train.stopoverTimes[i];
		last += train.travelTimes[i] + train.stopoverTimes[i];
		info.firstDay = first.d;
		info.lastDay = last.d;
		info.leave = first.t;
	}
	stations.insert({train.stations[train.stationNum - 1], info});

	Tickets tick{};
	memset(&tick, 0, sizeof(tick));
	for (int day = train.saleDateBegin; day <= train.saleDateEnd; ++day) {
		for (int i = 0; i < train.stationNum - 1; ++i)
			tick.seat[day][i] = train.seatNum;
	}
	tickets.write(id, tick);
	return 0;
}

int TrainManager::delete_train(char const *trainID) {
	auto [id, isReleased] = get_id_and_released(trainID);
	if (id <= 0) return -1;
	if (isReleased) return -2;
	trainID2int.erase(trainID);
	return 0;
}

void TrainManager::get_ticket(int id, Date d, TicketsOnPath &tp) {
	tickets.read(id, tp, sizeof(TicketsOnPath) * (int(d)));
}

int TrainManager::query_train(char const *trainID, Date date, Train &train, TicketsOnPath &tp) {
	int id = find(trainID);
	if (id <= 0) return -1;
	trains.read(id, train);
	if (!(train.saleDateBegin <= date && date <= train.saleDateEnd))
		memset(&tp, 0, sizeof(int) * (train.stationNum - 1));
	else if (train.released)
		get_ticket(id, date, tp);
	else {
		for (int i = 0; i < train.stationNum - 1; ++i)
			tp[i] = train.seatNum;
	}
	return 0;
}

int TrainManager::query_ticket(char const *S, char const *T, Date date, kupi::vector<QueryResult> &res) {
	res.clear();
	String<30> Sa(S), Ta(T);
	QueryResult qr;// put it outside the for loop to reduce allocate...
	TicketsOnPath tp;
	auto p1 = stations.find(Sa), p2 = stations.find(Ta);
	while (p1 != stations.end() && p2 != stations.end() && p1->first == Sa && p2->first == Ta) {
		if (p1->second < p2->second) ++p1;
		else if (p2->second < p1->second)
			++p2;
		else {
			auto const &t1 = p1->second, &t2 = p2->second;
			++p1;
			++p2;
			if (!t1.contain_day(date) || t2.kth <= t1.kth) continue;

			trains.read(t1.train_id, qr.trainID, offsetof(Train, trainID));
			qr.leave = DateTime{date, t1.leave};

			qr.time = t2.running_time - t1.running_time - t1.stop;
			qr.price = t2.price - t1.price;
			DateTime startTime = qr.leave - (t1.running_time + t1.stop);
			get_ticket(t1.train_id, startTime.d, tp);
			qr.seat = 0x7fffff;
			for (int i = t1.kth; i < t2.kth; ++i)
				if (tp[i] < qr.seat) qr.seat = tp[i];
			res.push_back(qr);
		}
	}
	return 0;
}


}// namespace ticket