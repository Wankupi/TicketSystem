#pragma once
#include "base.h"
#include "bpt/bpt.h"
#include "bpt/multibpt.h"
#include "file/DataBase.h"
#include "vector.h"

namespace ticket {

struct Bill {
	int user_id, train_id;
	String<20> trainID;
	DateTime leave, arrive;
	Date start;
	String<30> S;
	String<30> T;
	int price, count;
	enum Status { success,
				  pending,
				  refunded } stat;
};

inline std::ostream &operator<<(std::ostream &os, Bill const &bill) {
	constexpr const char *const status_str[] = {"[success]", "[pending]", "[refunded]"};
	return os << status_str[bill.stat] << ' ' << bill.trainID << ' ' << bill.S << ' ' << bill.leave << " -> " << bill.T << ' ' << bill.arrive << ' ' << bill.price << ' ' << bill.count;
}

class BillManager {
public:
	BillManager(std::string const &head_file, std::string const &data_file, std::string const &wait_list_file)
		: head(head_file), data(data_file), waiting(wait_list_file), max_user_id(head.size()) {}
	int add_bill(Bill const &bill);
	void query_bill(int user_id, kupi::vector<Bill> &res);
	/**
	 * @return 0 success
	 * @return -1  this user do not have that many bills
	 */
	int refund_bill(int user_id, int n, Bill &bill);
	kupi::multibpt<std::pair<int, Date>, int>::iterator get_waiting(int train_id, Date date) {
		return waiting.find({train_id, date});
	}
	kupi::multibpt<std::pair<int, Date>, int>::iterator end_of_waiting() {
		return waiting.end();
	}
	void get_bill(int bill_id, Bill &bill) {
		data.read(bill_id, bill, offsetof(decltype(data.read(0)), first));
	}
	void set_success(int train_id, Date date, int bill_id);

private:
	kupi::DataBase<std::pair<int, int>> head;
	kupi::DataBase<std::pair<Bill, int>> data;
	kupi::multibpt<std::pair<int, Date>, int> waiting;
	int max_user_id;// to avoid reading at wrong position
};

}// namespace ticket