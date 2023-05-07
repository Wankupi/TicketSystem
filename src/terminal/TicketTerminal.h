#pragma once
#include "train/train.h"
#include "user/user.h"
#include <ostream>

class TicketTerminal {
public:
	TicketTerminal(UserManager &userManager, TrainManager &trainManager) : users(userManager), trains(trainManager) {}
	int run(char const *s, std::ostream &os);

private:
	UserManager &users;
	TrainManager &trains;

private:
	using Params = char const *[26];
	using Function = int (TicketTerminal::*)(Params const &param, std::ostream &os);
	int run_add_user(Params const &param, std::ostream &os);
	int run_login(Params const &param, std::ostream &os);
	int run_logout(Params const &param, std::ostream &os);
	int run_query_profile(Params const &param, std::ostream &os);
	int run_modify_profile(Params const &param, std::ostream &os);
	int run_add_train(Params const &param, std::ostream &os);
	int run_delete_train(Params const &param, std::ostream &os);
	int run_release_train(Params const &param, std::ostream &os);
	int run_query_train(Params const &param, std::ostream &os);
	int run_query_ticket(Params const &param, std::ostream &os);
	int run_query_transfer(Params const &param, std::ostream &os);
	int run_buy_ticket(Params const &param, std::ostream &os);
	int run_query_order(Params const &param, std::ostream &os);
	int run_refund_ticket(Params const &param, std::ostream &os);
	int run_clean(Params const &param, std::ostream &os);
	int run_exit(Params const &param, std::ostream &os);
	int run_nothing(Params const &param, std::ostream &os) { return 0; }
	
	static void parse_command(char *s, char *(&params)[26]);
	static Function get_function(char const *command);
};
