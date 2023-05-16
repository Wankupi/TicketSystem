#include "ticket.h"

namespace ticket {

int BillManager::add_bill(Bill const &bill) {
	std::pair<int, int> meta{0, 0};
	if (bill.user_id <= max_user_id) meta = head.read(bill.user_id);
	else
		max_user_id = bill.user_id;
	meta.first = data.insert({bill, meta.first});
	++meta.second;
	head.write(bill.user_id, meta);

	if (bill.stat == Bill::pending) {
		waiting.insert({{bill.train_id, bill.start}, meta.first});
	}
	return 0;
}

kupi::vector<Bill> BillManager::query_bill(int user_id) {
	kupi::vector<Bill> res;
	int index = 0;
	if (user_id <= max_user_id) {
		auto meta = head.read(user_id);
		index = meta.first;
		res.reserve(meta.second);
	}
	while (index) {
		auto ret = data.read(index);
		res.emplace_back(ret.first);
		index = ret.second;
	}
	return res;
}

int BillManager::refund_bill(int user_id, int n, Bill &bill) {
	if (user_id > max_user_id) return -1;
	auto [index, total] = head.read(user_id);
	if (total < n) return -1;
	for (int i = 1; i < n; ++i)
		data.read(index, index, offsetof(decltype(data.read(0)), second));
	data.read(index, bill, offsetof(decltype(data.read(0)), first));
	if (bill.stat == Bill::refunded)
		return -2;
	bool isPending = bill.stat == Bill::pending;
	bill.stat = Bill::refunded;
	data.write(index, bill.stat, offsetof(decltype(data.read(0)), first) + offsetof(Bill, stat));
	return isPending;
}

void BillManager::set_success(int train_id, Date date, int bill_id) {
	Bill::Status stat = Bill::success;
	data.write(bill_id, stat, offsetof(decltype(data.read(0)), first) + offsetof(Bill, stat));
	waiting.erase({train_id, date}, bill_id);
}

}// namespace ticket