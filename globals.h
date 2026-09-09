#pragma once
#include <vector>
#include <unordered_map>
#include "game_structures.hpp"
#include <d3d11.h>
#include <mutex>
#include <shared_mutex>
#include <unordered_set>
#include <utility>

namespace globals {
	using namespace protocol::engine::sdk;
	using namespace protocol::game::sdk;

	using tProcessEvent = void(*)(u_object*, u_function*, void*);
	inline tProcessEvent process_event = nullptr;

	inline u_world* gworld = 0;
	inline a_game_state_base* game_state = 0;
	inline u_game_instance* owning_instance = 0;
	inline u_localplayer* local_player = 0;
	inline a_player_controller* local_controller = 0;
	inline a_player_camera_manager* local_camera_manager = 0;
	inline mec_pawn* local_mec = 0;
	inline a_gm_c* gm_ref = nullptr;

	struct object_cache {
		std::vector<mec_pawn*> players;
		std::vector<world_item*> world_items;
		std::vector<task_vents*> task_vents;
		std::vector<task_machines*> task_machines;
		std::vector<task_alimentations*> task_alimentations;
		std::vector<task_deliveries*> task_deliveries;
		std::vector<task_pizzushis*> task_pizzushis;
		std::vector<task_data*> task_data;
		std::vector<task_scanner*> task_scanners;
		std::vector<a_alarm_button_c*> alarm_buttons;
		std::vector<a_rez_charger_c*> rez_chargers;
		std::vector<a_weapon_case_code_c*> weapon_cases;
	};

	inline object_cache cached_objects{};
	inline std::shared_mutex cached_objects_mutex;

	inline object_cache get_cached_objects() {
		std::shared_lock lock(cached_objects_mutex);
		return cached_objects;
	}

	inline void replace_cached_objects(object_cache next_cache) {
		std::unique_lock lock(cached_objects_mutex);
		cached_objects = std::move(next_cache);
	}

	struct s_font {
		ImFont* im_font = nullptr;
		float font_size = 14.0f;
	};

	inline s_font default_font;

	inline std::unordered_set<std::string> inserted_names;
}
