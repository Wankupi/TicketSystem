#include "terminal/TicketTerminal.h"
#include <iostream>

char buff[1024 * 4];

int main() {
	freopen("t.out", "w", stdout);
	std::ios::sync_with_stdio(false);
	std::cin.tie(0);

	UserManager um("abcd");
	TrainManager tm;
	TicketTerminal term(um, tm);
	while (std::cin >> buff) {
		std::cout << buff << " ";
		std::cin.get();
		std::cin.getline(buff, sizeof(buff));
		term.run(buff, std::cout);
		std::cout.put('\n');
	}
	return 0;
}
