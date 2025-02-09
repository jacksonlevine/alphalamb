//
// Created by jack on 1/27/2025.
//

#include "WorldRenderer.h"

#include "MaterialName.h"
#include "worldgenmethods/OverworldWorldGenMethod.h"
std::atomic<int> NUM_THREADS_RUNNING = 0;
///Prepare vbos and DrawInstructions info (vao, number of indices) so that it can be then drawn with drawFromDrawInstructions()
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
    glBufferData(GL_ARRAY_BUFFER, std::size(usable_mesh.positions) * sizeof(PxVec3), usable_mesh.positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PxVec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, bvbo);
    glBufferData(GL_ARRAY_BUFFER, std::size(usable_mesh.brightness) * sizeof(float), usable_mesh.brightness.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float)*2, (void*)0);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float)*2, (void*)(1*sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, uvvbo);
    glBufferData(GL_ARRAY_BUFFER, std::size(usable_mesh.texcoords) * sizeof(glm::vec2), usable_mesh.texcoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, std::size(usable_mesh.indices) * sizeof(PxU32), usable_mesh.indices.data(), GL_STATIC_DRAW);

    drawInstructions.indiceCount = std::size(usable_mesh.indices);

    glBindVertexArray(drawInstructions.tvao);

    glBindBuffer(GL_ARRAY_BUFFER, tvvbo);
    glBufferData(GL_ARRAY_BUFFER, std::size(usable_mesh.tpositions) * sizeof(PxVec3), usable_mesh.tpositions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PxVec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, tbvbo);
    glBufferData(GL_ARRAY_BUFFER, std::size(usable_mesh.tbrightness) * sizeof(float), usable_mesh.tbrightness.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float)*2, (void*)0);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float)*2, (void*)(1*sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, tuvvbo);
    glBufferData(GL_ARRAY_BUFFER, std::size(usable_mesh.ttexcoords) * sizeof(glm::vec2), usable_mesh.ttexcoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, std::size(usable_mesh.tindices) * sizeof(PxU32), usable_mesh.tindices.data(), GL_STATIC_DRAW);

    drawInstructions.tindiceCount = std::size(usable_mesh.tindices);
}

///This assumes shader is set up and uniforms are given values
void drawFromDrawInstructions(const DrawInstructions& drawInstructions)
{
    glBindVertexArray(drawInstructions.vao);
    glDrawElements(GL_TRIANGLES, drawInstructions.indiceCount, GL_UNSIGNED_INT, 0);
}

void drawTransparentsFromDrawInstructions(const DrawInstructions& drawInstructions)
{
    glBindVertexArray(drawInstructions.tvao);
    glDrawElements(GL_TRIANGLES, drawInstructions.tindiceCount, GL_UNSIGNED_INT, 0);
}

bool isChunkInFrustum(const TwoIntTup& chunkSpot, const glm::vec3& cameraPosition, const glm::vec3& cameraDirection) {

    glm::vec3 chunkBottomPos = glm::vec3(
        chunkSpot.x * WorldRenderer::chunkSize + WorldRenderer::chunkSize / 2.0f,
        cameraPosition.y + cameraDirection.y * 125,
        chunkSpot.z * WorldRenderer::chunkSize + WorldRenderer::chunkSize / 2.0f
    );

    glm::vec3 toChunk = chunkBottomPos - cameraPosition;

    glm::vec3 normalizedToChunk = glm::normalize(toChunk);
    glm::vec3 normalizedDirection = glm::normalize(cameraDirection);


    float dotProduct1 = glm::dot(normalizedToChunk, normalizedDirection);
    dotProduct1 = glm::clamp(dotProduct1, -1.0f, 1.0f);
    float angle1 = glm::degrees(std::acos(dotProduct1));


    const float FOV_ANGLE = 65.0f; // Half of your camera's total FOV

    return angle1 <= FOV_ANGLE;
}

