#pragma once
#ifdef USE_STL
#include <algorithm>
namespace kupi {
using std::lower_bound;
using std::sort;
}
#else
#include <algorithm>

namespace kupi {
template<typename T, typename Cmp>
T *lower_bound(T *beg, T *ed, T const &x, Cmp const &cmp) {
	while (beg < ed) {
		T *mid = beg + (ed - beg) / 2;
		if (!cmp(*mid, x)) ed = mid;
		else
			beg = mid + 1;
	}
	return ed;
}

template<typename T>
T *lower_bound(T *beg, T *ed, T const &x) {
	while (beg < ed) {
		T *mid = beg + (ed - beg) / 2;
		if (!(*mid < x)) ed = mid;
		else
			beg = mid + 1;
	}
	return ed;
}

using std::sort;
}// namespace kupi
#endif
