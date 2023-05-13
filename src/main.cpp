#include "terminal/TicketTerminal.h"
#include <filesystem>
#include <iostream>
char buff[1024 * 4];

void clear() {
	std::filesystem::remove_all("users");
	std::filesystem::remove_all("train");
	std::filesystem::remove_all("bill");
}

int main() {
	std::ios::sync_with_stdio(false);
	std::cin.tie(0);

	ticket::UserManager um("users/main.db", "users/index");
	ticket::TrainManager tm("train/main.db", "train/ticket.db", "train/trainID", "train/station");
	ticket::BillManager bm("bill/head.db", "bill/main.db", "bill/wait");
	ticket::TicketTerminal term(um, tm, bm);
	char step[20];
	while (std::cin >> step) {
		std::cout << step << ' ';
		std::cin.get();
		std::cin.getline(buff, sizeof(buff));
		int r = term.run(buff, std::cout);
		std::cout.put('\n');
		if (r == 1) break;
	}
	return 0;
}
