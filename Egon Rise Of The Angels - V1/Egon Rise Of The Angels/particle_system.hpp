#pragma once

#include "te_game_object.hpp"

namespace te {
	class ParticleSystem {
	public:
		ParticleSystem(TeScene& scene);


	private:
		TeScene& scene;
	};
}