//MAIN THREAD COROUTINE
void WorldRenderer::mainThreadDraw(jl::Camera* playerCamera, GLuint shader, WorldGenMethod* worldGenMethod, float deltaTime)
{
    for(size_t i = 0; i < changeBuffers.size(); i++) {
        auto& buffer = changeBuffers[i];
        //std::cout << "Buffer state: Ready: " << buffer.ready << " in use: " << buffer.in_use << std::endl;
        if(buffer.ready && !buffer.in_use) {
            //std::cout << "Mesh buffer came through on main thread: " << buffer.to.x << " " << buffer.to.z << std::endl;
            modifyOrInitializeDrawInstructions(chunkPool[buffer.chunkIndex].vvbo, chunkPool[buffer.chunkIndex].uvvbo,
            chunkPool[buffer.chunkIndex].ebo, chunkPool[buffer.chunkIndex].drawInstructions, buffer.mesh,
            chunkPool[buffer.chunkIndex].bvbo,
            chunkPool[buffer.chunkIndex].tvvbo, chunkPool[buffer.chunkIndex].tuvvbo, chunkPool[buffer.chunkIndex].tebo, chunkPool[buffer.chunkIndex].tbvbo);

            if (buffer.from == std::nullopt)
            {
                activeChunks.insert_or_assign(buffer.to, ReadyToDrawChunkInfo(buffer.chunkIndex));
            } else
            {
                activeChunks.erase(buffer.from.value());
                activeChunks.insert_or_assign(buffer.to, ReadyToDrawChunkInfo(buffer.chunkIndex));
            }
            confirmedActiveChunksQueue.push(buffer.to);
            buffer.ready = false;
            freedChangeBuffers.push(i);  // Return to free list

            break; //Only do one per frame
        }
    }


    for(size_t i = 0; i < userChangeMeshBuffers.size(); i++) {
        auto& buffer = userChangeMeshBuffers[i];
        //std::cout << "User Buffer state: Ready: " << buffer.ready << " in use: " << buffer.in_use << std::endl;
        if(buffer.ready && !buffer.in_use) {
            //std::cout << "Buffer came through on main thread: " << buffer.to.x << " " << buffer.to.z << std::endl;
            modifyOrInitializeDrawInstructions(chunkPool[buffer.chunkIndex].vvbo, chunkPool[buffer.chunkIndex].uvvbo,
            chunkPool[buffer.chunkIndex].ebo, chunkPool[buffer.chunkIndex].drawInstructions, buffer.mesh,
            chunkPool[buffer.chunkIndex].bvbo,
            chunkPool[buffer.chunkIndex].tvvbo, chunkPool[buffer.chunkIndex].tuvvbo, chunkPool[buffer.chunkIndex].tebo, chunkPool[buffer.chunkIndex].tbvbo);

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
            buffer.ready = false;
            freedUserChangeMeshBuffers.push(i);  // Return to free list

            break; //Only do one per frame
        }
    }
    TwoIntTup playerChunkPosition = worldToChunkPos(
        TwoIntTup(std::floor(playerCamera->transform.position.x),
            std::floor(playerCamera->transform.position.z)));

    static GLuint grassRedChangeLoc = glGetUniformLocation(shader, "grassRedChange");
    static GLuint timeRenderedLoc = glGetUniformLocation(shader, "timeRendered");

    for (auto& [spot, chunkinfo] : activeChunks) {
        bool isDrawingThis = true;

        if (isChunkInFrustum(spot, playerCamera->transform.position - (playerCamera->transform.direction * (float)chunkSize), playerCamera->transform.direction)) {
            int dist = abs(spot.x - playerChunkPosition.x) + abs(spot.z - playerChunkPosition.z);
            if (dist <= MIN_DISTANCE) {
                auto glInfo = chunkPool[chunkinfo.chunkIndex];
                IntTup chunkRealSpot = IntTup(spot.x * chunkSize, spot.z * chunkSize);
                float grassRedChange = (worldGenMethod->getTemperatureNoise(chunkRealSpot) - worldGenMethod->getHumidityNoise(chunkRealSpot));
                glUniform1f(grassRedChangeLoc, grassRedChange);
                glUniform1f(timeRenderedLoc, chunkinfo.timeBeenRendered);
                drawFromDrawInstructions(glInfo.drawInstructions);
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
        if (isChunkInFrustum(spot, playerCamera->transform.position - (playerCamera->transform.direction * (float)chunkSize), playerCamera->transform.direction)) {
            int dist = abs(spot.x - playerChunkPosition.x) + abs(spot.z - playerChunkPosition.z);
            if (dist <= MIN_DISTANCE) {
                auto glInfo = chunkPool[chunkinfo.chunkIndex];
                IntTup chunkRealSpot = IntTup(spot.x * chunkSize, spot.z * chunkSize);
                float grassRedChange = (worldGenMethod->getTemperatureNoise(chunkRealSpot) - worldGenMethod->getHumidityNoise(chunkRealSpot));
                glUniform1f(grassRedChangeLoc, grassRedChange);
                glUniform1f(timeRenderedLoc, chunkinfo.timeBeenRendered);
                drawTransparentsFromDrawInstructions(glInfo.drawInstructions);
            }
        }
    }
}



//SECOND THREAD MESH BUILDING COROUTINE
void WorldRenderer::meshBuildCoroutine(jl::Camera* playerCamera, World* world)
{

    std::cout << "Mesh thread started!\n";
    NUM_THREADS_RUNNING.fetch_add(1);  // Atomic increment
    std::cout << "Mesh incremented NUM_THREADS_RUNNING. Current value: " << NUM_THREADS_RUNNING.load() << "\n";

    for(size_t i = 0; i < changeBuffers.size(); i++) {
        freedChangeBuffers.push(i);
    }
    for(size_t i = 0; i < userChangeMeshBuffers.size(); i++) {
        freedUserChangeMeshBuffers.push(i);
    }
    if (chunkPool.capacity() < maxChunks)
    {
        chunkPool.reserve(maxChunks);
        mbtActiveChunks.reserve(maxChunks);
        activeChunks.reserve(maxChunks);
    }


    while(meshBuildingThreadRunning)
    {




        TwoIntTup playerChunkPosition = worldToChunkPos(
        TwoIntTup(std::floor(playerCamera->transform.position.x),
            std::floor(playerCamera->transform.position.z)));

        static std::array<TwoIntTup, (renderDistance*2)*(renderDistance*2)> checkspots = {};

        int index = 0;
        for (int i = -renderDistance; i < renderDistance; i++)
        {
            for (int j = -renderDistance; j < renderDistance; j++)
            {
                checkspots[index] = playerChunkPosition + TwoIntTup(i,j);
                index+=1;
            }
        }



        std::sort(checkspots.begin(), checkspots.end(), [&playerChunkPosition](const TwoIntTup& a, const TwoIntTup& b)
        {
            int distfroma = abs(a.x - playerChunkPosition.x) + abs(a.z - playerChunkPosition.z);
            int distfromb = abs(b.x - playerChunkPosition.x) + abs(b.z - playerChunkPosition.z);
            return distfroma < distfromb;
        });


        for (auto & spotHere : checkspots)
        {

                TwoIntTup confirmedChunk;
                while (confirmedActiveChunksQueue.pop(confirmedChunk) && meshBuildingThreadRunning) {
                    if (mbtActiveChunks.contains(confirmedChunk))
                    {
                        mbtActiveChunks.at(confirmedChunk).confirmedByMainThread = true;
                    }


                }

                    int dist = abs(spotHere.x - playerChunkPosition.x) + abs(spotHere.z - playerChunkPosition.z);
                    if(dist <= MIN_DISTANCE)
                    {
                        if (!mbtActiveChunks.contains(spotHere))
                    {
                            if(!generatedChunks.contains(spotHere))
                            {
                                generateChunk(world, spotHere);
                            }
                        //std::cout << "Spot " << i << " " << j << std::endl;

                        //IF we havent reached the max chunks yet, we can just make a new one for this spot.
                        //ELSE, we reuse the furthest one from the player, ONLY IF the new distance will be shorter than the last distance!
                        if (chunkPool.size() < maxChunks)
                        {
                            size_t changeBufferIndex = -1;
                            while (!freedChangeBuffers.pop(changeBufferIndex) && meshBuildingThreadRunning) {
                                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                                std::cout << "Trying to pop change buffer on mesh \n";
                            }
                            if (changeBufferIndex != -1)
                            {
                                //Add the mesh, in full form, to our reserved Change Buffer (The main thread coroutine will make GL calls and free this slot to be reused)

                                auto& buffer = changeBuffers[changeBufferIndex];
                                buffer.in_use = true;
                                buffer.mesh = fromChunk(spotHere, world, chunkSize);

                                buffer.chunkIndex = addUninitializedChunkBuffer();
                                buffer.from = std::nullopt;
                                buffer.to = spotHere;

                                mbtActiveChunks.insert_or_assign(spotHere, UsedChunkInfo(buffer.chunkIndex));

                                buffer.ready = true;   // Signal that data is ready
                                buffer.in_use = false;
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
                                if (distance > MIN_DISTANCE) {
                                    chunksWithDistances.emplace_back(distance, chunkPos);
                                }

                            }



                            //If any still matching the criteria
                            if(!chunksWithDistances.empty())
                            {
                                // Sort chunks by distance
                                std::sort(chunksWithDistances.begin(), chunksWithDistances.end(),
                                          [](const std::pair<float, TwoIntTup>& a, const std::pair<float, TwoIntTup>& b) {
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
                                        while (!freedChangeBuffers.pop(changeBufferIndex) && meshBuildingThreadRunning) {
                                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                                        }
                                        if (changeBufferIndex != -1)
                                        {
                                            //Add the mesh, in full form, to our reserved Change Buffer (The main thread coroutine will make GL calls and free this slot to be reused)

                                            auto& buffer = changeBuffers[changeBufferIndex];
                                            buffer.in_use = true;
                                            buffer.mesh = fromChunk(spotHere, world, chunkSize);

                                            buffer.chunkIndex = chunkIndex;
                                            buffer.from = oldSpot;
                                            buffer.to = spotHere;



                                            mbtActiveChunks.erase(oldSpot);
                                            mbtActiveChunks.insert_or_assign(spotHere, UsedChunkInfo(chunkIndex));

                                            buffer.ready = true;   // Signal that data is ready
                                            buffer.in_use = false;
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

void WorldRenderer::generateChunk(World* world, TwoIntTup& chunkSpot)
{

    srand(chunkSpot.x * 73856093 ^ chunkSpot.z * 19349663);
    if (rand() > 9000)
    {
        int y = 50;
        float offsetNoise = world->worldGenMethod->getHumidityNoise(IntTup(chunkSpot.x, chunkSpot.z)) * 5.0;
        float offsetNoise2 = world->worldGenMethod->getHumidityNoise(IntTup(chunkSpot.x, chunkSpot.z)) * 5.0;
        IntTup realSpot(chunkSpot.x * chunkSize + (chunkSize >> 1) + offsetNoise , chunkSpot.z * chunkSize  + (chunkSize >> 1) + offsetNoise2);
        bool surfaceBlockFound = false;
        while(y < 170 && !surfaceBlockFound)
        {
            realSpot.y = y;
            if(world->get(realSpot) == GRASS)
            {
                surfaceBlockFound = true;
            }
            y++;
        }
        if(surfaceBlockFound)
        {
            Climate climate = world->worldGenMethod->getClimate(realSpot);
            std::vector<TerrainFeature>& possibleFeatures = getTerrainFeaturesFromClimate(climate);
            size_t selectedIndex = (int)(((float)rand()/(float)RAND_MAX) * (float)(possibleFeatures.size()-1));
            TerrainFeature selectedFeature = possibleFeatures[selectedIndex];

            std::vector<VoxelModelName>& possibleModels = getVoxelModelNamesFromTerrainFeatureName(selectedFeature);
            size_t selectedModelIndex = (int)(((float)rand()/(float)RAND_MAX) * (float)(possibleModels.size()-1));
            VoxelModelName selectedModel = possibleModels[selectedModelIndex];

            IntTup& voxDim = voxelModels[selectedModel].dimensions;
            IntTup offset = IntTup(-(voxDim.x / 2), 0, -(voxDim.z / 2));

            for(auto & point : voxelModels[selectedModel].points)
            {
                if(point.colorIndex != AIR)
                {
                    world->nonUserDataMap->set(realSpot + point.localSpot + offset, point.colorIndex);
                }
            }
        }
    }

    generatedChunks.insert(chunkSpot);
}

///Call this with an external index and UsableMesh to mutate them
__inline void addFace(PxVec3 offset, Side side, MaterialName material, int sideHeight, UsableMesh& mesh, PxU32& index, PxU32& tindex)
{
    auto & tex = TEXS[material];
    auto faceindex = (side == Top) ? 2 : (side == Bottom) ? 1 : 0;
    auto & face = tex[faceindex];

    float uvoffsetx = (float)face.first * texSlotWidth;
    float uvoffsety = 1.0f - ((float)face.second * texSlotWidth);

    static glm::vec2 texOffsets[] = {
        glm::vec2(onePixel, -onePixel),
        glm::vec2(onePixel + textureWidth, -onePixel),
        glm::vec2(onePixel + textureWidth, -(onePixel + textureWidth)),
        glm::vec2(onePixel, -(onePixel + textureWidth)),
    };
    //If the material's transparent, add these verts to the "t____" parts of the mesh. If not, add them to the normal mesh.
    if (std::find(transparents.begin(), transparents.end(), material) != transparents.end())
    {
        std::ranges::transform(cubefaces[side], std::back_inserter(mesh.tpositions), [&offset, &sideHeight](const auto& v) {
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

        float isGrass = material == GRASS ? 1.0f : 0.0f;

        switch(side) {
        case Side::Top:    mesh.tbrightness.insert(mesh.tbrightness.end(), {1.0f, isGrass, 1.0f, isGrass, 1.0f, isGrass, 1.0f, isGrass}); break;
        case Side::Left:   mesh.tbrightness.insert(mesh.tbrightness.end(), {0.7f, isGrass, 0.7f, isGrass, 0.7f, isGrass, 0.7f, isGrass}); break;
        case Side::Bottom: mesh.tbrightness.insert(mesh.tbrightness.end(), {0.4f, isGrass, 0.4f, isGrass, 0.4f, isGrass, 0.4f, isGrass}); break;
        case Side::Right:  mesh.tbrightness.insert(mesh.tbrightness.end(), {0.8f, isGrass, 0.8f, isGrass, 0.8f, isGrass, 0.8f, isGrass}); break;
        default:          mesh.tbrightness.insert(mesh.tbrightness.end(), {0.9f, isGrass, 0.9f, isGrass, 0.9f, isGrass, 0.9f, isGrass});
        }

        tindex += 4;
    } else
    {
        std::ranges::transform(cubefaces[side], std::back_inserter(mesh.positions), [&offset, &sideHeight](const auto& v) {
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

        float isGrass = material == GRASS ? 1.0f : 0.0f;

        switch(side) {
        case Side::Top:    mesh.brightness.insert(mesh.brightness.end(), {1.0f, isGrass, 1.0f, isGrass, 1.0f, isGrass, 1.0f, isGrass}); break;
        case Side::Left:   mesh.brightness.insert(mesh.brightness.end(), {0.7f, isGrass, 0.7f, isGrass, 0.7f, isGrass, 0.7f, isGrass}); break;
        case Side::Bottom: mesh.brightness.insert(mesh.brightness.end(), {0.4f, isGrass, 0.4f, isGrass, 0.4f, isGrass, 0.4f, isGrass}); break;
        case Side::Right:  mesh.brightness.insert(mesh.brightness.end(), {0.8f, isGrass, 0.8f, isGrass, 0.8f, isGrass, 0.8f, isGrass}); break;
        default:          mesh.brightness.insert(mesh.brightness.end(), {0.9f, isGrass, 0.9f, isGrass, 0.9f, isGrass, 0.9f, isGrass});
        }
        index += 4;
    }



}
UsableMesh _fromChunk(TwoIntTup spot, World* world, int chunkSize, bool locked);
UsableMesh fromChunk(TwoIntTup spot, World* world, int chunkSize)
{
    return _fromChunk(spot, world, chunkSize, false);
}
UsableMesh fromChunkLocked(TwoIntTup spot, World* world, int chunkSize)
{
    return _fromChunk(spot, world, chunkSize, true);
}
///Create a UsableMesh from the specified chunk spot
///This gets called in the mesh building coroutine
UsableMesh _fromChunk(TwoIntTup spot, World* world, int chunkSize, bool locked)
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
                uint32_t blockHere = 0;

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
                    bool blockHereTransparent = std::find(transparents.begin(), transparents.end(), blockHere) != transparents.end();
                    for (int i = 0; i < std::size(neighborSpots); i++)
                    {
                        auto neigh = neighborSpots[i];

                        uint32_t neighblock = 0;
                        switch (locked)
                        {
                        case true:
                            neighblock = world->getLocked(neigh + here);
                            break;
                        case false:
                            neighblock = world->get(neigh + here);
                        }

                        auto neightransparent = std::find(transparents.begin(), transparents.end(), neighblock) != transparents.end();
                        auto neighborair = neighblock == AIR;

                        if (neighborair || (neightransparent && !blockHereTransparent))
                        {
                            addFace(PxVec3(here.x, here.y, here.z), (Side)i, (MaterialName)blockHere, 1, mesh, index, tindex);
                        }
                    }
                }
            }
        }
    }

    return mesh;
}