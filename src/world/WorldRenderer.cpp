//
// Created by jack on 1/27/2025.
//

#include "WorldRenderer.h"

#include "MaterialName.h"

///Prepare vbos and DrawInstructions info (vao, number of indices) so that it can be then drawn with drawFromDrawInstructions()
void modifyOrInitializeDrawInstructions(GLuint& vvbo, GLuint& uvvbo, GLuint& ebo, DrawInstructions& drawInstructions, UsableMesh& usable_mesh, GLuint& bvbo)
{
    if(drawInstructions.vao == 0)
    {
        glGenVertexArrays(1, &drawInstructions.vao);
        glBindVertexArray(drawInstructions.vao);
        glGenBuffers(1, &vvbo);
        glGenBuffers(1, &uvvbo);
        glGenBuffers(1, &ebo);
        glGenBuffers(1, &bvbo);
    } else
    {
        glBindVertexArray(drawInstructions.vao);
    }

    glBindBuffer(GL_ARRAY_BUFFER, vvbo);
    glBufferData(GL_ARRAY_BUFFER, std::size(usable_mesh.positions) * sizeof(PxVec3), usable_mesh.positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PxVec3), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, bvbo);
    glBufferData(GL_ARRAY_BUFFER, std::size(usable_mesh.brightness) * sizeof(float), usable_mesh.brightness.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, uvvbo);
    glBufferData(GL_ARRAY_BUFFER, std::size(usable_mesh.texcoords) * sizeof(glm::vec2), usable_mesh.texcoords.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, std::size(usable_mesh.indices) * sizeof(PxU32), usable_mesh.indices.data(), GL_STATIC_DRAW);

    drawInstructions.indiceCount = std::size(usable_mesh.indices);
}

///This assumes shader is set up and uniforms are given values
void drawFromDrawInstructions(const DrawInstructions& drawInstructions)
{
    glBindVertexArray(drawInstructions.vao);
    glDrawElements(GL_TRIANGLES, drawInstructions.indiceCount, GL_UNSIGNED_INT, 0);
}


//MAIN THREAD COROUTINE
void WorldRenderer::mainThreadDraw()
{
    for(size_t i = 0; i < changeBuffers.size(); i++) {
        auto& buffer = changeBuffers[i];
        if(buffer.ready && !buffer.in_use) {
            modifyOrInitializeDrawInstructions(chunkPool[buffer.chunkIndex].vvbo, chunkPool[buffer.chunkIndex].uvvbo,
            chunkPool[buffer.chunkIndex].ebo, chunkPool[buffer.chunkIndex].drawInstructions, buffer.mesh,
            chunkPool[buffer.chunkIndex].bvbo);

            if (buffer.from == std::nullopt)
            {
                activeChunks.insert_or_assign(buffer.to, buffer.chunkIndex);

            } else
            {
                activeChunks.erase(buffer.from.value());
                activeChunks.insert_or_assign(buffer.to, buffer.chunkIndex);
            }
            confirmedQueue.push(buffer.to);
            buffer.ready = false;
            freeChangeBuffers.push(i);  // Return to free list
            break; //Only do one per frame
        }
    }
    for (const auto & [spot, index] : activeChunks)
    {
        auto glInfo = chunkPool[index];
        drawFromDrawInstructions(glInfo.drawInstructions);
    }
}



