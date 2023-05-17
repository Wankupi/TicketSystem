#include "terminal/TicketTerminal.h"
#include <filesystem>
#include <iostream>

char buff[1024 * 4];
char stepStr[20];
int stepNum = 0;

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
    char (&step)[20] = stepStr;
    while (is >> step) {
        os << step << ' ';
        stepNum = atoi(step + 1);
        is.get();
        is.getline(buff, sizeof(buff));
        int r = term.run(buff, os);
        os.put('\n');
        if (r == 1) break;
        if (r == 2) return 1;
    }
    return 0;
}

int main(int argc, char *argv[]) {
	freopen("t.txt", "w", stderr);
    if (argc < 2) {
        std::cout << "params not enough";
        return 1;
    }
    clear();
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);
    for (int i = 1; i <= 30; ++i) {
        std::ifstream is(std::string(argv[1]) + std::to_string(i) + ".in");
        if (!is) break;
        std::ofstream os(std::to_string(i) + ".out");
        std::cout << "case " << i << std::endl;
        while (run(is, os)) {
            clear();
        }
        is.close();
        os.close();
    }
    return 0;
}
