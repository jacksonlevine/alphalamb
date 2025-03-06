//
// Created by jack on 1/27/2025.
//

#include "WorldRenderer.h"

#include "MaterialName.h"
#include "../AmbOcclSetting.h"
#include "worldgenmethods/OverworldWorldGenMethod.h"
#include "../IndexOptimization.h"
std::atomic<int> NUM_THREADS_RUNNING = 0;


void genCGLBuffers()
{

    //
    // struct ChunkGLInfo
    // {
    //     GLuint vvbo, uvvbo, bvbo, ebo = 0;
    //     GLuint tvvbo, tuvvbo, tbvbo, tebo = 0;
    //     DrawInstructions drawInstructions = {};
    // };


    GLuint garbageCan = 0;

    glGenVertexArrays(1, &garbageCan);  //1
    glGenVertexArrays(1, &garbageCan); //2


    glGenBuffers(1, &garbageCan);
    glGenBuffers(1, &garbageCan);
    glGenBuffers(1, &garbageCan);
    glGenBuffers(1, &garbageCan);


    glGenBuffers(1, &garbageCan);
    glGenBuffers(1, &garbageCan);
    glGenBuffers(1, &garbageCan);
    glGenBuffers(1, &garbageCan);

    //The buffers are generated, we know from the chunkIndex what their names will be, we needn't store the names

}
///Prepare vbos and DrawInstructions info (vao, number of indices) so that it can be then drawn with drawFromDrawInstructions()
///
void modifyOrInitializeDrawInstructions(GLuint& vvbo, GLuint& uvvbo, GLuint& ebo, DrawInstructions& drawInstructions, UsableMesh& usable_mesh, GLuint& bvbo,
    GLuint& tvvbo, GLuint& tuvvbo, GLuint& tebo, GLuint& tbvbo)
{
    if(drawInstructions.vao == 0)
    {
        glGenVertexArrays(1, &drawInstructions.vao);
        glBindVertexArray(drawInstructions.vao);
        glGenBuffers(1, &vvbo);
        glGenBuffers(1, &uvvbo);
        glGenBuffers(1, &ebo);
        glGenBuffers(1, &bvbo);
        glGenVertexArrays(1, &drawInstructions.tvao);
        glBindVertexArray(drawInstructions.tvao);
        glGenBuffers(1, &tvvbo);
        glGenBuffers(1, &tuvvbo);
        glGenBuffers(1, &tebo);
        glGenBuffers(1, &tbvbo);
    }

    glBindVertexArray(drawInstructions.vao);

    glBindBuffer(GL_ARRAY_BUFFER, vvbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(std::size(usable_mesh.positions) * sizeof(PxVec3)), usable_mesh.positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PxVec3), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, bvbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(std::size(usable_mesh.brightness) * sizeof(float)), usable_mesh.brightness.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float)*2, nullptr);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float)*2, static_cast<void*>(static_cast<char*>(nullptr)+ (1*sizeof(float))));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, uvvbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(std::size(usable_mesh.texcoords) * sizeof(glm::vec2)), usable_mesh.texcoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(std::size(usable_mesh.indices) * sizeof(PxU32)), usable_mesh.indices.data(), GL_STATIC_DRAW);

    drawInstructions.indiceCount = static_cast<int>(std::size(usable_mesh.indices));

    glBindVertexArray(drawInstructions.tvao);

    glBindBuffer(GL_ARRAY_BUFFER, tvvbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(std::size(usable_mesh.tpositions) * sizeof(PxVec3)), usable_mesh.tpositions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PxVec3), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, tbvbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(std::size(usable_mesh.tbrightness) * sizeof(float)), usable_mesh.tbrightness.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float)*2, nullptr);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float)*2, static_cast<void*>(static_cast<char*>(nullptr) + (1*sizeof(float))));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, tuvvbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(std::size(usable_mesh.ttexcoords) * sizeof(glm::vec2)), usable_mesh.ttexcoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(std::size(usable_mesh.tindices) * sizeof(PxU32)), usable_mesh.tindices.data(), GL_STATIC_DRAW);

    drawInstructions.tindiceCount = static_cast<int>(std::size(usable_mesh.tindices));
}




