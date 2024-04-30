#pragma once

#include <glm/glm.hpp>
#include <cmath>
#include "te_device.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include "te_game_object.hpp"

namespace te {
	class TePhysics {
	public:
        struct OBB
        {
            glm::vec3 Pos, AxisX, AxisY, AxisZ, Half_size;
        };

        class Ray {
        public:
            glm::vec3 origin;
            glm::vec3 direction;

            Ray(const glm::vec3& origin, const glm::vec3& direction) : origin(origin), direction(glm::normalize(direction)) {}

            glm::vec3 pointAt(float t) const {
                return origin + t * direction;
            }
        };

        class OBBCollider {
        public:
            
            static bool getSeparatingPlane(const glm::vec3& RPos, const glm::vec3& Plane, const te::TePhysics::OBB& box1, const OBB& box2);

            static bool getCollision(const OBB& box1, const OBB& box2);

            //static OBBComponent makeOBBComponent(TeScene* scene, TeScene::Entity entity);
        };

        static void eigenDecomposition(const glm::mat3& A, glm::vec3& eigenvalues, glm::mat3& eigenvectors, int maxIterations, float epsilon);
    };
}