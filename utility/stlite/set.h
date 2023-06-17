#pragma once

#ifdef USE_STL
#include <set>
namespace kupi {
using std::set;
}
#else
#include <set>
namespace kupi {

enum NodeColor { red,
				 black };

template<class Key,
		 class Compare = std::less<Key>,
		 template<typename Type> class Alloc = std::allocator>
class set {
public:
	using value_type = const Key;

private:
	struct Node {
	public:
		Node(Node *fa, value_type const &val) : fa(fa), data(val) {}
		Node(Node *fa, value_type &&val) noexcept : fa(fa), data(std::move(val)) {}
		// @attention must ensure fa != nullptr
		[[nodiscard]] int who() const { return fa->son[1] == this; }
		Node *brother() const { return fa->son[fa->son[0] == this]; }
		void swap_position(Node &rhs) {
			int n = fa ? this->who() : 0, m = rhs.fa ? rhs.who() : 0;
			std::swap(fa, rhs.fa);
			link(this, fa, m);
			link(&rhs, rhs.fa, n);
			std::swap(son, rhs.son);
			for (int i = 0; i < 2; ++i) {
				link(son[i], this, i);
				link(rhs.son[i], &rhs, i);
			}
			std::swap(color, rhs.color);
		}

	public:
		Node *fa = nullptr;
		Node *son[2] = {nullptr, nullptr};
		NodeColor color = red;
		value_type data;
	};

	class iterator_base {
		friend class set;
		iterator_base(Node *_node, const set *tr) : _ptr(_node), _set(tr) {}

	public:
		iterator_base() = default;
		bool operator==(const iterator_base &rhs) const {
			return _ptr == rhs._ptr && _set == rhs._set;
		}
		bool operator!=(const iterator_base &rhs) const {
			return _ptr != rhs._ptr || _set != rhs._set;
		}

	protected:
		Node *_ptr = nullptr;
		const set *_set = nullptr;
	};

	template<bool is_const>
	class iterator_common : public iterator_base {
	public:
		using difference_type = std::ptrdiff_t;
		using value_type = set::value_type;
		using pointer = typename std::conditional<is_const, const value_type *, value_type *>::type;
		using reference = typename std::conditional<is_const, const value_type &, value_type &>::type;
		using iterator_category = std::bidirectional_iterator_tag;

	public:
		using iterator_base::iterator_base;
		iterator_common(const iterator_common<false> &it) : iterator_base(it) {}
		iterator_common operator++(int) {
			iterator_common ret = *this;
			++*this;
			return ret;
		}
		iterator_common &operator++() {
			Node *&p = this->_ptr;
			if (!p) return *this;
			if (p->son[1]) {
				p = p->son[1];
				while (p->son[0]) p = p->son[0];
				return *this;
			}
			while (p->fa && p->who() == 1)
				p = p->fa;
			if (p->fa) p = p->fa;
			else
				p = nullptr;
			return *this;
		}
		iterator_common operator--(int) {
			iterator_common ret = *this;
			--*this;
			return ret;
		}
		iterator_common &operator--() {
			Node *&p = this->_ptr;
			if (!p) {
				p = this->_set->back_ptr();
				return *this;
			}
			if (p->son[0]) {
				p = p->son[0];
				while (p->son[1]) p = p->son[1];
				return *this;
			}
			while (p->fa && p->who() == 0) p = p->fa;
			if (p->fa) p = p->fa;
			return *this;
		}
		reference operator*() const { return this->_ptr->data; }
		pointer operator->() const noexcept { return &this->_ptr->data; }
	};

public:
	using iterator = iterator_common<false>;
	using const_iterator = iterator_common<true>;

	set() = default;
	set(set const &rhs) : _size(rhs._size) {
		if (!_size) return;
		copy_recursive(_rt, rhs._rt);
	}
	set(set &&rhs) noexcept
		: _rt(rhs._rt), _size(rhs._size), _alloc(std::move(rhs._alloc)) {
		rhs._size = 0;
		rhs._rt = nullptr;
	}
	set &operator=(set const &rhs) {
		if (this != &rhs) {
			release_recursive(_rt);
			_size = rhs._size;
			if (_size) copy_recursive(_rt, rhs._rt);
		}
		return *this;
	}
	set &operator=(set &&rhs) noexcept {
		if (this != &rhs) {
			release_recursive(_rt);
			_rt = rhs._rt;
			_size = rhs._size;
			_alloc = std::move(rhs._alloc);
			rhs._rt = nullptr;
			rhs._size = 0;
		}
		return *this;
	}
	~set() { clear(); }

	iterator begin() { return {begin_ptr(), this}; }
	const_iterator begin() const { return cbegin(); }
	const_iterator cbegin() const { return {begin_ptr(), this}; }
	iterator end() { return {end_ptr(), this}; }
	const_iterator end() const { return cend(); }
	const_iterator cend() const { return {end_ptr(), this}; }

	[[nodiscard]] bool empty() const { return !_size; }
	[[nodiscard]] size_t size() const { return _size; }

	void clear() {
		release_recursive(_rt);
		_size = 0;
	}