void modifyOrInitializeChunkIndex(int chunkIndex, SmallChunkGLInfo& info, UsableMesh& usable_mesh)
{

    GLuint vao = vaoNameOfChunkIndex(chunkIndex, IndexOptimization::Vao::VAO);
    GLuint tvao = vaoNameOfChunkIndex(chunkIndex, IndexOptimization::Vao::TVAO);

    GLuint vvbo = bufferNameOfChunkIndex(chunkIndex, IndexOptimization::Buffer::VVBO);
    GLuint uvvbo = bufferNameOfChunkIndex(chunkIndex, IndexOptimization::Buffer::UVVBO);
    GLuint bvbo = bufferNameOfChunkIndex(chunkIndex, IndexOptimization::Buffer::BVBO);
    GLuint ebo = bufferNameOfChunkIndex(chunkIndex, IndexOptimization::Buffer::EBO);

    GLuint tvvbo = bufferNameOfChunkIndex(chunkIndex, IndexOptimization::Buffer::TVVBO);
    GLuint tuvvbo = bufferNameOfChunkIndex(chunkIndex, IndexOptimization::Buffer::TUVVBO);
    GLuint tbvbo = bufferNameOfChunkIndex(chunkIndex, IndexOptimization::Buffer::TBVBO);
    GLuint tebo = bufferNameOfChunkIndex(chunkIndex, IndexOptimization::Buffer::TEBO);

    //This never happens because this is for the ones that we pregen
    // if(vao == 0)
    // {
    //     glGenVertexArrays(1, &vao);
    //     glBindVertexArray(vao);
    //     glGenBuffers(1, &vvbo);
    //     glGenBuffers(1, &uvvbo);
    //     glGenBuffers(1, &ebo);
    //     glGenBuffers(1, &bvbo);
    //     glGenVertexArrays(1, &tvao);
    //     glBindVertexArray(tvao);
    //     glGenBuffers(1, &tvvbo);
    //     glGenBuffers(1, &tuvvbo);
    //     glGenBuffers(1, &tebo);
    //     glGenBuffers(1, &tbvbo);
    // }

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vvbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(std::size(usable_mesh.positions) * sizeof(PxVec3)), usable_mesh.positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PxVec3), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, bvbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(std::size(usable_mesh.brightness) * sizeof(float)), usable_mesh.brightness.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float)*2, nullptr);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float)*2, static_cast<void*>(static_cast<char*>(nullptr) + 1*sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, uvvbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(std::size(usable_mesh.texcoords) * sizeof(glm::vec2)), usable_mesh.texcoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(std::size(usable_mesh.indices) * sizeof(PxU32)), usable_mesh.indices.data(), GL_STATIC_DRAW);

    info.indiceCount = static_cast<int>(std::size(usable_mesh.indices));

    glBindVertexArray(tvao);

    glBindBuffer(GL_ARRAY_BUFFER, tvvbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(std::size(usable_mesh.tpositions) * sizeof(PxVec3)), usable_mesh.tpositions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PxVec3), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, tbvbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(std::size(usable_mesh.tbrightness) * sizeof(float)), usable_mesh.tbrightness.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float)*2, nullptr);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float)*2, static_cast<void*>(static_cast<char*>(nullptr) + 1*sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, tuvvbo);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(std::size(usable_mesh.ttexcoords) * sizeof(glm::vec2)), usable_mesh.ttexcoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(std::size(usable_mesh.tindices) * sizeof(PxU32)), usable_mesh.tindices.data(), GL_STATIC_DRAW);

    info.tindiceCount = static_cast<int>(std::size(usable_mesh.tindices));
}

///This assumes shader is set up and uniforms are given values
void drawFromDrawInstructions(const DrawInstructions& drawInstructions)
{
    glBindVertexArray(drawInstructions.vao);
    glDrawElements(GL_TRIANGLES, drawInstructions.indiceCount, GL_UNSIGNED_INT, nullptr);
}

void drawFromChunkIndex(int chunkIndex, const SmallChunkGLInfo& cgl)
{
    GLuint vao = vaoNameOfChunkIndex(chunkIndex, IndexOptimization::Vao::VAO);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, cgl.indiceCount, GL_UNSIGNED_INT, nullptr);
}

void drawTransparentsFromChunkIndex(int chunkIndex, const SmallChunkGLInfo& cgl)
{
    GLuint tvao = vaoNameOfChunkIndex(chunkIndex, IndexOptimization::Vao::TVAO);
    glBindVertexArray(tvao);
    glDrawElements(GL_TRIANGLES, cgl.tindiceCount, GL_UNSIGNED_INT, nullptr);
}



void drawTransparentsFromDrawInstructions(const DrawInstructions& drawInstructions)
{
    glBindVertexArray(drawInstructions.tvao);
    glDrawElements(GL_TRIANGLES, drawInstructions.tindiceCount, GL_UNSIGNED_INT, nullptr);
}

bool isChunkInFrustum(const TwoIntTup& chunkSpot, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection) {

    glm::vec3 chunkBottomPos = glm::vec3(
        static_cast<float>(chunkSpot.x) * static_cast<float>(WorldRenderer::chunkSize) + static_cast<float>(WorldRenderer::chunkSize) / 2.0f,
        cameraPosition.y + cameraDirection.y * 125,
        static_cast<float>(chunkSpot.z) * static_cast<float>(WorldRenderer::chunkSize) + static_cast<float>(WorldRenderer::chunkSize) / 2.0f
    );

    glm::vec3 toChunk = chunkBottomPos - cameraPosition;

    glm::vec3 normalizedToChunk = glm::normalize(toChunk);
    glm::vec3 normalizedDirection = glm::normalize(cameraDirection);


    float dotProduct1 = glm::dot(normalizedToChunk, normalizedDirection);
    dotProduct1 = glm::clamp(dotProduct1, -1.0f, 1.0f);
    float angle1 = glm::degrees(std::acos(dotProduct1));


    constexpr float FOV_ANGLE = 65.0f; // Half of your camera's total FOV

    return angle1 <= FOV_ANGLE;
}

