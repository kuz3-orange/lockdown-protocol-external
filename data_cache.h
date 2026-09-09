#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <mutex>
#include <shared_mutex>
#include "game_structures.hpp"

using namespace protocol::game::sdk;

extern std::unordered_map<std::string, u_data_item*> unique_item_data;
extern std::vector<std::string> item_names;

template <typename T>
std::vector<T> ReadRemoteArray(const protocol::engine::sdk::t_array<T>& remote_array) {
	constexpr int32_t max_safe_elements = 1'000'000;
	if (!remote_array._data || remote_array.count <= 0 || remote_array.count > max_safe_elements)
		return {};

	try {
		std::vector<T> result(static_cast<size_t>(remote_array.count));
		if (!mem::read_raw(remote_array._data, result.data(), result.size() * sizeof(T)))
			return {};

		return result;
	}
	catch (const std::bad_alloc&) {
		return {};
	}
}

inline std::unordered_map<u_data_item*, std::string> cached_item_names;
inline std::shared_mutex cached_item_names_mutex;

inline std::string GetCachedItemName(u_data_item* item_data) {
	if (!item_data)
		return {};

	{
		std::shared_lock lock(cached_item_names_mutex);
		auto cached_name = cached_item_names.find(item_data);
		if (cached_name != cached_item_names.end())
			return cached_name->second;
	}

	std::string item_name = item_data->get_name().read_string();
	if (item_name.empty())
		return {};

	std::unique_lock lock(cached_item_names_mutex);
	return cached_item_names.emplace(item_data, std::move(item_name)).first->second;
}
