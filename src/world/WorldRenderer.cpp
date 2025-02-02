//
// Created by jack on 1/27/2025.
//

#include "WorldRenderer.h"

#include "MaterialName.h"
#include "worldgenmethods/OverworldWorldGenMethod.h"

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
void WorldRenderer::mainThreadDraw(jl::Camera* playerCamera, GLuint shader, WorldGenMethod* worldGenMethod)
{
    for(size_t i = 0; i < changeBuffers.size(); i++) {
        auto& buffer = changeBuffers[i];
        if(buffer.ready && !buffer.in_use) {
            modifyOrInitializeDrawInstructions(chunkPool[buffer.chunkIndex].vvbo, chunkPool[buffer.chunkIndex].uvvbo,
            chunkPool[buffer.chunkIndex].ebo, chunkPool[buffer.chunkIndex].drawInstructions, buffer.mesh,
            chunkPool[buffer.chunkIndex].bvbo,
            chunkPool[buffer.chunkIndex].tvvbo, chunkPool[buffer.chunkIndex].tuvvbo, chunkPool[buffer.chunkIndex].tebo, chunkPool[buffer.chunkIndex].tbvbo);

            if (buffer.from == std::nullopt)
            {
                activeChunks.insert_or_assign(buffer.to, buffer.chunkIndex);

            } else
            {
                activeChunks.erase(buffer.from.value());
                activeChunks.insert_or_assign(buffer.to, buffer.chunkIndex);
            }
            confirmedActiveChunksQueue.push(buffer.to);

            freedChangeBuffers.push(i);  // Return to free list
            buffer.ready = false;
            break; //Only do one per frame
        }
    }
    TwoIntTup playerChunkPosition = worldToChunkPos(
        TwoIntTup(std::floor(playerCamera->transform.position.x),
            std::floor(playerCamera->transform.position.z)));
    for (const auto& [spot, index] : activeChunks) {
        // Check if the chunk is within the frustum
        if (isChunkInFrustum(spot, playerCamera->transform.position - (playerCamera->transform.direction * (float)chunkSize), playerCamera->transform.direction)) {
            int dist = abs(spot.x - playerChunkPosition.x) + abs(spot.z - playerChunkPosition.z);
            if (dist <= MIN_DISTANCE) {
                auto glInfo = chunkPool[index];
                static GLuint grassRedChangeLoc = glGetUniformLocation(shader, "grassRedChange");
                IntTup chunkRealSpot = IntTup(spot.x * chunkSize, spot.z * chunkSize);
                float grassRedChange = (worldGenMethod->getTemperatureNoise(chunkRealSpot) - worldGenMethod->getHumidityNoise(chunkRealSpot));
                glUniform1f(grassRedChangeLoc, grassRedChange);
                drawFromDrawInstructions(glInfo.drawInstructions);
            }
        }
    }
}



//SECOND THREAD MESH BUILDING COROUTINE
void WorldRenderer::meshBuildCoroutine(jl::Camera* playerCamera, World* world)
{


    for(size_t i = 0; i < changeBuffers.size(); i++) {
        freedChangeBuffers.push(i);
    }
    chunkPool.reserve(maxChunks);
    mbtActiveChunks.reserve(maxChunks);
    activeChunks.reserve(maxChunks);

    while(true)
    {

        TwoIntTup confirmedChunk;
        while (confirmedActiveChunksQueue.pop(confirmedChunk)) {
            mbtActiveChunks.at(confirmedChunk).confirmedByMainThread = true;
        }


        TwoIntTup playerChunkPosition = worldToChunkPos(
        TwoIntTup(std::floor(playerCamera->transform.position.x),
            std::floor(playerCamera->transform.position.z)));

        for (int i = -renderDistance; i < renderDistance; i++)
        {
            for (int j = -renderDistance; j < renderDistance; j++)
            {


                    TwoIntTup spotHere = playerChunkPosition + TwoIntTup(i,j);

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
                            size_t changeBufferIndex;
                            while (!freedChangeBuffers.pop(changeBufferIndex)) {
                                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            }

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

                                        size_t changeBufferIndex;
                                        while (!freedChangeBuffers.pop(changeBufferIndex)) {
                                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                                        }

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

                                        break;
                                    }

                                }
                            }



                        }

                    }
                    }


            }
        }
        std::this_thread::sleep_for(std::chrono::seconds(4));


    }




}

void WorldRenderer::generateChunk(World* world, TwoIntTup& chunkSpot)
{

    srand(chunkSpot.x + chunkSpot.z);
    int y = 50;
    IntTup realSpot(chunkSpot.x * chunkSize, chunkSpot.z * chunkSize);
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

        for(auto & point : voxelModels[selectedModel])
        {
            if(point.colorIndex != AIR)
            {
                world->nonUserDataMap->set(realSpot + point.localSpot, point.colorIndex);
            }
        }
    }
    generatedChunks.insert(chunkSpot);
}

///Call this with an external index and UsableMesh to mutate them
__inline void addFace(PxVec3 offset, Side side, MaterialName material, int sideHeight, UsableMesh& mesh, PxU32& index, PxU32& tindex)
{
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

        mesh.ttexcoords.insert(mesh.ttexcoords.end(),
            {glm::vec2(0.0 + ((float)material / 16.0f), 0.0), glm::vec2(0.0625f + ((float)material / 16.0f), 0.0),
             glm::vec2(0.0625f + ((float)material / 16.0f), 1.0 * sideHeight), glm::vec2(0.0 + ((float)material / 16.0f), 1.0 * sideHeight)});

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
            {glm::vec2(0.0 + ((float)material / 16.0f), 0.0), glm::vec2(0.0625f + ((float)material / 16.0f), 0.0),
             glm::vec2(0.0625f + ((float)material / 16.0f), 1.0 * sideHeight), glm::vec2(0.0 + ((float)material / 16.0f), 1.0 * sideHeight)});

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

///Create a UsableMesh from the specified chunk spot
///This gets called in the mesh building coroutine
UsableMesh fromChunk(TwoIntTup spot, World* world, int chunkSize)
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
                uint32_t blockHere = world->get(here);

                if (blockHere != AIR)
                {
                    bool blockHereTransparent = std::find(transparents.begin(), transparents.end(), blockHere) != transparents.end();
                    for (int i = 0; i < std::size(neighborSpots); i++)
                    {
                        auto neigh = neighborSpots[i];
                        auto neighblock = world->get(neigh + here);
                        auto neightransparent = std::find(transparents.begin(), transparents.end(), neighblock) != transparents.end();
                        auto transparentbordering = blockHereTransparent && neighblock != blockHere;

                        if (neightransparent || transparentbordering)
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