//MAIN THREAD COROUTINE
void WorldRenderer::mainThreadDraw(const jl::Camera* playerCamera, GLuint shader, WorldGenMethod* worldGenMethod, float deltaTime, bool actuallyDraw)
{
    for(size_t i = 0; i < changeBuffers.size(); i++) {
        auto& buffer = changeBuffers[i];
        //std::cout << "Buffer state: Ready: " << buffer.ready << " in use: " << buffer.in_use << std::endl;
        if(buffer.ready.load() && !buffer.in_use.load()) {
            //std::cout << "Mesh buffer came through on main thread: " << buffer.to.x << " " << buffer.to.z << std::endl;
            modifyOrInitializeChunkIndex(static_cast<int>(buffer.chunkIndex), chunkPool.at(buffer.chunkIndex), buffer.mesh);


            if (buffer.from == std::nullopt)
            {
                activeChunks.insert_or_assign(buffer.to, ReadyToDrawChunkInfo(buffer.chunkIndex));
            } else
            {
                activeChunks.erase(buffer.from.value());
                activeChunks.insert_or_assign(buffer.to, ReadyToDrawChunkInfo(buffer.chunkIndex));
            }
            confirmedActiveChunksQueue.push(buffer.to);
            buffer.ready.store(false);
            freedChangeBuffers.push(i);  // Return to free list
            mbtBufferCV.notify_one();
            break; //Only do one per frame
        }
    }


    for(size_t i = 0; i < userChangeMeshBuffers.size(); i++) {
        auto& buffer = userChangeMeshBuffers[i];
        //std::cout << "User Buffer state: Ready: " << buffer.ready << " in use: " << buffer.in_use << std::endl;
        if(buffer.ready.load() && !buffer.in_use.load()) {
            //std::cout << "Buffer came through on main thread: " << buffer.to.x << " " << buffer.to.z << std::endl;
            modifyOrInitializeChunkIndex(static_cast<int>(buffer.chunkIndex), chunkPool.at(buffer.chunkIndex), buffer.mesh);

            if (buffer.from == std::nullopt)
            {
                activeChunks.insert_or_assign(buffer.to, ReadyToDrawChunkInfo(buffer.chunkIndex));

            } else
            {
                // activeChunks.erase(buffer.from.value());
                // activeChunks.insert_or_assign(buffer.to, ReadyToDrawChunkInfo(buffer.chunkIndex));
            }
            //Dont need to do this cause these user-requested chunk rebuilds never involve moving the chunk
            //confirmedActiveChunksQueue.push(buffer.to);
            buffer.ready.store(false);
            freedUserChangeMeshBuffers.push(i);  // Return to free list
            notifyBufferFreed();

            //break; //Only do one per frame
        }
    }
    TwoIntTup playerChunkPosition = worldToChunkPos(
        TwoIntTup(std::floor(playerCamera->transform.position.x),
            std::floor(playerCamera->transform.position.z)));

    static GLint grassRedChangeLoc = glGetUniformLocation(shader, "grassRedChange");
    static GLint timeRenderedLoc = glGetUniformLocation(shader, "timeRendered");
    static GLint rdistLoc = glGetUniformLocation(shader, "renderDistance");
    glUniform1f(rdistLoc, static_cast<float>(currentRenderDistance));
    for (auto& [spot, chunkinfo] : activeChunks) {
        bool isDrawingThis = true;

        if (isChunkInFrustum(spot, playerCamera->transform.position - (playerCamera->transform.direction * static_cast<float>(chunkSize)), playerCamera->transform.direction)) {
            int dist = abs(spot.x - playerChunkPosition.x) + abs(spot.z - playerChunkPosition.z);
            if (dist <= currentMinDistance()) {
                auto & glInfo = chunkPool[chunkinfo.chunkIndex];
                IntTup chunkRealSpot = IntTup(spot.x * chunkSize, spot.z * chunkSize);
                float grassRedChange = (worldGenMethod->getTemperatureNoise(chunkRealSpot) - worldGenMethod->getHumidityNoise(chunkRealSpot));
                glUniform1f(grassRedChangeLoc, grassRedChange);
                glUniform1f(timeRenderedLoc, chunkinfo.timeBeenRendered);
                if (actuallyDraw)
                {
                    drawFromChunkIndex(static_cast<int>(chunkinfo.chunkIndex), glInfo);
                }

                chunkinfo.timeBeenRendered += deltaTime;
            } else
            {
                isDrawingThis = false;
            }
        }

        if (!isDrawingThis)
        {
            chunkinfo.timeBeenRendered = 0.0f;
        }
    }
    for (const auto& [spot, chunkinfo] : activeChunks) {
        if (isChunkInFrustum(spot, playerCamera->transform.position - (playerCamera->transform.direction * static_cast<float>(chunkSize)), playerCamera->transform.direction)) {
            int dist = abs(spot.x - playerChunkPosition.x) + abs(spot.z - playerChunkPosition.z);
            if (dist <= currentMinDistance()) {
                auto glInfo = chunkPool[chunkinfo.chunkIndex];
                IntTup chunkRealSpot = IntTup(spot.x * chunkSize, spot.z * chunkSize);
                float grassRedChange = (worldGenMethod->getTemperatureNoise(chunkRealSpot) - worldGenMethod->getHumidityNoise(chunkRealSpot));
                glUniform1f(grassRedChangeLoc, grassRedChange);
                glUniform1f(timeRenderedLoc, chunkinfo.timeBeenRendered);
                if (actuallyDraw)
                {
                    drawTransparentsFromChunkIndex(static_cast<int>(chunkinfo.chunkIndex), glInfo);
                }

            }
        }
    }
}



