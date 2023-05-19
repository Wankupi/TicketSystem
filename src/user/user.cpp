#include "user.h"
namespace ticket {

std::ostream &operator<<(std::ostream &os, User const &user) {
	return os << user.username << ' ' << user.name << ' ' << user.mail << ' ' << int(user.privilege);
}

unsigned char UserManager::get_privilege(int id) {
	unsigned char p;
	data.read(id, p, offsetof(User, privilege));
	return p;
}

int UserManager::find(const char *username) {
	auto res = name2id.find(username);
	//	if (!res.empty()) return res[0];
	if (res != name2id.end()) return res->second;
	return 0;
}

int UserManager::add_user(const char *username, const char *password, const char *name, const char *mail, unsigned char privilege) {
	if (find(username)) return -1;
	if (!username || !password || !name || !mail || privilege > User::MaxPrivilege) return -2;
	User user{username, password, name, mail, privilege};
	int id = data.insert(user);
	if (id) {
		name2id.insert(username, id);
		++siz;
	}
	return id;
}

std::optional<User> UserManager::set(int id, const char *username, const char *password, const char *name, const char *mail, unsigned char privilege) {
	User user{data.read(id)};
	if (username) {
		name2id.erase(user.username);
		user.username = username;
		name2id.insert(user.username, id);
	}
	if (password) user.password = password;
	if (name) user.name = name;
	if (mail) user.mail = mail;
	if (privilege <= 10) user.privilege = privilege;
	if (username || password || name || mail || privilege)
		data.write(id, user);// if no change then not write
	return user;
}

int UserManager::check_password(const char *username, const char *password) {
	int id = find(username);
	if (!id) return -1;
	decltype(User::password) pwd;
	data.read(id, pwd, offsetof(User, password));
	if (pwd != decltype(User::password)(password)) return -2;
	return id;
}

}// namespace ticket
