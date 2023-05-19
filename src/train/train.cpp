#include "train.h"
#include "map.h"

namespace ticket {

int TrainManager::add_train(Train const &train) {
	if (trainID2int.find(train.trainID) != trainID2int.end())
		return -1;
	// there should be some checks
	int id = trains.insert(train);
	if (id <= 0)
		return -2;
	trainID2int.insert(train.trainID, id);
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
		stations.insert(train.stations[i], info);
		++info.kth;
		info.running_time = train.travelTime[i];
		info.stop = train.stopoverTimes[i];
		info.price = train.prices[i];
		info.firstDay = (first + train.travelTime[i] + train.stopoverTimes[i]).d;
		info.lastDay = (last + train.travelTime[i] + train.stopoverTimes[i]).d;
		info.leave = (first + train.travelTime[i] + train.stopoverTimes[i]).t;
	}
	stations.insert(train.stations[train.stationNum - 1], info);

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

void TrainManager::set_ticket(int id, Date d, TicketsOnPath &tp) {
	tickets.write(id, tp, sizeof(TicketsOnPath) * (int(d)));
}

int TrainManager::query_train(char const *trainID, Date date, Train &train, TicketsOnPath &tp) {
	int id = find(trainID);
	if (id <= 0) return -1;
	trains.read(id, train);
	if (!(train.saleDateBegin <= date && date <= train.saleDateEnd))
		return -2;
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
	auto v1 = stations.find_all(Sa), v2 = stations.find_all(Ta);
	auto p1 = v1.begin(), p2 = v2.begin();
	while (p1 != v1.end() && p2 != v2.end()) {
		if (p1->train_id < p2->train_id) ++p1;
		else if (p2->train_id < p1->train_id)
			++p2;
		else {
			auto const &t1 = *p1, &t2 = *p2;
			++p1;
			++p2;
			if (!t1.contain_leave_day(date) || t2.kth <= t1.kth) continue;

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

int TrainManager::query_transfer(const char *S, const char *T, Date date, TransferResult &res, bool (*cmp)(TransferResult const &, TransferResult const &)) {
	String<30> Sa(S), Ta(T);
	kupi::map<String<30>, std::vector<std::tuple<int, QueryResult>>> arr;// stationName --> { train_id, arrive_date_time }
	Train train;

	for (auto p = stations.find(Sa); p != stations.end() && p->first == Sa; ++p) {
		if (!p->second.contain_leave_day(date)) continue;
		trains.read(p->second.train_id, train);
		for (int i = p->second.kth + 1; i < train.stationNum; ++i) {
			arr[train.stations[i]].emplace_back(p->second.train_id, QueryResult{
																			train.trainID,
																			DateTime(date, p->second.leave),
																			train.travelTime[i - 1] - p->second.running_time - p->second.stop,
																			train.prices[i - 1] - p->second.price,
																			0});
		}
	}
	TransferResult tmp;
	int id1 = 0, id2 = 0;
	bool has_result = false;
	for (auto p = stations.find(Ta); p != stations.end() && p->first == Ta; ++p) {
		trains.read(p->second.train_id, train);
		for (int i = 0; i < p->second.kth; ++i) {
			auto st = arr.find(train.stations[i]);
			if (st == arr.end()) continue;

			for (auto [_id1, path1]: st->second) {
				if (_id1 == p->second.train_id)
					continue;
				DateTime arrive_time = path1.leave + path1.time;
				DateTime leave_time{arrive_time.d, Time((train.startTime + (i ? train.travelTime[i - 1] + train.stopoverTimes[i - 1] : 0)) % (24 * 60))};
				if (leave_time < arrive_time) leave_time += 24 * 60;

				DateTime train2_start_time = leave_time - (i ? train.travelTime[i - 1] + train.stopoverTimes[i - 1] : 0);
				if (train2_start_time.d < train.saleDateBegin) {
					int dis = train.saleDateBegin - train2_start_time.d;
					train2_start_time += dis * 24 * 60;
					leave_time += dis * 24 * 60;
				}
				if (train2_start_time.d > train.saleDateEnd)
					continue;

				tmp.part1 = path1;
				tmp.mid_station = train.stations[i];
				tmp.part2.trainID = train.trainID;
				tmp.part2.leave = leave_time;
				tmp.part2.time = train.travelTime[p->second.kth - 1] - (i ? train.travelTime[i - 1] + train.stopoverTimes[i - 1] : 0);
				tmp.part2.price = train.prices[p->second.kth - 1] - (i ? train.prices[i - 1] : 0);

				if (!has_result || cmp(tmp, res)) {
					id1 = _id1;
					id2 = p->second.train_id;
					has_result = true;
					res = tmp;
				}
			}
		}
	}
	if (!has_result)
		return 1;
	TicketsOnPath tp;
	auto load_ticket_number = [this, &train, &tp](int train_id, String<30> const &S, String<30> const &T, DateTime leaveTime) -> int {
		trains.read(train_id, train);
		int iS = 0, iT = 0;
		for (iS = 0; train.stations[iS] != S; ++iS)
			;
		for (iT = iS + 1; train.stations[iT] != T; ++iT)
			;
		DateTime startTime = leaveTime - (iS ? train.travelTime[iS - 1] + train.stopoverTimes[iS - 1] : 0);
		get_ticket(train_id, startTime.d, tp);
		int num = 0x3f3f3f3f;
		for (int i = iS; i < iT; ++i)
			if (tp[i] < num) num = tp[i];
		return num;
	};
	res.part1.seat = load_ticket_number(id1, Sa, res.mid_station, res.part1.leave);
	res.part2.seat = load_ticket_number(id2, res.mid_station, Ta, res.part2.leave);
	return 0;
}

int TrainManager::buy_ticket(int id, String<30> const &S, String<30> const &T, Date date, int count, buy_ticket_result &res) {
	auto train = get_train(id);
	if (!train.released) return -1;
	if (train.seatNum < count)
		return -3;
	int iS = 0, iT = 0;
	for (iS = 0; iS < train.stationNum; ++iS)
		if (train.stations[iS] == S) break;
	for (iT = iS + 1; iT < train.stationNum; ++iT)
		if (train.stations[iT] == T) break;
	if (iT >= train.stationNum) return -2;

	DateTime startTime{train.saleDateBegin, train.startTime},
			leave = startTime + (iS ? train.travelTime[iS - 1] + train.stopoverTimes[iS - 1] : 0),
			arrive = startTime + train.travelTime[iT - 1];
	int distance = int(date) - (leave.d);
	if (distance < 0 || distance > train.saleDateEnd - train.saleDateBegin)
		return -1;
	startTime.add_day(distance);
	leave.add_day(distance);
	arrive.add_day(distance);
	int cost = train.prices[iT - 1] - (iS == 0 ? 0 : train.prices[iS - 1]);

	res = {cost, startTime.d, leave, arrive};

	TicketsOnPath tp;
	get_ticket(id, startTime.d, tp);
	for (int i = iS; i < iT; ++i)
		if ((tp[i] -= count) < 0)
			return 1;
	set_ticket(id, startTime.d, tp);
	return 0;
}

int TrainManager::add_ticket(int id, String<30> const &S, String<30> const &T, Date date, int count) {
	auto train = get_train(id);
	String<30> SS{S}, TT{T};
	int iS = 0, iT = 0;
	for (iS = 0; iS < train.stationNum; ++iS)
		if (train.stations[iS] == SS) break;
	for (iT = iS + 1; iT < train.stationNum; ++iT)
		if (train.stations[iT] == TT) break;

	TicketsOnPath tp;
	get_ticket(id, date, tp);
	for (int i = iS; i < iT; ++i)
		tp[i] += count;
	set_ticket(id, date, tp);
	return 0;
}


}// namespace ticket