//SECOND THREAD MESH BUILDING COROUTINE
void WorldRenderer::meshBuildCoroutine(jl::Camera* playerCamera, World* world)
{

    std::cout << "Mesh thread started!\n";
    NUM_THREADS_RUNNING.fetch_add(1);  // Atomic increment
    //std::cout << "Mesh incremented NUM_THREADS_RUNNING. Current value: " << NUM_THREADS_RUNNING.load() << "\n";

    for(size_t i = 0; i < changeBuffers.size(); i++) {
        freedChangeBuffers.push(i);
    }
    for(size_t i = 0; i < userChangeMeshBuffers.size(); i++) {
        freedUserChangeMeshBuffers.push(i);
    }

        mbtActiveChunks.reserve(maxChunks);
        activeChunks.reserve(maxChunks);


    while(meshBuildingThreadRunning)
    {

        std::unordered_set<TwoIntTup, TwoIntTupHash> implicatedChunks;


        TwoIntTup playerChunkPosition = worldToChunkPos(
        TwoIntTup(std::floor(playerCamera->transform.position.x),
            std::floor(playerCamera->transform.position.z)));

        static std::array<TwoIntTup, (renderDistance*2)*(renderDistance*2)> checkspots = {};

        int index = 0;
        for (int i = -currentRenderDistance; i < currentRenderDistance; i++)
        {
            for (int j = -currentRenderDistance; j < currentRenderDistance; j++)
            {
                checkspots[index] = playerChunkPosition + TwoIntTup(i,j);
                index+=1;
            }
        }



        std::ranges::sort(checkspots, [&playerChunkPosition](const TwoIntTup& a, const TwoIntTup& b)
        {
            int distfroma = abs(a.x - playerChunkPosition.x) + abs(a.z - playerChunkPosition.z);
            int distfromb = abs(b.x - playerChunkPosition.x) + abs(b.z - playerChunkPosition.z);
            return distfroma < distfromb;
        });


        for (auto & spotHere : checkspots)
        {

            for (auto & chunk : implicatedChunks)
            {
                if (mbtActiveChunks.contains(chunk))
                {
                    auto & uci = mbtActiveChunks.at(chunk);
                    UsableMesh mesh;
                    {
                        if (auto locks = world->tryToGetReadLockOnDMs()) {
                            //std::cout << "Got readlock on dms \n";
                            mesh = fromChunkLocked(chunk, world, chunkSize);
                        }
                    }


                    size_t changeBufferIndex = -1;
                    {
                        std::unique_lock<std::mutex> lock(mbtBufferMutex);
                        mbtBufferCV.wait(lock, [&]() {
                            return freedChangeBuffers.pop(changeBufferIndex) || !meshBuildingThreadRunning;
                        });

                        if (!meshBuildingThreadRunning) break;
                    }

                    if (changeBufferIndex != -1)
                    {
                        auto& buffer = changeBuffers[changeBufferIndex];
                        buffer.in_use.store(true);
                        buffer.mesh = mesh;
                        buffer.chunkIndex = uci.chunkIndex;
                        buffer.from = chunk;
                        buffer.to = chunk;
                        buffer.ready.store(true);
                        buffer.in_use.store(false);
                    }
                }

            }
            implicatedChunks.clear();

                TwoIntTup confirmedChunk;
                while (confirmedActiveChunksQueue.pop(confirmedChunk) && meshBuildingThreadRunning) {
                    if (mbtActiveChunks.contains(confirmedChunk))
                    {
                        mbtActiveChunks.at(confirmedChunk).confirmedByMainThread = true;
                    }


                }

                    int dist = abs(spotHere.x - playerChunkPosition.x) + abs(spotHere.z - playerChunkPosition.z);
                    if(dist <= currentMinDistance())
                    {
                        if (!mbtActiveChunks.contains(spotHere))
                    {
                            if(!generatedChunks.contains(spotHere))
                            {
                                generateChunk(world, spotHere, implicatedChunks);
                            }
                        //std::cout << "Spot " << i << " " << j << std::endl;

                        //IF we havent reached the max chunks yet, we can just make a new one for this spot.
                        //ELSE, we reuse the furthest one from the player, ONLY IF the new distance will be shorter than the last distance!
                        if (chunkPoolSize.load() < currentMaxChunks())
                        {
                            size_t changeBufferIndex = -1;
                            {
                                std::unique_lock<std::mutex> lock(mbtBufferMutex);
                                mbtBufferCV.wait(lock, [&]() {
                                    return freedChangeBuffers.pop(changeBufferIndex) || !meshBuildingThreadRunning;
                                });

                                if (!meshBuildingThreadRunning) break;
                            }
                            if (changeBufferIndex != -1)
                            {
                                //Add the mesh, in full form, to our reserved Change Buffer (The main thread coroutine will make GL calls and free this slot to be reused)

                                auto& buffer = changeBuffers[changeBufferIndex];
                                buffer.in_use.store(true);
                                buffer.mesh = fromChunk(spotHere, world, chunkSize);

                                buffer.chunkIndex = addUninitializedChunkBuffer();
                                buffer.from = std::nullopt;
                                buffer.to = spotHere;


                                mbtActiveChunks.insert_or_assign(spotHere, UsedChunkInfo(buffer.chunkIndex));


                                buffer.ready.store(true);   // Signal that data is ready
                                buffer.in_use.store(false);
                            }




                        } else
                        {


                            std::vector<std::pair<int,TwoIntTup>> chunksWithDistances;

                            for (const auto& [chunkPos, usedChunkInfo] : mbtActiveChunks) {

                                if (!usedChunkInfo.confirmedByMainThread)
                                {
                                    //We only want ones confirmed by main thread, we know we can repurpose those
                                    continue;
                                }

                                // Calculate distance to yourPosition
                                int distance = abs(chunkPos.x - playerChunkPosition.x) + abs(chunkPos.z - playerChunkPosition.z);


                                // Filter out chunks closer than MIN_DISTANCE
                                if (distance > currentMinDistance()) {
                                    chunksWithDistances.emplace_back(distance, chunkPos);
                                }

                            }



                            //If any still matching the criteria
                            if(!chunksWithDistances.empty())
                            {
                                // Sort chunks by distance
                                std::ranges::sort(chunksWithDistances,
                                                  [](const std::pair<int, TwoIntTup>& a, const std::pair<int, TwoIntTup>& b) {
                                                      return a.first > b.first;
                                                  });

                                for (const auto& [oldDistance, oldSpot] : chunksWithDistances)
                                {
                                    //If the place we're going will afford a shorter distance from player, choose this one.

                                    int newDistance = abs(spotHere.x - playerChunkPosition.x) + abs(spotHere.z - playerChunkPosition.z);

                                    if (newDistance < oldDistance)
                                    {
                                        size_t chunkIndex = mbtActiveChunks.at(oldSpot).chunkIndex;

                                        size_t changeBufferIndex = -1;
                                        {
                                            std::unique_lock<std::mutex> lock(mbtBufferMutex);
                                            mbtBufferCV.wait(lock, [&]() {
                                                return freedChangeBuffers.pop(changeBufferIndex) || !meshBuildingThreadRunning;
                                            });

                                            if (!meshBuildingThreadRunning) break;
                                        }

                                        if (changeBufferIndex != -1)
                                        {
                                            //Add the mesh, in full form, to our reserved Change Buffer (The main thread coroutine will make GL calls and free this slot to be reused)

                                            auto& buffer = changeBuffers[changeBufferIndex];
                                            buffer.in_use.store(true);
                                            buffer.mesh = fromChunk(spotHere, world, chunkSize);

                                            buffer.chunkIndex = chunkIndex;
                                            buffer.from = oldSpot;
                                            buffer.to = spotHere;



                                            mbtActiveChunks.erase(oldSpot);
                                            mbtActiveChunks.insert_or_assign(spotHere, UsedChunkInfo(chunkIndex));

                                            buffer.ready.store(true);   // Signal that data is ready
                                            buffer.in_use.store(false);
                                        }



                                        break;
                                    }

                                }
                            }



                        }

                    }
                    }



        }

        std::this_thread::sleep_for(std::chrono::seconds(1));


    }

    mbtActiveChunks.clear();

    NUM_THREADS_RUNNING.fetch_sub(1);
    std::cout << "Mesh build thread finished!\n";

}

