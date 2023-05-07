#include "TicketTerminal.h"
#include <cstring>

int TicketTerminal::run(const char *s, std::ostream &os) {
	static char buff[4096];
	static char *params[26];
	size_t N = strlen(s);
	memcpy(buff, s, N + 1);
	parse_command(buff, params);
	auto func = get_function(buff);
	return (this->*func)(params, os);
}

TicketTerminal::Function TicketTerminal::get_function(char const *cmd) {
	if (cmd[0] == 'a') {
		return cmd[4] == 'u' ? &TicketTerminal::run_add_user : &TicketTerminal::run_add_train;
	}
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

void TicketTerminal::parse_command(char *s, char *(&params)[26]) {
	memset(params, 0, sizeof(params));
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

int TicketTerminal::run_add_user(Params const &param, std::ostream &os) { return 0; }
int TicketTerminal::run_login(Params const &param, std::ostream &os) { return 0; }
int TicketTerminal::run_logout(Params const &param, std::ostream &os) { return 0; }
int TicketTerminal::run_query_profile(Params const &param, std::ostream &os) { return 0; }
int TicketTerminal::run_modify_profile(Params const &param, std::ostream &os) { return 0; }
int TicketTerminal::run_add_train(Params const &param, std::ostream &os) { return 0; }
int TicketTerminal::run_delete_train(Params const &param, std::ostream &os) { return 0; }
int TicketTerminal::run_release_train(Params const &param, std::ostream &os) { return 0; }
int TicketTerminal::run_query_train(Params const &param, std::ostream &os) { return 0; }
int TicketTerminal::run_query_ticket(Params const &param, std::ostream &os) { return 0; }
int TicketTerminal::run_query_transfer(Params const &param, std::ostream &os) { return 0; }
int TicketTerminal::run_buy_ticket(Params const &param, std::ostream &os) { return 0; }
int TicketTerminal::run_query_order(Params const &param, std::ostream &os) { return 0; }
int TicketTerminal::run_refund_ticket(Params const &param, std::ostream &os) { return 0; }
int TicketTerminal::run_clean(Params const &param, std::ostream &os) { return 0; }


int TicketTerminal::run_exit(Params const &param, std::ostream &os) {
	os << "bye";
	return 1;
}