//SECOND THREAD MESH BUILDING COROUTINE
void WorldRenderer::meshBuildCoroutine(jl::Camera* playerCamera, World* world)
{
    for(size_t i = 0; i < changeBuffers.size(); i++) {
        freeChangeBuffers.push(i);
    }
    while(true)
    {

        IntTup confirmedChunk;
        while (confirmedQueue.pop(confirmedChunk)) {
            myActiveChunks.at(confirmedChunk).confirmedByMainThread = true;
        }


        IntTup playerChunkPosition = worldToChunkPos(
        IntTup(std::floor(playerCamera->transform.position.x),
            std::floor(playerCamera->transform.position.z)));
        for (int i = -renderDistance; i < renderDistance; i++)
        {
            for (int j = -renderDistance; j < renderDistance; j++)
            {

                    IntTup spotHere = playerChunkPosition + IntTup(i,j);
                    if (!myActiveChunks.contains(spotHere))
                    {
                        //std::cout << "Spot " << i << " " << j << std::endl;

                        //IF we havent reached the max chunks yet, we can just make a new one for this spot.
                        //ELSE, we reuse the furthest one from the player, ONLY IF the new distance will be shorter than the last distance!
                        if (chunkPool.size() < maxChunks)
                        {
                            size_t changeBufferIndex;
                            while (!freeChangeBuffers.pop(changeBufferIndex)) {
                                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                            }

                            //Add the mesh, in full form, to our reserved Change Buffer (The main thread coroutine will make GL calls and free this slot to be reused)

                            auto& buffer = changeBuffers[changeBufferIndex];
                            buffer.in_use = true;
                            buffer.mesh = fromChunk(spotHere, world, chunkSize);

                            buffer.chunkIndex = addUninitializedChunkBuffer();
                            buffer.from = std::nullopt;
                            buffer.to = spotHere;

                            buffer.ready = true;   // Signal that data is ready
                            buffer.in_use = false;

                            myActiveChunks.insert_or_assign(spotHere, UsedChunkInfo(buffer.chunkIndex));

                        } else
                        {
                            constexpr float MIN_DISTANCE = (float)(renderDistance + 1) * (chunkSize);

                            std::vector<std::pair<float,IntTup>> chunksWithDistances;

                            for (const auto& [chunkPos, usedChunkInfo] : myActiveChunks) {

                                if (!usedChunkInfo.confirmedByMainThread)
                                {
                                    //We only want ones confirmed by main thread, we know we can repurpose those
                                    continue;
                                }
                                // Convert IntTup to world position (assuming Chunk::WIDTH is defined)
                                glm::vec3 worldPosition = glm::vec3(
                                    chunkPos.x * chunkSize,
                                    0, // IntTup Y is 0 for chunkPos
                                    chunkPos.z * chunkSize
                                );

                                // Calculate distance to yourPosition
                                float distance = glm::distance(worldPosition, playerCamera->transform.position);

                                // Filter out chunks closer than MIN_DISTANCE
                                if (distance > MIN_DISTANCE) {
                                    chunksWithDistances.emplace_back(distance, chunkPos);
                                }

                            }



                            //If any still matching the criteria
                            if(chunksWithDistances.size() > 0)
                            {
                                // Sort chunks by distance
                                std::sort(chunksWithDistances.begin(), chunksWithDistances.end(),
                                          [](const std::pair<float, IntTup>& a, const std::pair<float, IntTup>& b) {
                                              return a.first > b.first;
                                });

                                for (const auto& [oldDistance, spot] : chunksWithDistances)
                                {
                                    //If the place we're going will afford a shorter distance from player, choose this one.
                                    glm::vec3 newWorldPosition = glm::vec3(
                                        spotHere.x * chunkSize,
                                        0, // IntTup Y is 0 for chunkPos
                                        spotHere.z * chunkSize
                                    );
                                    float newDistance = glm::distance(newWorldPosition, playerCamera->transform.position);

                                    if (newDistance < oldDistance)
                                    {
                                        size_t chunkIndex = myActiveChunks.at(spot).chunkIndex;

                                        size_t changeBufferIndex;
                                        while (!freeChangeBuffers.pop(changeBufferIndex)) {
                                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                                        }

                                        //Add the mesh, in full form, to our reserved Change Buffer (The main thread coroutine will make GL calls and free this slot to be reused)

                                        auto& buffer = changeBuffers[changeBufferIndex];
                                        buffer.in_use = true;
                                        buffer.mesh = fromChunk(spotHere, world, chunkSize);

                                        buffer.chunkIndex = chunkIndex;
                                        buffer.from = spot;
                                        buffer.to = spotHere;

                                        buffer.ready = true;   // Signal that data is ready
                                        buffer.in_use = false;

                                        myActiveChunks.erase(spot);
                                        myActiveChunks.insert_or_assign(spotHere, UsedChunkInfo(chunkIndex));


                                        break;
                                    }

                                }
                            }



                        }

                    }

            }
        }
    }
}

///Call this with an external index and UsableMesh to mutate them
void addFace(PxVec3 offset, Side side, MaterialName material, int sideHeight, UsableMesh& mesh, PxU32& index)
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

    switch(side) {
    case Side::Top:    mesh.brightness.insert(mesh.brightness.end(), {1.0f, 1.0f, 1.0f, 1.0f}); break;
    case Side::Left:   mesh.brightness.insert(mesh.brightness.end(), {0.7f, 0.7f, 0.7f, 0.7f}); break;
    case Side::Bottom: mesh.brightness.insert(mesh.brightness.end(), {0.4f, 0.4f, 0.4f, 0.4f}); break;
    case Side::Right:  mesh.brightness.insert(mesh.brightness.end(), {0.8f, 0.8f, 0.8f, 0.8f}); break;
    default:          mesh.brightness.insert(mesh.brightness.end(), {0.9f, 0.9f, 0.9f, 0.9f});
    }

    index += 4;
}

///Create a UsableMesh from the specified chunk spot
///This gets called in the mesh building coroutine
UsableMesh fromChunk(IntTup spot, World* world, int chunkSize)
{
    UsableMesh mesh;
    PxU32 index = 0;

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
                    for (int i = 0; i < std::size(neighborSpots); i++)
                    {
                        auto neigh = neighborSpots[i];
                        if (world->get(neigh + here) == AIR)
                        {
                            addFace(PxVec3(here.x, here.y, here.z), (Side)i, (MaterialName)blockHere, 1, mesh, index);
                        }
                    }
                }
            }
        }
    }

    return mesh;
}