#include "terminal/TicketTerminal.h"
#include <filesystem>
#include <iostream>

char buff[1024 * 4];

void clear() {
	std::filesystem::remove_all("users");
	std::filesystem::remove_all("train");
	std::filesystem::remove_all("bill");
}

int run(std::istream &is, std::ostream &os) {
	ticket::UserManager um("users/main.db", "users/index");
	ticket::TrainManager tm("train/main.db", "train/ticket.db", "train/trainID", "train/station");
	ticket::BillManager bm("bill/head.db", "bill/main.db", "bill/wait");
	ticket::TicketTerminal term(um, tm, bm);
	char step[20];
	while (is >> step) {
		os << step << ' ';
		is.get();
		is.getline(buff, sizeof(buff));
		int r = term.run(buff, os);
		os.put('\n');
		if (r == 1) break;
		if (r == 2) return 1;
	}
	return 0;
}

int main() {
	std::ios::sync_with_stdio(false);
	std::cin.tie(0);
	while (run(std::cin, std::cout)) {
		clear();
	}
	return 0;
}
