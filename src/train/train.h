#pragma once
#include "base.h"
#include "bpt/bpt.h"
#include "bpt/multibpt.h"
#include "file/DataBase.h"
#include "vector.h"

namespace ticket {
struct Train {
	String<20> trainID;
	unsigned char stationNum;
	String<30> stations[100];
	int seatNum;
	int prices[99];
	Time startTime;
	int travelTimes[99];
	int stopoverTimes[98];
	Date saleDateBegin;
	Date saleDateEnd;
	char type = 0;
	bool released = false;
};

using TicketsOnPath = std::array<int, 99>;
struct Tickets {
	TicketsOnPath seat[92];// maximum 92 days and 100 stations
};

class TrainManager {
public:
	TrainManager(std::string const &train_file, std::string const &ticket_file, std::string const &train2id_file, std::string const &station_file)
		: trains(train_file), tickets(ticket_file), trainID2int(train2id_file), stations(station_file) {}
	/**
	 * @return 0 : success
	 * @return -1 : trainID conflict
	 * @return -2 : error occurs in insert, should not happen
	 */
	//	int add_train(char const *trainID, unsigned char stationNum, int seatNum, char const *stations[], int prices[], Time startTime, int travelTimes[], int stopoverTimes[], Date saleDateStart, Date saleDateEnd, char type);
	int add_train(Train const &train);
	/**
	 * @return 0 : success
	 * @return -1 : train not found
	 * @return -2 : train already released
	 */
	int delete_train(char const *trainID);
	/**
	 * @return 0 : success
	 * @return -1 : train not found
	 * @return -2 : train already released
	 */
	int release_train(char const *trainID);
	/**
	 * @return positive number: found
	 * @return 0 or negative: not found
	 */
	int find(char const *trainID);
	/**
	 * @attention caller should ensure id is valid
	 */
	Train get_train(int id) { return trains.read(id); }

	int query_train(char const *trainID, Date date, Train &train, TicketsOnPath &tp);

	struct QueryResult {
		String<20> trainID;
		DateTime leave;
		int time;
		int price;
		int seat;
		static bool cmpCost(QueryResult const &lhs, QueryResult const &rhs) {
			return lhs.price == rhs.price ? lhs.trainID < rhs.trainID : lhs.price < rhs.price;
		}
		static bool cmpTime(QueryResult const &lhs, QueryResult const &rhs) {
			return lhs.time == rhs.time ? lhs.trainID < rhs.trainID : lhs.time < rhs.time;
		}
	};
	int query_ticket(char const *S, char const *T, Date date, kupi::vector<QueryResult> &res);

private:
	/**
	 * @attention caller should ensure id and d is valid
	 */
	void get_ticket(int id, Date d, TicketsOnPath &ret);

private:
	std::pair<int, bool> get_id_and_released(char const *trainID);

private:
	kupi::DataBase<Train, true, false> trains;
	kupi::DataBase<Tickets, false, false> tickets;
	kupi::bpt<decltype(Train::trainID), int> trainID2int;
	struct train_info_in_station {
		int train_id;
		Date firstDay, lastDay;// leaving day
		Time leave;
		int stop;
		int kth;
		int running_time; // arrival time
		int price;// prefix sum
		bool operator<(train_info_in_station const &rhs) const { return train_id < rhs.train_id; }
		bool contain_day(Date d) const { return firstDay <= d && d <= lastDay; }
	};
	kupi::multibpt<String<30>, train_info_in_station> stations;
};

}// namespace ticket