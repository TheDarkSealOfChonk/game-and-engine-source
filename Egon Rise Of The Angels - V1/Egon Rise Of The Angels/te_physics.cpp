#include "te_physics.hpp"

namespace te {
    // check if there's a separating plane in between the selected axes
    bool TePhysics::OBBCollider::getSeparatingPlane(const glm::vec3& RPos, const glm::vec3& Plane, const te::TePhysics::OBB& box1, const OBB& box2)
    {
        glm::vec3 comp1 = glm::abs(RPos * Plane);

        glm::vec3 comp2 = (glm::abs((box1.AxisX * box1.Half_size.x) * Plane) +
            glm::abs((box1.AxisY * box1.Half_size.y) * Plane) +
            glm::abs((box1.AxisZ * box1.Half_size.z) * Plane) +
            glm::abs((box2.AxisX * box2.Half_size.x) * Plane) +
            glm::abs((box2.AxisY * box2.Half_size.y) * Plane) +
            glm::abs((box2.AxisZ * box2.Half_size.z) * Plane));

        return ((comp1.x > comp2.x) && (comp1.y > comp2.y) && (comp1.z > comp2.z));
    }

    // test for separating planes in all 15 axes
    bool TePhysics::OBBCollider::getCollision(const OBB& box1, const OBB& box2)
    {
        glm::vec3 RPos;
        RPos = box2.Pos - box1.Pos;

        return !(getSeparatingPlane(RPos, box1.AxisX, box1, box2) ||
            getSeparatingPlane(RPos, box1.AxisY, box1, box2) ||
            getSeparatingPlane(RPos, box1.AxisZ, box1, box2) ||
            getSeparatingPlane(RPos, box2.AxisX, box1, box2) ||
            getSeparatingPlane(RPos, box2.AxisY, box1, box2) ||
            getSeparatingPlane(RPos, box2.AxisZ, box1, box2) ||
            getSeparatingPlane(RPos, box1.AxisX ^ box2.AxisX, box1, box2) ||
            getSeparatingPlane(RPos, box1.AxisX ^ box2.AxisY, box1, box2) ||
            getSeparatingPlane(RPos, box1.AxisX ^ box2.AxisZ, box1, box2) ||
            getSeparatingPlane(RPos, box1.AxisY ^ box2.AxisX, box1, box2) ||
            getSeparatingPlane(RPos, box1.AxisY ^ box2.AxisY, box1, box2) ||
            getSeparatingPlane(RPos, box1.AxisY ^ box2.AxisZ, box1, box2) ||
            getSeparatingPlane(RPos, box1.AxisZ ^ box2.AxisX, box1, box2) ||
            getSeparatingPlane(RPos, box1.AxisZ ^ box2.AxisY, box1, box2) ||
            getSeparatingPlane(RPos, box1.AxisZ ^ box2.AxisZ, box1, box2));
    }

    /*OBBComponent TePhysics::OBBCollider::makeOBBComponent(TeScene* scene, TeScene::Entity entity) {
        TransformComponent* entityTransform = scene->getComponent<TransformComponent>(entity);
        ModelComponent* entityModelComponent = scene->getComponent<ModelComponent>(entity);
        std::shared_ptr<TeModel> model = entityModelComponent->model;
        glm::mat4 transformMatrix = entityTransform->mat4();
        std::vector<glm::vec3> vertices;

        glm::vec3 center(0.0f);
        for (const auto& vertex : vertices) {
            center += vertex;
        }
        center /= static_cast<float>(vertices.size());

        // Transform vertices by the inverse of the transformation matrix
        glm::mat4 inverseTransform = glm::inverse(transformMatrix);
        std::vector<glm::vec3> transformedVertices;
        for (const auto& vertex : vertices) {
            transformedVertices.push_back(glm::vec3(inverseTransform * glm::vec4(vertex, 1.0f)));
        }

        // Calculate covariance matrix
        glm::mat3 covariance(0.0f);
        for (const auto& vertex : transformedVertices) {
            glm::vec3 diff = vertex - center;
            covariance += glm::outerProduct(diff, diff);
        }
        covariance /= static_cast<float>(transformedVertices.size());

        // Calculate eigenvectors and eigenvalues
        glm::vec3 eigenvalues;
        glm::mat3 eigenvectors;
        TePhysics::eigenDecomposition(covariance, eigenvalues, eigenvectors);

        // Calculate the rotation matrix
        glm::mat3 rotationMatrix = eigenvectors;

        // Calculate the extents along each axis
        glm::vec3 extents(
            glm::sqrt(eigenvalues[0]),
            glm::sqrt(eigenvalues[1]),
            glm::sqrt(eigenvalues[2])
        );

        // extract the position, scale, and rotation from the transformation matrix
        glm::vec3 position = glm::vec3(transformMatrix[3]);
        glm::vec3 scale(
            glm::length(glm::vec3(transformMatrix[0])),
            glm::length(glm::vec3(transformMatrix[1])),
            glm::length(glm::vec3(transformMatrix[2]))
        );
        glm::vec3 rotation(
            glm::atan(transformMatrix[1][2], transformMatrix[2][2]),
            glm::asin(-transformMatrix[0][2]),
            glm::atan(transformMatrix[0][1], transformMatrix[0][0])
        );
    }*/

    void TePhysics::eigenDecomposition(const glm::mat3& A, glm::vec3& eigenvalues, glm::mat3& eigenvectors, int maxIterations = 100, float epsilon = 1e-6) {
        int n = A.length();

        eigenvectors = glm::mat3(1.0f);

        for (int i = 0; i < maxIterations; ++i) {
            glm::vec3 prevEigenvalues = eigenvalues;

            eigenvalues = glm::vec3(glm::vec3(A * eigenvectors[0]), glm::vec3(A * eigenvectors[1]), glm::vec3(A * eigenvectors[2]));

            float normFactor = glm::length(eigenvalues);

            eigenvalues /= normFactor;

            if (glm::all(glm::lessThan(glm::abs(eigenvalues - prevEigenvalues), glm::vec3(epsilon))))
                break;

            for (int j = 0; j < n; ++j) {
                eigenvectors[j] = glm::normalize(A * eigenvectors[j]);
                for (int k = 0; k < j; ++k) {
                    eigenvectors[j] -= glm::dot(eigenvectors[j], eigenvectors[k]) * eigenvectors[k];
                }
            }
        }

    }
}