void WorldRenderer::generateChunk(World* world, const TwoIntTup& chunkSpot, std::unordered_set<TwoIntTup, TwoIntTupHash>& implicatedChunks)
{

    srand(chunkSpot.x * 73856093 ^ chunkSpot.z * 19349663);
    for (int o = 0; o < 3; o++)
    {
        auto offset1 = ((float)rand() / RAND_MAX);
        auto offset2 = ((float)rand() / RAND_MAX);
        if (rand() > 9000)
        {
            int y = 50;
            float offsetNoise = world->worldGenMethod->getHumidityNoise(IntTup(chunkSpot.x, chunkSpot.z)) * 5.0f;
            float offsetNoise2 = world->worldGenMethod->getHumidityNoise(IntTup(chunkSpot.x, chunkSpot.z)) * 5.0f;
            IntTup realSpot(chunkSpot.x * chunkSize + (chunkSize >> 1) + static_cast<int>(offsetNoise) + offset1 * 10.0f , chunkSpot.z * chunkSize  + (chunkSize >> 1) + static_cast<int>(offsetNoise2) + offset2 * 10.0f);
            bool surfaceBlockFound = false;

            MaterialName fb = OverworldWorldGenMethod::getFloorBlockInClimate(world->worldGenMethod->getClimate(realSpot));
            while(y < 170 && !surfaceBlockFound)
            {
                realSpot.y = y;
                if(world->get(realSpot) == fb)
                {
                    surfaceBlockFound = true;
                }
                y++;
            }
            if(surfaceBlockFound)
            {
                Climate climate = world->worldGenMethod->getClimate(realSpot);
                std::vector<TerrainFeature>& possibleFeatures = getTerrainFeaturesFromClimate(climate);
                size_t selectedIndex = static_cast<int>((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * static_cast<float>(possibleFeatures.
                    size()));
                selectedIndex = std::min(selectedIndex, possibleFeatures.size() - 1);
                TerrainFeature selectedFeature = possibleFeatures[selectedIndex];

                std::vector<VoxelModelName>& possibleModels = getVoxelModelNamesFromTerrainFeatureName(selectedFeature);
                size_t selectedModelIndex = static_cast<int>((static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * static_cast<float>(possibleModels.
                    size()));
                selectedModelIndex = std::min(selectedModelIndex, possibleModels.size() - 1);
                VoxelModelName selectedModel = possibleModels[selectedModelIndex];

                IntTup& voxDim = voxelModels[selectedModel].dimensions;
                IntTup offset = IntTup(-(voxDim.x / 2), 0, -(voxDim.z / 2));

                for(auto & point : voxelModels[selectedModel].points)
                {
                    if(point.colorIndex != AIR)
                    {
                        auto finalspot = realSpot + point.localSpot + offset;
                        auto chunkhere = WorldRenderer::worldToChunkPos(TwoIntTup(finalspot.x, finalspot.z));
                        if (chunkhere != chunkSpot)
                        {
                            implicatedChunks.insert(chunkhere);
                        }
                        world->nonUserDataMap->set(finalspot, point.colorIndex);
                    }
                }
            }
        }
    }


    // for (auto & chunk: implicatedChunks)
    // {
    //
    // }

    generatedChunks.insert(chunkSpot);
}

///Call this with an external index and UsableMesh to mutate them
template<bool doBrightness>
__inline void addFace(PxVec3 offset, Side side, MaterialName material, int sideHeight, UsableMesh& mesh, PxU32& index, PxU32& tindex)
{
    auto & tex = TEXS[material];
    auto faceindex = (side == Side::Top) ? 2 : (side == Side::Bottom) ? 1 : 0;
    auto & face = tex[faceindex];

    float uvoffsetx = static_cast<float>(face.first) * texSlotWidth;
    float uvoffsety = 1.0f - (static_cast<float>(face.second) * texSlotWidth);

    static glm::vec2 texOffsets[] = {
        glm::vec2(onePixel, -onePixel),
        glm::vec2(onePixel + textureWidth, -onePixel),
        glm::vec2(onePixel + textureWidth, -(onePixel + textureWidth)),
        glm::vec2(onePixel, -(onePixel + textureWidth)),
    };
    //If the material's transparent, add these verts to the "t____" parts of the mesh. If not, add them to the normal mesh.
    if (std::ranges::find(transparents, material) != transparents.end())
    {
        std::ranges::transform(cubefaces[static_cast<int>(side)], std::back_inserter(mesh.tpositions), [&offset, &sideHeight](const auto& v) {
            auto newv = v;
            newv.y *= sideHeight;
            return newv + offset;
        });

        std::ranges::transform(cwindices, std::back_inserter(mesh.tindices), [&tindex](const auto& i) {
            return tindex + i;
        });

        mesh.ttexcoords.insert(mesh.ttexcoords.end(),{
                glm::vec2(uvoffsetx + texOffsets[0].x, uvoffsety + texOffsets[0].y),
            glm::vec2(uvoffsetx + texOffsets[1].x, uvoffsety + texOffsets[1].y),
            glm::vec2(uvoffsetx + texOffsets[2].x, uvoffsety + texOffsets[2].y),
            glm::vec2(uvoffsetx + texOffsets[3].x, uvoffsety + texOffsets[3].y),
            });

        if constexpr(doBrightness)
        {
            float isGrass = material == GRASS ? 1.0f : 0.0f;

            switch(side) {
            case Side::Top:    mesh.tbrightness.insert(mesh.tbrightness.end(), {1.0f, isGrass, 1.0f, isGrass, 1.0f, isGrass, 1.0f, isGrass}); break;
            case Side::Left:   mesh.tbrightness.insert(mesh.tbrightness.end(), {0.7f, isGrass, 0.7f, isGrass, 0.7f, isGrass, 0.7f, isGrass}); break;
            case Side::Bottom: mesh.tbrightness.insert(mesh.tbrightness.end(), {0.4f, isGrass, 0.4f, isGrass, 0.4f, isGrass, 0.4f, isGrass}); break;
            case Side::Right:  mesh.tbrightness.insert(mesh.tbrightness.end(), {0.8f, isGrass, 0.8f, isGrass, 0.8f, isGrass, 0.8f, isGrass}); break;
            default:          mesh.tbrightness.insert(mesh.tbrightness.end(), {0.9f, isGrass, 0.9f, isGrass, 0.9f, isGrass, 0.9f, isGrass});
            }
        }


        tindex += 4;
    } else
    {
        std::ranges::transform(cubefaces[static_cast<int>(side)], std::back_inserter(mesh.positions), [&offset, &sideHeight](const auto& v) {
            auto newv = v;
            newv.y *= sideHeight;
            return newv + offset;
        });

        std::ranges::transform(cwindices, std::back_inserter(mesh.indices), [&index](const auto& i) {
            return index + i;
        });

        mesh.texcoords.insert(mesh.texcoords.end(),
{glm::vec2(uvoffsetx + texOffsets[0].x, uvoffsety + texOffsets[0].y),
glm::vec2(uvoffsetx + texOffsets[1].x, uvoffsety + texOffsets[1].y),
glm::vec2(uvoffsetx + texOffsets[2].x, uvoffsety + texOffsets[2].y),
glm::vec2(uvoffsetx + texOffsets[3].x, uvoffsety + texOffsets[3].y),});

        if constexpr(doBrightness)
        {
            float isGrass = material == GRASS ? 1.0f : 0.0f;

            switch(side) {
            case Side::Top:    mesh.brightness.insert(mesh.brightness.end(), {1.0f, isGrass, 1.0f, isGrass, 1.0f, isGrass, 1.0f, isGrass}); break;
            case Side::Left:   mesh.brightness.insert(mesh.brightness.end(), {0.7f, isGrass, 0.7f, isGrass, 0.7f, isGrass, 0.7f, isGrass}); break;
            case Side::Bottom: mesh.brightness.insert(mesh.brightness.end(), {0.4f, isGrass, 0.4f, isGrass, 0.4f, isGrass, 0.4f, isGrass}); break;
            case Side::Right:  mesh.brightness.insert(mesh.brightness.end(), {0.8f, isGrass, 0.8f, isGrass, 0.8f, isGrass, 0.8f, isGrass}); break;
            default:          mesh.brightness.insert(mesh.brightness.end(), {0.9f, isGrass, 0.9f, isGrass, 0.9f, isGrass, 0.9f, isGrass});
            }
        }
        index += 4;
    }



}



UsableMesh fromChunk(const TwoIntTup& spot, World* world, int chunkSize, bool locked);
UsableMesh fromChunk(const TwoIntTup& spot, World* world, int chunkSize)
{
    return fromChunk(spot, world, chunkSize, false);
}
UsableMesh fromChunkLocked(const TwoIntTup& spot, World* world, int chunkSize)
{
    return fromChunk(spot, world, chunkSize, true);
}


///Create a UsableMesh from the specified chunk spot
///This gets called in the mesh building coroutine
UsableMesh fromChunk(const TwoIntTup& spot, World* world, int chunkSize, bool locked)
{
    UsableMesh mesh;
    PxU32 index = 0;
    PxU32 tindex = 0;

    IntTup start(spot.x * chunkSize, spot.z * chunkSize);

    for (int x = 0; x < chunkSize; x++)
    {
        for (int z = 0; z < chunkSize; z++)
        {
            for (int y = 0; y < 250; y++)
            {
                IntTup here = start + IntTup(x, y, z);
                BlockType blockHere = 0;

                switch (locked)
                {
                case true:
                    blockHere = world->getLocked(here);
                    break;
                case false:
                    blockHere = world->get(here);
                }

                if (blockHere != AIR)
                {
                    bool blockHereTransparent = std::ranges::find(transparents, blockHere) != transparents.end();
                    for (int i = 0; i < std::size(neighborSpots); i++)
                    {
                        auto neigh = neighborSpots[i];

                        BlockType neighblock = 0;
                        switch (locked)
                        {
                        case true:
                            neighblock = world->getLocked(neigh + here);
                            break;
                        case false:
                            neighblock = world->get(neigh + here);
                        }

                        auto neightransparent = std::ranges::find(transparents, neighblock) != transparents.end();
                        auto neighborair = neighblock == AIR;

                        if (neighborair || (neightransparent && !blockHereTransparent))
                        {
                            Side side = static_cast<Side>(i);

                            if (!ambOccl) {
                                // Use the original addFace with built-in brightness calculations
                                addFace<true>(PxVec3(static_cast<float>(here.x), static_cast<float>(here.y), static_cast<float>(here.z)),
                                              side, static_cast<MaterialName>(blockHere), 1, mesh, index, tindex);
                            } else {
                                // Use addFace without brightness calculations
                                //std::cout << "Amboc \n";
                                addFace<false>(PxVec3(static_cast<float>(here.x), static_cast<float>(here.y), static_cast<float>(here.z)),
                                               side, static_cast<MaterialName>(blockHere), 1, mesh, index, tindex);

                                // Calculate and add our own ambient occlusion brightness values
                                calculateAmbientOcclusion(here, side, world, locked, blockHere, mesh);
                            }
                        }
                    }
                }
            }
        }
    }

    return mesh;
}

// This function calculates and adds ambient occlusion brightness values for a face
void calculateAmbientOcclusion(const IntTup& blockPos, Side side, World* world, bool locked, BlockType blockType, UsableMesh& mesh)
{
    float baseBrightness;
    switch(side) {
        case Side::Top:    baseBrightness = 1.0f; break;
        case Side::Left:   baseBrightness = 0.7f; break;
        case Side::Bottom: baseBrightness = 0.4f; break;
        case Side::Right:  baseBrightness = 0.8f; break;
        default:           baseBrightness = 0.9f; break;
    }

    float isGrass = blockType == GRASS ? 1.0f : 0.0f;

    // Calculate occlusion for each vertex of the face
    float occlusion[4];
    for (int v = 0; v < 4; v++) {
        // Get the 3 adjacent blocks for this vertex
        std::array<IntTup, 3> adjacentOffsets = getAdjacentOffsets(side, v);

        // Count solid adjacent blocks
        int solidCount = 0;
        for (const auto& offset : adjacentOffsets) {
            IntTup adjPos = blockPos + offset;
            BlockType adjBlock = locked ? world->getLocked(adjPos) : world->get(adjPos);
            if (adjBlock != AIR && std::ranges::find(transparents, adjBlock) == transparents.end()) {
                solidCount++;
            }
        }

        // Apply occlusion based on how many adjacent blocks are solid
        float occlusionValue = 0.0f;
        switch (solidCount) {
            case 1: occlusionValue = -0.3f; break;
            case 2: occlusionValue = -0.5f; break;
            case 3: occlusionValue = -0.9f; break;
            default: occlusionValue = 0.0f; break;
        }

        // Clamp final brightness to valid range
        occlusion[v] = std::max(0.0f, baseBrightness + occlusionValue);
    }

    // Add brightness data to the appropriate arrays based on transparency
    bool isTransparent = std::ranges::find(transparents, blockType) != transparents.end();
    if (isTransparent) {
        mesh.tbrightness.insert(mesh.tbrightness.end(), {
            occlusion[0], isGrass, occlusion[1], isGrass,
            occlusion[2], isGrass, occlusion[3], isGrass
        });
    } else {
        mesh.brightness.insert(mesh.brightness.end(), {
            occlusion[0], isGrass, occlusion[1], isGrass,
            occlusion[2], isGrass, occlusion[3], isGrass
        });
    }
}

// This function returns the 3 adjacent block offsets for a specific vertex of a face
std::array<IntTup, 3> getAdjacentOffsets(Side side, int vertexIndex)
{
    // Map of adjacent block checks for each vertex of each face
    // Format: [side][vertexIndex][3 adjacent blocks]
    static const std::array<std::array<std::array<IntTup, 3>, 4>, 6> adjacentOffsets = {{
        // Front face (0, 0, 0) -> (1, 1, 0)
        {{
            {IntTup(-1, 0, -1), IntTup(0, -1, -1), IntTup(-1, -1, -1)}, // bottom-left vertex
            {IntTup(1, 0, -1), IntTup(0, -1, -1), IntTup(1, -1, -1)},   // bottom-right vertex
            {IntTup(1, 0, -1), IntTup(0, 1, -1), IntTup(1, 1, -1)},     // top-right vertex
            {IntTup(-1, 0, -1), IntTup(0, 1, -1), IntTup(-1, 1, -1)}    // top-left vertex
        }},
        // Right face (1, 0, 0) -> (1, 1, 1)
        {{
            {IntTup(1, 0, -1), IntTup(1, -1, 0), IntTup(1, -1, -1)},    // bottom-front vertex
            {IntTup(1, 0, 1), IntTup(1, -1, 0), IntTup(1, -1, 1)},      // bottom-back vertex
            {IntTup(1, 0, 1), IntTup(1, 1, 0), IntTup(1, 1, 1)},        // top-back vertex
            {IntTup(1, 0, -1), IntTup(1, 1, 0), IntTup(1, 1, -1)}       // top-front vertex
        }},
        // Back face (1, 0, 1) -> (0, 1, 1)
        {{
            {IntTup(1, 0, 1), IntTup(0, -1, 1), IntTup(1, -1, 1)},      // bottom-right vertex
            {IntTup(-1, 0, 1), IntTup(0, -1, 1), IntTup(-1, -1, 1)},    // bottom-left vertex
            {IntTup(-1, 0, 1), IntTup(0, 1, 1), IntTup(-1, 1, 1)},      // top-left vertex
            {IntTup(1, 0, 1), IntTup(0, 1, 1), IntTup(1, 1, 1)}         // top-right vertex
        }},
        // Left face (0, 0, 1) -> (0, 1, 0)
        {{
            {IntTup(-1, 0, 1), IntTup(-1, -1, 0), IntTup(-1, -1, 1)},   // bottom-back vertex
            {IntTup(-1, 0, -1), IntTup(-1, -1, 0), IntTup(-1, -1, -1)}, // bottom-front vertex
            {IntTup(-1, 0, -1), IntTup(-1, 1, 0), IntTup(-1, 1, -1)},   // top-front vertex
            {IntTup(-1, 0, 1), IntTup(-1, 1, 0), IntTup(-1, 1, 1)}      // top-back vertex
        }},
        // Top face (0, 1, 0) -> (1, 1, 1)
        {{
            {IntTup(-1, 1, 0), IntTup(0, 1, -1), IntTup(-1, 1, -1)},    // front-left vertex
            {IntTup(1, 1, 0), IntTup(0, 1, -1), IntTup(1, 1, -1)},      // front-right vertex
            {IntTup(1, 1, 0), IntTup(0, 1, 1), IntTup(1, 1, 1)},        // back-right vertex
            {IntTup(-1, 1, 0), IntTup(0, 1, 1), IntTup(-1, 1, 1)}       // back-left vertex
        }},
        // Bottom face (0, 0, 1) -> (1, 0, 0)
        {{
            {IntTup(-1, -1, 1), IntTup(0, -1, 1), IntTup(-1, -1, 1)},   // back-left vertex
            {IntTup(1, -1, 1), IntTup(0, -1, 1), IntTup(1, -1, 1)},     // back-right vertex
            {IntTup(1, -1, 0), IntTup(0, -1, -1), IntTup(1, -1, -1)},   // front-right vertex
            {IntTup(-1, -1, 0), IntTup(0, -1, -1), IntTup(-1, -1, -1)}  // front-left vertex
        }}
    }};

    return adjacentOffsets[static_cast<int>(side)][vertexIndex];
}