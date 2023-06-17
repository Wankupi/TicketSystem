#include "TicketTerminal.h"
#include "algorithm.h"
#include <cstring>

namespace ticket {

int TicketTerminal::run(const char *s, std::ostream &os) {
	static char buff[4096];
	static Params params;
	size_t N = strlen(s);
	memcpy(buff, s, N + 1);
	parse_command(buff, params);
	auto func = get_function(buff);
	return (this->*func)(params, os);
}

TicketTerminal::Function TicketTerminal::get_function(char const *cmd) {
	if (cmd[0] == 'a') return cmd[4] == 'u' ? &TicketTerminal::run_add_user : &TicketTerminal::run_add_train;
	else if (cmd[0] == 'b')
		return &TicketTerminal::run_buy_ticket;
	else if (cmd[0] == 'c')
		return &TicketTerminal::run_clean;
	else if (cmd[0] == 'd')
		return &TicketTerminal::run_delete_train;
	else if (cmd[0] == 'e')
		return &TicketTerminal::run_exit;
	else if (cmd[0] == 'l')
		return cmd[3] == 'i' ? &TicketTerminal::run_login : &TicketTerminal::run_logout;
	else if (cmd[0] == 'm')
		return &TicketTerminal::run_modify_profile;
	else if (cmd[0] == 'q') {
		if (cmd[9] == 'f') return &TicketTerminal::run_query_profile;
		else if (cmd[9] == 'i')
			return &TicketTerminal::run_query_train;
		else if (cmd[9] == 'k')
			return &TicketTerminal::run_query_ticket;
		else if (cmd[9] == 'n')
			return &TicketTerminal::run_query_transfer;
		else if (cmd[9] == 'e')
			return &TicketTerminal::run_query_order;
	}
	else if (cmd[0] == 'r')
		return cmd[2] == 'l' ? &TicketTerminal::run_release_train : &TicketTerminal::run_refund_ticket;
	return &TicketTerminal::run_nothing;
}

void TicketTerminal::parse_command(char *s, Params &params) {
	memset(&params, 0, sizeof(params));
	// no need to consider utf-8 here, because utf-b will always be 0b1xxxxxxx;
	auto skip_word = [&s]() {
		while (*s && *s != ' ') ++s;
		if (!*s) return true;
		*s++ = 0;
		return false;
	};
	if (skip_word()) return;
	while (*s == '-') {
		++s;
		params[*s] = s + 2;
		*++s = 0;
		++s;
		if (skip_word()) break;
	}
}

// // clang-format off
enum RunResult : int { normal,
					   exit,
					   clean,
					   file_error,
					   params_missing,
					   params_invalid,
					   permission_denied,
					   user_not_found,
					   re_login,
					   un_login,
					   trainId_conflict,
					   train_not_found,
					   train_already_released,
					   time_outbound,
					   bills_not_enough,
					   bill_already_refunded };
// clang-format on

static inline int ok(std::ostream &os) {
	os << '0';
	return normal;
}

static inline int fail(std::ostream &os, int code = normal) {
	os << "-1";
	return code;
}

int TicketTerminal::run_add_user(Params const &params, std::ostream &os) {
	int privilege;
	if (users.empty())
		privilege = 10;
	else {
		int id_cur = users.find(params['c']);
		if (!loginList.count(id_cur))
			return fail(os, un_login);
		auto privilege_cur = users.get_privilege(id_cur);
		privilege = atoi(params['g']);
		if (privilege >= privilege_cur)
			return fail(os, permission_denied);
	}
	auto res = users.add_user(params['u'], params['p'], params['n'], params['m'], privilege);
	if (!res) return fail(os, params_invalid);
	return ok(os);
}

int TicketTerminal::run_login(Params const &params, std::ostream &os) {
	int id = users.check_password(params['u'], params['p']);
	if (id <= 0) return fail(os, user_not_found);
	if (loginList.count(id)) return fail(os, re_login);
	loginList.insert(id);
	return ok(os);
}

int TicketTerminal::run_logout(Params const &params, std::ostream &os) {
	int id = users.find(params['u']);
	if (id <= 0) return fail(os, user_not_found);
	if (!loginList.erase(id))
		return fail(os, un_login);
	return ok(os);
}

int TicketTerminal::run_query_profile(Params const &params, std::ostream &os) {
	int id_cur = users.find(params['c']);
	if (!loginList.count(id_cur))
		return fail(os, un_login);
	if (strcmp(params['c'], params['u']) == 0) {
		os << users.get(id_cur);
		return 0;
	}
	int id_query = users.find(params['u']);
	if (id_query <= 0)
		return fail(os, user_not_found);
	auto privilege_cur = users.get_privilege(id_cur);
	auto res = users.get(id_query);
	if (privilege_cur <= res.privilege)
		return fail(os, permission_denied);
	os << res;
	return 0;
}

int TicketTerminal::run_modify_profile(Params const &params, std::ostream &os) {
	int id_cur = users.find(params['c']);
	if (!loginList.count(id_cur))
		return fail(os, un_login);
	int id = users.find(params['u']);
	if (id <= 0)
		return fail(os, user_not_found);
	int privilege_cur = users.get_privilege(id_cur), privilege_tar = users.get_privilege(id);
	if (privilege_cur < privilege_tar || (privilege_cur == privilege_tar && id_cur != id)) return fail(os, permission_denied);
	int new_privilege = User::MaxPrivilege + 1;
	if (params['g']) {
		new_privilege = atoi(params['g']);
		if (new_privilege >= privilege_cur) return fail(os, permission_denied);
	}
	auto res = users.set(id, nullptr, params['p'], params['n'], params['m'], new_privilege);
	if (!res) return fail(os, params_invalid);
	os << *res;
	return normal;
}

void split_string_to_array(char const *start, decltype(Train::stations) t, int n) {
	for (int i = 0; i < n; ++i) {
		auto dest = t[i].data();
		while (*start && *start != '|')
			*dest++ = *start++;
		*dest = 0;
		++start;
	}
}

void split_string_to_int(char const *start, int t[], int n) {
	for (int i = 0; i < n; ++i) {
		t[i] = atoi(start);
		while (*start && *start != '|')
			*start++;
		++start;
	}
}

int TicketTerminal::run_add_train(Params const &params, std::ostream &os) {
	Train train;
	train.trainID = params['i'];
	train.stationNum = atoi(params['n']);
	train.seatNum = atoi(params['m']);
	split_string_to_array(params['s'], train.stations, train.stationNum);
	split_string_to_int(params['p'], train.prices, train.stationNum - 1);
	split_string_to_int(params['t'], train.travelTime, train.stationNum - 1);
	split_string_to_int(params['o'], train.stopoverTimes, train.stationNum - 2);
	train.startTime = Time{params['x']};
	train.saleDateBegin = Date{params['d']};
	train.saleDateEnd = Date{params['d'] + 6};
	train.type = *params['y'];

	for (int i = 1; i < train.stationNum - 1; ++i)
		train.travelTime[i] += train.travelTime[i - 1] + train.stopoverTimes[i - 1];
	for (int i = 1; i < train.stationNum - 1; ++i) train.prices[i] += train.prices[i - 1];

	int res = trains.add_train(train);
	if (res == -1)
		return fail(os, trainId_conflict);
	else if (res == -2)
		return fail(os, file_error);
	return ok(os);
}

int TicketTerminal::run_delete_train(Params const &params, std::ostream &os) {
	int res = trains.delete_train(params['i']);
	if (res == -1) return fail(os, train_not_found);
	else if (res == -2)
		return fail(os, train_already_released);
	return ok(os);
}

int TicketTerminal::run_release_train(Params const &params, std::ostream &os) {
	int res = trains.release_train(params['i']);
	if (res == -1) return fail(os, train_not_found);
	else if (res == -2)
		return fail(os, train_already_released);
	return ok(os);
}

int TicketTerminal::run_query_train(Params const &params, std::ostream &os) {
	Train train;
	TicketsOnPath ts;
	Date d{params['d']};
	if (int(d) < 0 || int(d) >= 92)
		return fail(os, time_outbound);
	int res = trains.query_train(params['i'], d, train, ts);
	if (res == -1)
		return fail(os, train_not_found);
	if (res == -2)
		return fail(os, time_outbound);
	os << train.trainID << ' ' << train.type << '\n';
	DateTime dt{d, train.startTime};
	os << train.stations[0] << " xx-xx xx:xx -> " << dt << " 0 " << ts[0] << '\n';
	dt += train.travelTime[0];
	for (int i = 1; i < train.stationNum - 1; ++i) {
		os << train.stations[i] << ' ' << dt << " -> ";
		dt += train.stopoverTimes[i - 1];
		os << dt << ' ' << train.prices[i - 1] << ' ' << ts[i] << '\n';
		dt = DateTime{d, train.startTime} + train.travelTime[i];
	}
	os << train.stations[train.stationNum - 1] << ' ' << dt << " -> xx-xx xx:xx " << train.prices[train.stationNum - 2] << " x";
	return normal;
}

int TicketTerminal::run_query_ticket(Params const &params, std::ostream &os) {
	static kupi::vector<TrainManager::QueryResult> res;
	trains.query_ticket(params['s'], params['t'], Date{params['d']}, res);
	kupi::vector<int> p(res.size(), 0);
	for (int i = 0; i < p.size(); ++i) p[i] = i;
	if (params['p'] && params['p'][0] == 't')
		kupi::sort(p.begin(), p.end(), [](int x, int y) { return TrainManager::QueryResult::cmpTime(res[x], res[y]); });
	else
		kupi::sort(p.begin(), p.end(), [](int x, int y) { return TrainManager::QueryResult::cmpCost(res[x], res[y]); });

	os << res.size();
	for (int i: p) {
		auto &qr = res[i];
		os << '\n'
		   << qr.trainID << ' ' << params['s'] << ' ' << qr.leave << " -> " << params['t'] << ' ' << (qr.leave + qr.time)
		   << ' ' << qr.price << ' ' << qr.seat;
	}
	return normal;
}


int TicketTerminal::run_query_transfer(Params const &params, std::ostream &os) {
	TrainManager::TransferResult res;
	int ret = trains.query_transfer(params['s'], params['t'], Date{params['d']}, res,
									(params['p'] && params['p'][0] == 'c' ? TrainManager::TransferResult::cmp_price : TrainManager::TransferResult::cmp_time));
	if (ret == 1) {// found no train
		os << "0";
		return normal;
	}
	os << res.part1.trainID << ' ' << params['s'] << ' ' << res.part1.leave << " -> " << res.mid_station << ' ' << (res.part1.leave + res.part1.time)
	   << ' ' << res.part1.price << ' ' << res.part1.seat << '\n';
	os << res.part2.trainID << ' ' << res.mid_station << ' ' << res.part2.leave << " -> " << params['t'] << ' ' << (res.part2.leave + res.part2.time)
	   << ' ' << res.part2.price << ' ' << res.part2.seat;
	return normal;
}


int TicketTerminal::run_buy_ticket(Params const &params, std::ostream &os) {
	int user_id = users.find(params['u']);
	if (!loginList.count(user_id))
		return fail(os, un_login);
	int train_id = trains.find(params['i']);
	int count = atoi(params['n']);
	TrainManager::buy_ticket_result res;
	auto res_code = trains.buy_ticket(train_id, params['f'], params['t'], Date{params['d']}, count, res);

	if (res_code == 0) {
		bills.add_bill(Bill{user_id, train_id, params['i'], res.leave, res.arrive, res.start, params['f'], params['t'], res.cost, count, Bill::success});
		os << res.cost * count;
	}
	else if (res_code == 1 && params['q'] && params['q'][0] == 't') {// waiting is true
		bills.add_bill(Bill{user_id, train_id, params['i'], res.leave, res.arrive, res.start, params['f'], params['t'], res.cost, count, Bill::pending});
		os << "queue";
	}
	else
		os << -1;
	return normal;
}

int TicketTerminal::run_query_order(Params const &params, std::ostream &os) {
	int user_id = users.find(params['u']);
	if (!loginList.count(user_id))
		return fail(os, un_login);
	static kupi::vector<Bill> res;
	bills.query_bill(user_id, res);
	os << res.size();
	for (auto const &bill: res)
		os << '\n'
		   << bill;
	return normal;
}

int TicketTerminal::run_refund_ticket(Params const &params, std::ostream &os) {
	int user_id = users.find(params['u']);
	if (!loginList.count(user_id))
		return fail(os, un_login);
	int n = atoi(params['n']);
	Bill bill;
	int res = bills.refund_bill(user_id, n, bill);
	if (res == -1) return fail(os, bills_not_enough);
	if (res == -2) return fail(os, bill_already_refunded);
	if (res == 1)// refund a pending ticket
		return ok(os);

	trains.add_ticket(bill.train_id, bill.S, bill.T, bill.start, bill.count);
	// deal with waiting list
	std::pair<int, Date> meta{bill.train_id, bill.start};
	auto cur = bills.get_waiting(meta.first, meta.second), end = bills.end_of_waiting();
	static kupi::vector<int> successes;
	successes.clear();
	TrainManager::buy_ticket_result buy_res;
	while (cur != end && cur->first == meta) {
		bills.get_bill(cur->second, bill);
		res = trains.buy_ticket(bill.train_id, bill.S, bill.T, bill.leave.d, bill.count, buy_res);
		if (res == 0)
			successes.push_back(cur->second);
		++cur;
	}
	for (auto i: successes) bills.set_success(meta.first, meta.second, i);
	return ok(os);
}

int TicketTerminal::run_clean(Params const &params, std::ostream &os) {
	os << '0';
	return clean;
}

int TicketTerminal::run_exit(Params const &params, std::ostream &os) {
	os << "bye";
	return RunResult::exit;
}


char const *TicketTerminal::run_result_to_string(int res) {
	constexpr const char *error_code[] = {
			"normal",
			"exit",
			"clean",
			"file_error",
			"params_missing",
			"params_invalid",
			"permission_denied",
			"user_not_found",
			"re_login",
			"un_login",
			"trainId_conflict",
			"train_not_found",
			"train_already_released",
			"time_outbound",
			"bills_not_enough",
			"bill_already_refunded"};
	return error_code[res];
}


}// namespace ticket
