#pragma once

#include <array>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string>

template<std::size_t N>
struct String : public std::array<char, N> {
	using std::array<char, N>::array;
	String() {
		memset(this->data(), 0, N);
	}
	String(std::string const &str) : String() {
		memcpy(this->data(), str.data(), std::min(N, str.size()));
	}
	String(const char *str) : String() {
		memcpy(this->data(), str, std::min(N, strlen(str)));
	}
	explicit operator std::string() const {
		if (this->back()) {
			std::string s(N, 0);
			memcpy(s.data(), this->data(), N);
			return s;
		}
		else
			return std::string(this->data());
	}
	bool allzero() const {
		for (int i = 0; i < N; ++i)
			if ((*this)[i]) return false;
		return true;
	}
};

template<std::size_t N>
std::ostream &operator<<(std::ostream &os, String<N> const &str) {
	if (str.back())
		os.write(str.data(), N);
	else
		os << str.data();
	return os;
}


struct Date {
	friend class DateTime;

private:
	int date = 0;// 06-01 ~ 08-31 : 92 days
public:
	Date() = default;
	explicit Date(char const *str) : date(atoi(str + 3) - 1) {
		int month = atoi(str);
		if (month == 8) date += 30 + 31;
		else if (month == 7)
			date += 30;
	}
	operator int() const { return date; }
	friend std::ostream &operator<<(std::ostream &os, Date const &time) {
		int month = 6, day = time.date;
		if (day > 30) {
			++month;
			day -= 31;
		}
		if (day > 30) {
			++month;
			day -= 31;
		}
		return os << std::setfill('0') << std::setw(2) << month << '-'
				  << std::setw(2) << day;
	}
};

struct Time {
	friend class DateTime;

private:
	int time = 0;// 00:00 - 23:59 : 1440 minutes
public:
	Time() = default;
	explicit Time(char const *str) : time(atoi(str) * 60 + atoi(str + 3)) {}
	operator int() const { return time; }
	friend std::ostream &operator<<(std::ostream &os, Time const &time) {
		return os << std::setfill('0') << std::setw(2) << time.time / 60 << ':'
				  << std::setw(2) << time.time % 60;
	}
};

struct DateTime {
	Date d;
	Time t;
	DateTime &operator+=(int x) {
		t.time += x;
		d.date += t.time / (24 * 60);
		t.time %= 24 * 60;
		return *this;
	}
};

inline std::ostream &operator<<(std::ostream &os, DateTime const &time) {
	return os << time.d << ' ' << time.t;
}