	std::pair<iterator, bool> insert(const Key &value) {
		Node **ptr = &_rt, *last = nullptr;
		while (*ptr) {
			Node *p = last = *ptr;
			if (opt(p->data, value))
				ptr = &p->son[1];
			else if (opt(value, p->data))
				ptr = &p->son[0];
			else
				return {{p, this}, false};
		}
		Node *ret = *ptr = _alloc.allocate(1);
		new (ret) Node{last, value};
		++_size;
		update_insert(ret);
		return {{ret, this}, true};
	}

	bool erase(iterator const &pos) {
		if (pos._ptr == nullptr || pos._set != this)
			//			throw invalid_iterator{};
			return false;
		Node *p = pos._ptr;
		// after swap, p have at most 1 child.
		if (p->son[0] && p->son[1]) {
			iterator pre = pos;
			--pre;
			pre._ptr->swap_position(*p);
			if (_rt == p) _rt = pre._ptr;
		}
		erase_on_tree(p);
		p->~Node();
		_alloc.deallocate(p, 1);
		--_size;
		return true;
	}

	bool erase(Key const &key) { return erase(find(key)); }

	size_t count(const Key &key) const { return find(key) != end(); }
	iterator find(const Key &key) {
		Node *p = _rt;
		while (p) {
			if (opt(key, p->data))
				p = p->son[0];
			else if (opt(p->data, key))
				p = p->son[1];
			else
				return {p, this};
		}
		return end();
	}
	const_iterator find(const Key &key) const {
		return const_cast<set *>(this)->find(key);
	}

private:
	Node *_rt = nullptr;
	size_t _size = 0;
	[[no_unique_address]] Compare opt;
	[[no_unique_address]] Alloc<Node> _alloc;

private:
	void rotate(Node *p) {
		int m = p->who();
		Node *fa = p->fa, *pa = p->fa->fa;
		link(p->son[m ^ 1], fa, m);
		link(p, pa, pa ? fa->who() : 0);
		link(fa, p, m ^ 1);
		if (!pa) _rt = p;
	}
	Node *begin_ptr() const {
		if (!_rt) return end_ptr();
		Node *p = _rt;
		while (p->son[0]) p = p->son[0];
		return p;
	}
	Node *end_ptr() const { return nullptr; }
	Node *back_ptr() const {
		if (!_rt) return end_ptr();
		Node *p = _rt;
		while (p->son[1]) p = p->son[1];
		return p;
	}
	static constexpr void link(Node *son, Node *fa, int n) {
		son && (son->fa = fa);
		fa && (fa->son[n] = son);
	}

	void copy_recursive(Node *&des, Node *src) {
		des = _alloc.allocate(1);
		new (des) Node{*src};
		for (int i = 0; i < 2; ++i)
			if (src->son[i]) {
				copy_recursive(des->son[i], src->son[i]);
				des->son[i]->fa = des;
			}
	}
	void release_recursive(Node *&nd) {
		if (!nd) return;
		if (nd->son[0]) release_recursive(nd->son[0]);
		if (nd->son[1]) release_recursive(nd->son[1]);
		nd->~Node();
		_alloc.deallocate(nd, 1);
		nd = nullptr;
	}

	// ensure p has at most one child
	void erase_on_tree(Node *p) {
		Node *s = p->son[0] ? p->son[0] : p->son[1];// at least one of them are nullptr.
		int k = 0;
		if (p->fa)
			link(s, p->fa, k = p->who());
		else
			(_rt = s) && (s->fa = nullptr);
		// no need to set s to black, if p is already red.
		// no need to adjust the tree.
		if (p->color == red) return;
		if (!s || s->color == black)
			update_erase(p->fa, k);
		else
			s->color = black;
	}
	void update_insert(Node *p) {
		Node *uncle = nullptr;
		while (p->fa && p->fa->color == red && (uncle = p->fa->brother()) && uncle->color == red) {
			// p has red father imply p has grandpa
			p->fa->fa->color = red;
			p->fa->color = black;
			uncle->color = black;
			p = p->fa->fa;
		}
		Node *fa = p->fa;
		if (!fa) {
			p->color = black;
			_rt = p;
			return;
		}
		if (fa->color == black) return;
		Node *pa = fa->fa;
		int m = p->who(), n = fa->who();
		if (m != n) {
			rotate(p);
			fa = p;
		}
		rotate(fa);
		fa->color = black;
		pa->color = red;
	}
	void update_erase(Node *p, int k) {
		while (true) {
			// case 1
			if (!p) {
				_rt = nullptr;
				break;
			}
			// case 2 : leading to case 4
			Node *s = p->son[k ^ 1];
			if (s->color == red) {
				rotate(s);
				s->color = black;
				p->color = red;
				s = p->son[k ^ 1];
			}
			// case 5: leading to case 6
			if (Node *sk = s->son[k]; sk && sk->color == red) {
				rotate(sk);
				sk->color = black;
				s->color = red;
				s = sk;
				// not break, go in case 6
			}
			// case 6
			if (s->son[k ^ 1] && s->son[k ^ 1]->color == red) {
				rotate(s);
				s->son[k ^ 1]->color = black;
				s->color = p->color;
				p->color = black;
				break;
			}
			// now the children of s are black
			// case 4
			if (p->color == red) {
				p->color = black;
				s->color = red;
				break;
			}
			// case 3, loop again
			s->color = red;
			if (!p->fa) {
				_rt = p;
				break;
			}
			k = p->who();
			p = p->fa;
		}
	}
};

}// namespace kupi
#endif