#pragma once

#include <fstream>
#include <map>
#include <string>

namespace kupi {
template<typename Key, typename Val>
class bpt : public std::map<Key, Val> {
public:
	explicit bpt(std::string const &tr_name) : std::map<Key, Val>{}, leave_file{tr_name + ".bpt"} {
		std::ifstream file(leave_file);
		if (!file) return;
		std::pair<Key, Val> t;
		file.seekg(0, std::ios::end);
		auto siz = file.tellg() / sizeof(t);
		file.seekg(0);
		for (int i = 0; i < siz; ++i) {
			file.read(reinterpret_cast<char *>(&t), sizeof(t));
			this->insert(t);
		}
	}
	~bpt() {
		std::ofstream file(leave_file, std::ios::trunc);
		if (!file) return;
		for (auto const &x: *this) {
			file.write(reinterpret_cast<char const *>(&x), sizeof(x));
		}
	}

private:
	std::string leave_file;
};
}// namespace kupi