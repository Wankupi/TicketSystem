#include "terminal/TicketTerminal.h"
#include <filesystem>
#include <iostream>
char buff[1024 * 4];

void clear() {
	std::filesystem::remove_all("users");
}

int main() {
	clear();
	freopen("t.out", "w", stdout);
	std::ios::sync_with_stdio(false);
	std::cin.tie(0);

	ticket::UserManager um("users/main.db", "users/index");
	ticket::TrainManager tm;
	ticket::TicketTerminal term(um, tm);
	char step[20];
	while (std::cin >> step) {
		std::cout << step << " ";
		std::cin.get();
		std::cin.getline(buff, sizeof(buff));
		int r = term.run(buff, std::cout);
		std::cout.put('\n');
		if (r == 1) break;
	}
	return 0;
}
