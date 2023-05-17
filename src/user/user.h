#pragma once
#include "base.h"
#include "file/DataBase.h"
#include "bpt/bpt.h"
#include <optional>

namespace ticket {

struct User {
	String<20> username{};
	String<30> password{};
	String<15> name{};
	String<30> mail{};
	unsigned char privilege{};
	constexpr static int MaxPrivilege = 10;
};

std::ostream &operator<<(std::ostream &os, User const &user);

class UserManager {
public:
	/**
	 * @param main_file filename of total user data
	 * @param index_file filename of map: username->id.
	 */
	UserManager(std::string const &main_file, std::string const &index_file) : data{main_file}, name2id{index_file}, siz{data.size()} {}
	/**
	 * @return 0 if not found
	 */
	int find(char const *username);
	/**
	 * @return the new added user's id. 0 if failed.
	 */
	int add_user(char const *username, char const *password, char const *name, char const *mail, unsigned char privilege);
	/**
	 * @attention ensure id is valid
	 */
	User get(int id) { return data.read(id); }
	/**
	 * @attention ensure id is valid
	 */
	unsigned char get_privilege(int id);
	/**
	 * @attention ensure id is valid
	 * @return std::nullopt if params not valid
	 */
	std::optional<User> set(int id, char const *username, char const *password, char const *name, char const *mail, unsigned char privilege);
	/**
	 * @return id if password is right. -1 if user not exist, -2 if password incorrect.
	 */
	int check_password(char const *username, char const *password);
	bool empty() const { return !siz; }
private:
	kupi::DataBase<User, false, false> data;
	kupi::bpt<String<20>, int> name2id;
	int siz;
};

}// namespace ticket
