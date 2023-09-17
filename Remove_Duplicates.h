#pragma once
#include "search_server.h"

// operator != реализовал в размышлени€х над реализацией, даже не отлаживал, не пригодились, но жаль удал€ть.

template <typename T, typename Any>
bool operator != (const std::set<T>& left, const std::map<T, Any>& right)
{
	if (left.size() != right.size())
	{
		return false;
	}
	auto it_map = right.begin();
	for (auto it_set = left.begin(); it_set != left.end(); ++it_set)
	{
		if (*it_set != it_map->first)
		{
			return false;
		}
	}
}

template <typename T, typename Any>
bool operator != (const std::map<T, Any>& left, const std::set<T>& right)
{
	return right != left;
}

void RemoveDuplicates(SearchServer& search_server);

//return set<T> from map<T, ...>. map key type must be the same as set type.
template <typename T, typename Any>
std::set<T> MapKeysToSet (const std::map<T, Any>& map_var)
{
	std::set<T> result;
	for (const auto& [t, any] : map_var)
	{
		result.insert(t);
	}
	return result;
}