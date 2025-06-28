//
// Created by jack on 6/25/2025.
//

#ifndef INSTANCEDTHINGYTEMPLATE_H
#define INSTANCEDTHINGYTEMPLATE_H












#include "PhysXStuff.h"
#include "CollisionCage.h"
#include "ModelLoader.h"
#include "Scene.h"

template<typename T>
concept HasStartingDirection = requires(T t) {
    { t.startingDirection } -> std::same_as<glm::vec3>;
};

template<typename T>
concept HasBlockTexture = requires(T t)
{
    { t.block } -> std::convertible_to<BlockType>;
};

using BehaviorFunction = std::function<void(Scene*, float, entt::registry&, std::unordered_map<TwoIntTup, std::vector<entt::entity>, TwoIntTupHash>&)>;

template <typename ThingyComponent, const char* vertshad, const char* fragshad, const char* modelpath>
void renderAndBehaveInstancedThingy(entt::registry& reg, Scene* scene, float deltaTime, BehaviorFunction& bf)
{

    struct ThingyPhysicsBody
    {
        PxRigidDynamic* body = nullptr;
        CollisionCage<2> collisionCage = {};
        ThingyPhysicsBody() = default;
        explicit ThingyPhysicsBody(const glm::vec3& position, std::optional<const glm::vec3> direction)
        {

            assertNoNanOrInf(position.x, position.y, position.z);
            PxVec3 forward = PxVec3(1,0,0);

            if (direction.has_value())
            {
                auto dir = direction.value();
                assertNoNanOrInf(dir.x, dir.y, dir.z);
                forward = PxVec3(dir.x, dir.y, dir.z);
            }


            static PxMaterial* material = nullptr;
            static PxShape* shape = nullptr;

            if (material == nullptr)
            {
                material = gPhysics->createMaterial(0.5f, 0.5f, 0.6f);
                const PxSphereGeometry sphereGeom(0.25f);
                shape = gPhysics->createShape(sphereGeom, *material);
            }


            PxVec3 worldUp = PxVec3(0, 1, 0);

            // Compute right and adjusted up vectors
            PxVec3 right = worldUp.cross(forward).getNormalized();

            assertNoNanOrInf(right.x, right.y, right.z);

            PxVec3 up = forward.cross(right);

            assertNoNanOrInf(up.x, up.y, up.z);

            // Create 3x3 rotation matrix from axes
            PxMat33 rotation(right, up, forward);  // columns: X=right, Y=up, Z=forward

            assertNoNanOrInf(rotation.column0.x, rotation.column0.y, rotation.column0.z,
                 rotation.column1.x, rotation.column1.y, rotation.column1.z,
                 rotation.column2.x, rotation.column2.y, rotation.column2.z);

            auto quat = PxQuat(rotation);
            assertNoNanOrInf(quat.x, quat.y, quat.z, quat.w);

            auto transf = PxTransform(PxVec3(position.x, position.y, position.z), quat);

            PxRigidDynamic* sphere = gPhysics->createRigidDynamic(transf);

            sphere->attachShape(*shape);
            if (direction.has_value())
            {
                auto dir = direction.value();
                sphere->setLinearVelocity(PxVec3(dir.x, dir.y, dir.z) * 20.0f);
            }

            sphere->setMass(5.f);
            gScene->addActor(*sphere);
            body = sphere;
        }
        ~ThingyPhysicsBody()
        {
            if (body == nullptr) return;

            body->release();
            body = nullptr;
        }
        ThingyPhysicsBody& operator=(ThingyPhysicsBody& other) = delete;
        ThingyPhysicsBody(ThingyPhysicsBody& other) = delete;
        ThingyPhysicsBody(ThingyPhysicsBody&& other) noexcept
        {
            if (this != &other)
            {
                body = other.body;
                other.body = nullptr;
            }
        }
        ThingyPhysicsBody& operator=(ThingyPhysicsBody&& other) noexcept
        {
            if (this == &other) return *this;
            body = other.body;
            other.body = nullptr;
            return *this;
        }
    };

    static std::unordered_map<TwoIntTup, std::vector<entt::entity>, TwoIntTupHash> Thingysatspots;



    static std::unordered_map<entt::entity, ThingyPhysicsBody> physicsbodies;



    struct Thingydisplayinstance
    {
        glm::vec3 pos;
        glm::vec4 quat;
        glm::vec2 uvoffset;
    };



    static std::vector<Thingydisplayinstance> lootDisplayInstances;

    static jl::Shader shader(
        vertshad, fragshad,
        (std::string(modelpath) + "GltfInstanceShader").c_str()

    );

    static const jl::ModelAndTextures basemodeltex = jl::loadModel<true>(modelpath, false);


    glBindVertexArray(basemodeltex.modelGLObjects.at(0).vao);

    glUseProgram(shader.shaderID);

    //glUniformMatrix4fv(glGetUniformLocation(gltfShader.shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.mvp));
    glActiveTexture(GL_TEXTURE0);

    glBindTexture(GL_TEXTURE_2D, basemodeltex.texids.at(0));


    glUniform1i(glGetUniformLocation(shader.shaderID, "texture1"), 0);

    const auto & camera = scene->our<jl::Camera>();

    glUniform1f(glGetUniformLocation(shader.shaderID, "hideClose"), 0.0f);
    glUniform3f(glGetUniformLocation(shader.shaderID, "camPos"),0.f, 0.f, 0.f);
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderID, "mvp"), 1, GL_FALSE, glm::value_ptr(camera.mvp));
    glUniform1f(glGetUniformLocation(shader.shaderID, "time"), glfwGetTime());
    const auto view = reg.view<ThingyComponent, NPPositionComponent>();
    lootDisplayInstances.clear();
    for (auto entity : view)
    {
        //std::cout << "entity: "  << (int)entity << std::endl;
        auto & pos = view.template get<NPPositionComponent>(entity);
        glm::vec4 quat(0.0f, 0.0f, 0.0f, 1.0f); // Default identity quaternion

        auto thingy = view.template get<ThingyComponent>(entity);

        const auto oldblockspot = TwoIntTup(glm::floor(pos.position.x), glm::floor(pos.position.z));

        //Remove old dropsatspots entry if there
        if(Thingysatspots.contains(oldblockspot))
        {
            auto & there = Thingysatspots.at(oldblockspot);
            auto it = std::find(there.begin(), there.end(), entity);
            if(it != there.end())
            {
                there.erase(it);
            }
            if(there.empty())
            {
                Thingysatspots.erase(oldblockspot);
            }
        }

        //Update item to physics body spot
        if(physicsbodies.contains(entity))
        {
            const auto pose = physicsbodies.at(entity).body->getGlobalPose();
            const auto newpos = pose.p;
            const auto newquat = pose.q;
            pos.position = glm::vec3(newpos.x, newpos.y, newpos.z);
            quat = glm::vec4(newquat.x, newquat.y, newquat.z, newquat.w);

        } else
        {
            if constexpr (HasStartingDirection<ThingyComponent>) {
                physicsbodies.insert_or_assign(entity, ThingyPhysicsBody(pos.position, thingy.startingDirection));
                // use dir...
            } else {
                physicsbodies.insert_or_assign(entity, ThingyPhysicsBody(pos.position, std::nullopt));
            }

        }

        //Re-add to dropsatspots
        const auto newblockspot = TwoIntTup(glm::floor(pos.position.x), glm::floor(pos.position.z));
        if(!Thingysatspots.contains(newblockspot))
        {
            Thingysatspots.insert({newblockspot, std::vector<entt::entity>{ {entity} }});
        } else
        {
            auto & bp = Thingysatspots.at(newblockspot);

            if(std::find(bp.begin(), bp.end(), entity) == bp.end())
            {
                bp.push_back(entity);
            }
        }
        glm::vec2 uvoffset = glm::vec2(0.0f);

        if constexpr(HasBlockTexture<ThingyComponent>)
        {
            const auto & tex = TEXS[(int)thingy.block].at(0);
            uvoffset = glm::vec2((float)tex.first * texSlotWidth, (float)tex.second * -texSlotWidth);
        }

        physicsbodies.at(entity).collisionCage.updateToSpot(theScene.world, pos.position, deltaTime);
        lootDisplayInstances.emplace_back(pos.position, quat, uvoffset);
    }

    static GLuint instancevbo = 0;

    const auto  &mglo = basemodeltex.modelGLObjects.at(0);

        glBindVertexArray(mglo.vao);

        if (instancevbo == 0 )
        {
            glGenBuffers(1, &instancevbo);
        }

            glBindBuffer(GL_ARRAY_BUFFER, instancevbo);

            glBufferData(GL_ARRAY_BUFFER, sizeof(Thingydisplayinstance) * lootDisplayInstances.size(), lootDisplayInstances.data(), GL_DYNAMIC_DRAW);
            // Position attribute (location 2)
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Thingydisplayinstance), (GLvoid*)0);
            glEnableVertexAttribArray(2);
            glVertexAttribDivisor(2, 1);

            // Quaternion attribute (location 3)
            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Thingydisplayinstance), (GLvoid*)(3 * sizeof(float)));
            glEnableVertexAttribArray(3);
            glVertexAttribDivisor(3, 1);

            // UVOFFSET of instance (location 5)
            glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(Thingydisplayinstance), (GLvoid*)(7 * sizeof(float)));
            glEnableVertexAttribArray(5);
            glVertexAttribDivisor(5, 1);


        glDrawElementsInstanced(mglo.drawmode, mglo.indexcount, mglo.indextype, nullptr, lootDisplayInstances.size());


        std::erase_if(physicsbodies, [&reg](const auto& pair) {
            return !(reg.valid(pair.first) && reg.all_of<ThingyComponent>(pair.first));
        });


    bf(scene, deltaTime, reg, Thingysatspots);
    for (auto& [entity, physBody] : physicsbodies) {
        auto pose = physBody.body->getGlobalPose();
        auto vel = physBody.body->getLinearVelocity();
        assertNoNanOrInf(pose.p.x, pose.p.y, pose.p.z);
        assertNoNanOrInf(vel.x, vel.y, vel.z);
    }

}



#endif //INSTANCEDTHINGYTEMPLATE_H
