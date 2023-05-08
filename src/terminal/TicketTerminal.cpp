#include "TicketTerminal.h"
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

// clang-format off
enum RunResult : int { normal, exit, params_missing, params_invalid, permission_denied, user_not_found, re_login, un_login };
// clang-format on

static inline int ok(std::ostream &os) {
	os << '0';
	return 0;
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
	//	if (!params['c'] || !params['u'])
	//		return fail(os, params_missing);
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
	int privilege_cur = users.get_privilege(id_cur), privilege_tar = users.get_privilege(id);
	if (privilege_cur < privilege_tar) return fail(os, permission_denied);
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

int TicketTerminal::run_add_train(Params const &params, std::ostream &os) { return 0; }
int TicketTerminal::run_delete_train(Params const &params, std::ostream &os) { return 0; }
int TicketTerminal::run_release_train(Params const &params, std::ostream &os) { return 0; }
int TicketTerminal::run_query_train(Params const &params, std::ostream &os) { return 0; }
int TicketTerminal::run_query_ticket(Params const &params, std::ostream &os) { return 0; }
int TicketTerminal::run_query_transfer(Params const &params, std::ostream &os) { return 0; }
int TicketTerminal::run_buy_ticket(Params const &params, std::ostream &os) { return 0; }
int TicketTerminal::run_query_order(Params const &params, std::ostream &os) { return 0; }
int TicketTerminal::run_refund_ticket(Params const &params, std::ostream &os) { return 0; }
int TicketTerminal::run_clean(Params const &params, std::ostream &os) { return 0; }

int TicketTerminal::run_exit(Params const &params, std::ostream &os) {
	os << "bye";
	return RunResult::exit;
}


char const *TicketTerminal::run_result_to_string(int res) {
	constexpr const char *error_code[] = {"normal", "exit", "params_missing", "params_invalid", "permission_denied", "user_not_found", "re_login", "un_login"};
	return error_code[res];
}

}// namespace ticket
