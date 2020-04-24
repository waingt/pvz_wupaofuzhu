#pragma once
#include <initializer_list>

#define inrange(x,low,high) ((x)<(low)?(low):((x)>(high)?(high):(x)))
template<typename T>
bool in(T t, std::initializer_list<T> list) {
	for (auto& i : list)
	{
		if (t == i)return true;
	}
	return false;
}