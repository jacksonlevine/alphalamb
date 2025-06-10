//
// Created by jack on 1/27/2025.
//

#include "WorldRenderer.h"

#include "MaterialName.h"
#include "../AmbOcclSetting.h"
#include "worldgenmethods/OverworldWorldGenMethod.h"
#include "../IndexOptimization.h"
#include "../specialblocks/FindEntityCreateFunc.h"
#include "../specialblocks/FindSpecialBlock.h"
#include "../Light.tpp"
std::atomic<int> NUM_THREADS_RUNNING = 0;


LightMapType ambientlightmap;
LightMapType lightmap;
std::shared_mutex lightmapMutex = {};
tbb::concurrent_hash_map<TwoIntTup, bool, TwoIntTupHashCompare> litChunks;

#include "WorldRenderer.tpp"
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
    auto size1 = static_cast<GLsizeiptr>(std::size(usable_mesh.positions) * sizeof(PxVec3));
    glBufferData(GL_ARRAY_BUFFER, size1, NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size1, usable_mesh.positions.data());


    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PxVec3), nullptr);
    glEnableVertexAttribArray(0);

    auto size2 = static_cast<GLsizeiptr>(std::size(usable_mesh.brightness) * sizeof(float));
    glBindBuffer(GL_ARRAY_BUFFER, bvbo);
    glBufferData(GL_ARRAY_BUFFER, size2, NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size2, usable_mesh.brightness.data());

    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float)*2, nullptr);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float)*2, static_cast<void*>(static_cast<char*>(nullptr) + 1*sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, uvvbo);
    auto size3 =  static_cast<GLsizeiptr>(std::size(usable_mesh.texcoords) * sizeof(glm::vec2));
    glBufferData(GL_ARRAY_BUFFER, size3, NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0,size3, usable_mesh.texcoords.data());
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    auto size4 = static_cast<GLsizeiptr>(std::size(usable_mesh.indices) * sizeof(PxU32));
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size4, NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, size4, usable_mesh.indices.data());

    info.indiceCount = static_cast<int>(std::size(usable_mesh.indices));

    glBindVertexArray(tvao);

    glBindBuffer(GL_ARRAY_BUFFER, tvvbo);
    auto size5 = static_cast<GLsizeiptr>(std::size(usable_mesh.tpositions) * sizeof(PxVec3));
    glBufferData(GL_ARRAY_BUFFER, size5, NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size5, usable_mesh.tpositions.data());
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PxVec3), nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, tbvbo);
    auto size6 =  static_cast<GLsizeiptr>(std::size(usable_mesh.tbrightness) * sizeof(float));
    glBufferData(GL_ARRAY_BUFFER, size6, NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size6, usable_mesh.tbrightness.data());
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(float)*2, nullptr);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(float)*2, static_cast<void*>(static_cast<char*>(nullptr) + 1*sizeof(float)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, tuvvbo);
    auto size7 = static_cast<GLsizeiptr>(std::size(usable_mesh.ttexcoords) * sizeof(glm::vec2));

    glBufferData(GL_ARRAY_BUFFER, size7, NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, size7, usable_mesh.ttexcoords.data());
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tebo);
    auto size8 = static_cast<GLsizeiptr>(std::size(usable_mesh.tindices) * sizeof(PxU32));
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size8, NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0 , size8, usable_mesh.tindices.data());

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

double WorldRenderer::getDewyFogFactor(float temperature_noise, float humidity_noise)
{
    return glm::min(1.0, glm::max(0.0f, humidity_noise * 2.0f)
                    + /*Additional as it becomes chilly (negative temp)*/
                    (-2.0 * (glm::min(0.0f, temperature_noise))));
}

//MAIN THREAD COROUTINE
void WorldRenderer::mainThreadDraw(const jl::Camera* playerCamera, GLuint shader, WorldGenMethod* worldGenMethod, float deltaTime, bool actuallyDraw)
{
    TwoIntTup playerChunkPosition = worldToChunkPos(
        TwoIntTup(std::floor(playerCamera->transform.position.x),
            std::floor(playerCamera->transform.position.z)));

    for(size_t i = 0; i < changeBuffers.size(); i++) {
        auto& buffer = changeBuffers[i];
        //std::cout << "Buffer state: Ready: " << buffer.ready << " in use: " << buffer.in_use << std::endl;
        if(buffer.ready.load(std::memory_order_acquire)) {
            //std::cout << "Mesh buffer came through on main thread: " << buffer.to.x << " " << buffer.to.z << std::endl;



                    modifyOrInitializeChunkIndex(static_cast<int>(buffer.chunkIndex), chunkPool.at(buffer.chunkIndex), buffer.mesh);
                    bool confirm = true;
                    if (buffer.from == std::nullopt)
                    {
                       if (!activeChunks.contains(buffer.to))
                       {
                            activeChunks.insert_or_assign(buffer.to, ReadyToDrawChunkInfo(buffer.chunkIndex, 0.f));
                       }
                    }
                    else
                    {



                        auto todistfromplayer = std::abs(buffer.to.x - playerChunkPosition.x) + std::abs(buffer.to.z - playerChunkPosition.z);
                        auto fromdistfromplayer = std::abs(buffer.from.value().x - playerChunkPosition.x) + std::abs(buffer.from.value().z - playerChunkPosition.z);

                        if (todistfromplayer <= fromdistfromplayer)
                        {
                            activeChunks.erase(buffer.from.value());
                            activeChunks.insert_or_assign(buffer.to, ReadyToDrawChunkInfo(buffer.chunkIndex));
                        } else
                        {
                            confirm = false;
                            std::cout << "Denying chunk rebuild wanting to move chunk to " << buffer.to.x << " " << buffer.to.z << " which is " << todistfromplayer <<
                                " distance away, further than the " << fromdistfromplayer << " it had before. Therefore, buffer index " << buffer.chunkIndex << " at " << buffer.to.x <<
                                    " " << buffer.to.z << " should be removed from meshbuildingthread's mbtActiveChunks as it is not scenically relevant and will not be reflected in mainthread's activeChunks."
                            << " and we will keep buffer index " << buffer.chunkIndex << " at " << buffer.from.value().x<<  " " << buffer.from.value().z << " where it resides. " << std::endl;
                            removeTheseFromMBTAC.push(buffer.to);
                        }

                    }
                    if (confirm )
                    {
                        confirmedActiveChunksQueue.push(buffer.to);
                    }




            
            
            buffer.ready.store(false, std::memory_order_release);
            freedChangeBuffers.push(i);  // Return to free list
            mbtBufferCV.notify_one();

                break; //Only do one per frame
            
            
        }
    }


    for(size_t i = 0; i < userChangeMeshBuffers.size(); i++) {
        auto& buffer = userChangeMeshBuffers[i];
        //std::cout << "User Buffer state: Ready: " << buffer.ready << " in use: " << buffer.in_use << std::endl;
        if(buffer.ready.load(std::memory_order_acquire) && !buffer.in_use.load(std::memory_order_acquire)) {
            //std::cout << "Buffer came through on main thread: " << buffer.to.x << " " << buffer.to.z << std::endl;

            modifyOrInitializeChunkIndex(static_cast<int>(buffer.chunkIndex), chunkPool.at(buffer.chunkIndex), buffer.mesh);
            if (buffer.from == std::nullopt)
            {

                if (!activeChunks.contains(buffer.to))
                {
                    activeChunks.insert_or_assign(buffer.to, ReadyToDrawChunkInfo(buffer.chunkIndex, 0.f));
               }

            }
            else
            {
                // activeChunks.erase(buffer.from.value());
                // activeChunks.insert_or_assign(buffer.to, ReadyToDrawChunkInfo(buffer.chunkIndex));
            }
            
            

            
            //Dont need to do this cause these user-requested chunk rebuilds never involve moving the chunk
            //confirmedActiveChunksQueue.push(buffer.to);
            buffer.ready.store(false, std::memory_order_release);
            freedUserChangeMeshBuffers.push(i);  // Return to free list
            notifyBufferFreed();

            break; //Only do one per frame
            
        }
    }


    static GLint grassRedChangeLoc = glGetUniformLocation(shader, "grassRedChange");
    static GLint timeRenderedLoc = glGetUniformLocation(shader, "timeRendered");

    static GLuint dewyFogFactorLoc = glGetUniformLocation(shader, "dewyFogFactor");

    static GLint rdistLoc = glGetUniformLocation(shader, "renderDistance");
    glUniform1f(rdistLoc, static_cast<float>(currentRenderDistance));
    for (auto& [spot, chunkinfo] : activeChunks) {
        bool isDrawingThis = true;

        if (isChunkInFrustum(spot, playerCamera->transform.position - (playerCamera->transform.direction * static_cast<float>(chunkSize)), playerCamera->transform.direction)) {
            int dist = abs(spot.x - playerChunkPosition.x) + abs(spot.z - playerChunkPosition.z);
            if (dist <= currentMinDistance()) {
                auto & glInfo = chunkPool[chunkinfo.chunkIndex];
                IntTup chunkRealSpot = IntTup(spot.x * chunkSize, spot.z * chunkSize);
                float temperature_noise = worldGenMethod->getTemperatureNoise(chunkRealSpot);
                float humidity_noise = worldGenMethod->getHumidityNoise(chunkRealSpot);
                float grassRedChange = (temperature_noise - humidity_noise);
                float dewyFogFactor = getDewyFogFactor(temperature_noise, humidity_noise);
                glUniform1f(dewyFogFactorLoc, dewyFogFactor);
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
                float temperature_noise = worldGenMethod->getTemperatureNoise(chunkRealSpot);
                float humidity_noise = worldGenMethod->getHumidityNoise(chunkRealSpot);
                float grassRedChange = (temperature_noise - humidity_noise);
                float dewyFogFactor = getDewyFogFactor(temperature_noise, humidity_noise);
                glUniform1f(dewyFogFactorLoc, dewyFogFactor);
                glUniform1f(grassRedChangeLoc, grassRedChange);
                glUniform1f(timeRenderedLoc, chunkinfo.timeBeenRendered);
                //glDisable(GL_CULL_FACE);
                if (actuallyDraw)
                {
                    drawTransparentsFromChunkIndex(static_cast<int>(chunkinfo.chunkIndex), glInfo);
                }
                //glEnable(GL_CULL_FACE);
            }
        }
    }
}



//SECOND THREAD MESH BUILDING COROUTINE
void WorldRenderer::meshBuildCoroutine(jl::Camera* playerCamera, World* world)
{


    NUM_THREADS_RUNNING.fetch_add(1);  // Atomic increment
    //std::cout << "Mesh incremented NUM_THREADS_RUNNING. Current value: " << NUM_THREADS_RUNNING.load() << "\n";

    static bool abort = false;

    for(size_t i = 0; i < changeBuffers.size(); i++) {
        freedChangeBuffers.push(i);
    }
    for(size_t i = 0; i < userChangeMeshBuffers.size(); i++) {
        freedUserChangeMeshBuffers.push(i);
    }

        mbtActiveChunks.reserve(currentMaxChunks() + 4);
        activeChunks.reserve(currentMaxChunks() + 4);

    while(meshBuildingThreadRunning)
    {

        static int persistenceDenominatorDefault = 8;

        static int persistenceDenominator = 8;

        //std::cout << "Restarting this process" << std::endl;


        std::unordered_set<TwoIntTup, TwoIntTupHash> implicatedChunks;

        static TwoIntTup lastplayercpos = TwoIntTup(0, 0);

        std::chrono::time_point<std::chrono::system_clock> timeAtStart = std::chrono::system_clock::now();

        TwoIntTup playerChunkPosition = worldToChunkPos(
        TwoIntTup(std::floor(playerCamera->transform.position.x),
                     std::floor(playerCamera->transform.position.z)));

        static std::vector<TwoIntTup> checkspots((currentRenderDistance * 2) * (currentRenderDistance * 2));

        static int lastRendDist = currentRenderDistance;
        if (currentRenderDistance != lastRendDist) {
            lastRendDist = currentRenderDistance;
            checkspots.reserve((currentRenderDistance * 2) * (currentRenderDistance * 2));
        }

        static int lastMaxChunks = maxChunks;
        int cmc = currentMaxChunks();
        if (cmc != lastMaxChunks) {
            lastMaxChunks = cmc;
            chunkPool.resize(cmc);
        }


        int checkspotssize = ((currentRenderDistance/persistenceDenominator)*2) * ((currentRenderDistance/persistenceDenominator)*2);


        int index = 0;
        for (int i = -currentRenderDistance/persistenceDenominator; i < currentRenderDistance/persistenceDenominator; i++)
        {
            for (int j = -currentRenderDistance/persistenceDenominator; j < currentRenderDistance/persistenceDenominator; j++)
            {
                checkspots[index] = playerChunkPosition + TwoIntTup(i,j);
                index+=1;
            }
        }



        std::ranges::sort(checkspots.begin(), checkspots.begin() + checkspotssize, [&playerChunkPosition](const TwoIntTup& a, const TwoIntTup& b)
        {
            int distfroma = abs(a.x - playerChunkPosition.x) + abs(a.z - playerChunkPosition.z);
            int distfromb = abs(b.x - playerChunkPosition.x) + abs(b.z - playerChunkPosition.z);
            return distfroma < distfromb;
        });

        TwoIntTup cpcp = worldToChunkPos(
                TwoIntTup(std::floor(playerCamera->transform.position.x),
                    std::floor(playerCamera->transform.position.z)));

        for (int iz = 0; iz < checkspotssize; iz++)
        {

            auto spotHere = checkspots.at(iz);


            int dist = abs(spotHere.x - cpcp.x) + abs(spotHere.z - cpcp.z);
            if(dist <= currentMinDistance()/persistenceDenominator)
            {
                auto acc = tbb::concurrent_hash_map<TwoIntTup, bool, TwoIntTupHashCompare>::const_accessor();
                if(!generatedChunks.find(acc, spotHere))
                {
                    generateChunk(world, spotHere, &implicatedChunks);
                    generatedChunks.insert({spotHere, true});

                }
            }
        }


        for (int iz = 0; iz < checkspotssize; iz++)
        {

            //not for chunks, but for checking if  we need to change the persistence denominator and restart the process
            auto newcpos = worldToChunkPos(
            TwoIntTup(std::floor(playerCamera->transform.position.x),
                         std::floor(playerCamera->transform.position.z)));



                auto distfromlast = std::abs(newcpos.x - lastplayercpos.x) + abs(newcpos.z - lastplayercpos.z);
                if (distfromlast > currentRenderDistance / 8)
                {
                    persistenceDenominator = std::min(persistenceDenominator+1, persistenceDenominatorDefault);
                    break;
                } else
                {
                    persistenceDenominator = std::max(1, persistenceDenominator - 1);
                }

            auto spotHere = checkspots.at(iz);




            TwoIntTup toRemove = {};
            while (removeTheseFromMBTAC.pop(&toRemove) && meshBuildingThreadRunning)
            {
                mbtActiveChunks.erase(toRemove);
            }

            for (auto & chunk : implicatedChunks)
            {
                if (mbtActiveChunks.contains(chunk))
                {
                    mbtActiveChunks.at(chunk).dontRepurpose = true;


                    /*auto destdistfromcpcp = glm::abs(chunk.x - cpcp.x) + glm::abs(chunk.z - cpcp.z);

                    bool validchange = destdistfromcpcp < currentMinDistance();*/

                    //if (validchange) {
                        auto& uci = mbtActiveChunks.at(chunk);

                        UsableMesh mesh;
                        {


                            mesh = fromChunk(chunk, world, false, true);
                            //I think have to do light pass for generation implicated chunks because they put new blocks there


                        }

                        size_t changeBufferIndex = -1;
                        {
                            std::unique_lock<std::mutex> lock(mbtBufferMutex);
                            mbtBufferCV.wait(lock, [this, &changeBufferIndex]() {
                                bool popped = freedChangeBuffers.pop(changeBufferIndex);
                                                    if (popped)
                                                    {
                                                        auto & cb = changeBuffers[changeBufferIndex];
                                                        cb.in_use.store(true, std::memory_order_release);
                                                    }
                                                    return popped || !meshBuildingThreadRunning;
                                });

                            if (!meshBuildingThreadRunning) break;

                            auto newcpos = worldToChunkPos(
                            TwoIntTup(std::floor(playerCamera->transform.position.x),
                                         std::floor(playerCamera->transform.position.z)));



                                auto distfromlast = std::abs(newcpos.x - lastplayercpos.x) + abs(newcpos.z - lastplayercpos.z);
                                if (distfromlast > currentRenderDistance / 8)
                                {
                                    persistenceDenominator = std::min(persistenceDenominator+1, persistenceDenominatorDefault);
                                    break;
                                } else
                                {
                                    persistenceDenominator = std::max(1, persistenceDenominator - 1);
                                }
                        }

                        if (changeBufferIndex != -1)
                        {
                            auto & buffer = changeBuffers[changeBufferIndex];
                            buffer.mesh = std::move(mesh);
                            buffer.chunkIndex = uci.chunkIndex;
                            buffer.from = std::nullopt;
                            buffer.to = chunk;
                            buffer.ready.store(true, std::memory_order_release);
                            buffer.in_use.store(false, std::memory_order_release);
                        }
                    //}



                    


                }

            }
            implicatedChunks.clear();

                TwoIntTup confirmedChunk;
                while (confirmedActiveChunksQueue.pop(confirmedChunk) && meshBuildingThreadRunning) {
                    if (mbtActiveChunks.contains(confirmedChunk))
                    {
                        mbtActiveChunks.at(confirmedChunk).confirmedByMainThread = true;
                        //If we're confirmed to be in activechunks, then we can be repurposed
                        mbtActiveChunks.at(confirmedChunk).dontRepurpose = false;
                    }
                    else {
                        std::cout << "Chunk not in mbtActiveChunks confirmed as being in activeChunks" << std::endl;
                    }


                }

                    newcpos = worldToChunkPos(
                        TwoIntTup(std::floor(playerCamera->transform.position.x),
                            std::floor(playerCamera->transform.position.z)));
                    int dist = abs(spotHere.x - newcpos.x) + abs(spotHere.z - newcpos.z);
                    if(dist <= currentMinDistance()/persistenceDenominator)
                    {
                        if (!mbtActiveChunks.contains(spotHere))
                        {
                                // if(!generatedChunks.contains(spotHere))
                                // {
                                //     generateChunk(world, spotHere, implicatedChunks);
                                // }
                            //std::cout << "Spot " << i << " " << j << std::endl;

                            //IF we havent reached the max chunks yet, we can just make a new one for this spot.
                            //ELSE, we reuse the furthest one from the player, ONLY IF the new distance will be shorter than the last distance!

                            if (chunkPoolSize.load(std::memory_order_acquire) < (static_cast<unsigned long long>(currentMaxChunks()) - 4))
                            {
                                size_t changeBufferIndex = -1;
                                {
                                    std::unique_lock<std::mutex> lock(mbtBufferMutex);
                                    mbtBufferCV.wait(lock, [this, &changeBufferIndex]() {
                                        bool popped = freedChangeBuffers.pop(changeBufferIndex);
                                                    if (popped)
                                                    {
                                                        auto & cb = changeBuffers[changeBufferIndex];
                                                        cb.in_use.store(true, std::memory_order_release);
                                                    }
                                                    return popped || !meshBuildingThreadRunning;
                                    });

                                    if (!meshBuildingThreadRunning) break;

                                    auto newcpos = worldToChunkPos(
                                    TwoIntTup(std::floor(playerCamera->transform.position.x),
                                                 std::floor(playerCamera->transform.position.z)));



                                    auto distfromlast = std::abs(newcpos.x - lastplayercpos.x) + abs(newcpos.z - lastplayercpos.z);
                                    if (distfromlast > currentRenderDistance / 8)
                                    {
                                        persistenceDenominator = std::min(persistenceDenominator+1, persistenceDenominatorDefault);
                                        break;
                                    } else
                                    {
                                        persistenceDenominator = std::max(1, persistenceDenominator - 1);
                                    }
                                }

                                if (changeBufferIndex != -1)
                                {
                                    //Add the mesh, in full form, to our reserved Change Buffer (The main thread coroutine will make GL calls and free this slot to be reused)

                                    auto& buffer = changeBuffers[changeBufferIndex];
                                    buffer.in_use.store(true, std::memory_order_release);
                                    buffer.mesh = fromChunk(spotHere, world, false);

                                    buffer.chunkIndex = addUninitializedChunkBuffer();
                                   // std::cout << "Giving new buffer " << buffer.chunkIndex << std::endl;
                                    buffer.from = std::nullopt;
                                    buffer.to = spotHere;

                                    mbtActiveChunks.insert_or_assign(spotHere, UsedChunkInfo(buffer.chunkIndex));

                                    buffer.ready.store(true, std::memory_order_release);   // Signal that data is ready
                                    buffer.in_use.store(false, std::memory_order_release);
                                }

                            } else
                            {


                                std::vector<std::pair<int,TwoIntTup>> chunksWithDistances;

                                for (const auto& [chunkPos, usedChunkInfo] : mbtActiveChunks) {

                                    if (!usedChunkInfo.confirmedByMainThread || usedChunkInfo.dontRepurpose)
                                    {
                                        //We only want ones confirmed by main thread, we know we can repurpose those
                                        //Also if marked dontRepurpose of course
                                        continue;
                                    }

                                    // Calculate distance to yourPosition
                                    newcpos = stupidWorldRendererWorldToChunkPos(
                                        TwoIntTup(std::floor(playerCamera->transform.position.x),
                                            std::floor(playerCamera->transform.position.z)));

                                    int distance = abs(chunkPos.x - newcpos.x) + abs(chunkPos.z - newcpos.z);
                                    // int betterdistance = glm::round(glm::distance(glm::vec2(chunkPos.x, chunkPos.z), glm::vec2(cpcp.x, cpcp.z)));

                                    // Filter out chunks closer than MIN_DISTANCE (We only want to repurpose chunks outside of currentMinDistance
                                    if (distance > currentMinDistance()/persistenceDenominator) {
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
                                        // //If the place we're going will afford a shorter distance from player, choose this one.
                                        // int newDistance = abs(spotHere.x - cpcp.x) + abs(spotHere.z - cpcp.z);
                                        //
                                        // if (newDistance < (oldDistance-2)) //add some epsilon or range so it stops grabbing chunks from the edge of valid-chunk-range & leaving holes
                                        // {
                                            size_t chunkIndex = mbtActiveChunks.at(oldSpot).chunkIndex;
                                            //
                                            // std::chrono::high_resolution_clock::time_point time = std::chrono::high_resolution_clock::now();

                                            size_t changeBufferIndex = -1;
                                            {
                                                std::unique_lock<std::mutex> lock(mbtBufferMutex);
                                                mbtBufferCV.wait(lock, [this, &changeBufferIndex]() {
                                                    bool popped = freedChangeBuffers.pop(changeBufferIndex);
                                                    if (popped)
                                                    {
                                                        auto & cb = changeBuffers[changeBufferIndex];
                                                        cb.in_use.store(true, std::memory_order_release);
                                                    }
                                                    return popped || !meshBuildingThreadRunning;
                                                });

                                                if (!meshBuildingThreadRunning) break;

                                                auto newcpos = worldToChunkPos(
                                                TwoIntTup(std::floor(playerCamera->transform.position.x),
                                                             std::floor(playerCamera->transform.position.z)));



                                                auto distfromlast = std::abs(newcpos.x - lastplayercpos.x) + abs(newcpos.z - lastplayercpos.z);
                                                if (distfromlast > currentRenderDistance / 8)
                                                {
                                                    persistenceDenominator = std::min(persistenceDenominator+1, persistenceDenominatorDefault);
                                                    break;
                                                } else
                                                {
                                                    persistenceDenominator = std::max(1, persistenceDenominator - 1);
                                                }
                                            }
                                            //
                                            // std::chrono::high_resolution_clock::time_point time2 = std::chrono::high_resolution_clock::now();
                                            // std::chrono::duration<float> elapsed = time2 - time;
                                            // std::cout << "elapsed: " << std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() << std::endl;

                                            if (changeBufferIndex != -1)
                                            {
                                                //Add the mesh, in full form, to our reserved Change Buffer (The main thread coroutine will make GL calls and free this slot to be reused)

                                                auto& buffer = changeBuffers[changeBufferIndex];

                                                buffer.mesh = fromChunk(spotHere, world, false);

                                                buffer.chunkIndex = chunkIndex;
                                                buffer.from = oldSpot;
                                                buffer.to = spotHere;



                                                mbtActiveChunks.erase(oldSpot);
                                                generatedChunks.erase(oldSpot);
                                                litChunks.erase(oldSpot);
                                              {
                                                    auto lmlock = std::unique_lock<std::shared_mutex>(lightmapMutex);
                                                    ambientlightmap.deleteChunk(oldSpot);
                                                    lightmap.deleteChunk(oldSpot);
                                                }

                                                 world->nonUserDataMap.erase(oldSpot);
                                                mbtActiveChunks.insert_or_assign(spotHere, UsedChunkInfo(chunkIndex));

                                                buffer.ready.store(true, std::memory_order_release);   // Signal that data is ready
                                                buffer.in_use.store(false, std::memory_order_release);
                                            }



                                            break;
                                        //}

                                    }
                                }



                            }

                        }
                    }



        }

        auto timeAtEnd = std::chrono::system_clock::now();

        auto elapsed = timeAtEnd - timeAtStart;



        std::this_thread::sleep_for(std::chrono::milliseconds(500 - std::min(500, (int)std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count())));
        auto newcpos = worldToChunkPos(
        TwoIntTup(std::floor(playerCamera->transform.position.x),
                     std::floor(playerCamera->transform.position.z)));



        auto distfromlast = std::abs(newcpos.x - lastplayercpos.x) + abs(newcpos.z - lastplayercpos.z);
        if (distfromlast > currentRenderDistance / 8)
        {
            persistenceDenominator = std::min(persistenceDenominator+1, persistenceDenominatorDefault);
        } else
        {
            persistenceDenominator = std::max(1, persistenceDenominator - 1);
        }

        lastplayercpos = newcpos;
    }

    mbtActiveChunks.clear();

    NUM_THREADS_RUNNING.fetch_sub(1);


}

void WorldRenderer::rebuildThreadFunction(World* world)
{

    NUM_THREADS_RUNNING.fetch_add(1);  // Atomic increment
    //std::cout << "Rebuild thread incremented NUM_THREADS_RUNNING. Current value: " << NUM_THREADS_RUNNING.load() << "\n";

    while (rebuildThreadRunning) {
        ChunkRebuildRequest request;
        //std::cout << "Running \n";
        if (rebuildQueue.pop(request)) {
            //std::cout << "Popped one: " << request.chunkPos.x << " " << request.chunkPos.z << " \n";

            auto lightpass = (request.changeTo != std::nullopt) || request.doLightPass;
      /*      if (lightpass)
            {
                request.rebuild = true;
            }*/
            if (request.isArea)
            {
                std::vector<IntTup> spotsToEraseInUDM;
                spotsToEraseInUDM.reserve(500);


                {
                    //We're gonna do the block placing, then ask main to request the rebuilds because we won't know what chunks are active when we're done.
                    std::shared_lock<std::shared_mutex> udmRL(world->userDataMap.mutex());
                    auto lock = world->nonUserDataMap.getUniqueLock();

                    auto m = request.area;
                    int minX = std::min(m.corner1.x, m.corner2.x);
                    int maxX = std::max(m.corner1.x, m.corner2.x);
                    int minY = std::min(m.corner1.y, m.corner2.y);
                    int maxY = std::max(m.corner1.y, m.corner2.y);
                    int minZ = std::min(m.corner1.z, m.corner2.z);
                    int maxZ = std::max(m.corner1.z, m.corner2.z);

                    for (int x = minX; x <= maxX; x++) {
                        for (int y = minY; y <= maxY; y++) {
                            for (int z = minZ; z <= maxZ; z++) {
                                bool isBoundary = (x == minX || x == maxX ||
                                    y == minY || y == maxY ||
                                    z == minZ || z == maxZ);

                                if (isBoundary || !m.hollow) {
                                    world->setNUDMLocked(IntTup{x, y, z}, m.block);
                                    if (world->userDataMap.getUnsafe(IntTup{x, y, z}) != std::nullopt)
                                    {
                                        spotsToEraseInUDM.emplace_back(x, y, z);
                                    }
                                }

                            }
                        }
                    }
                }

                {
                    auto lock = world->userDataMap.getUniqueLock();
                    for (auto & spot : spotsToEraseInUDM)
                    {
                        world->userDataMap.erase(spot, true);
                    }
                }


                if (rebuildToMainAreaNotifications.write_available() > 0)
                {
                    rebuildToMainAreaNotifications.push(request.area);
                } else
                {

                }


            } else
                if (request.isVoxelModel)
                {
                    auto & vm = voxelModels[request.vm.name];
                    std::vector<IntTup> spotsToEraseInUDM;
                    spotsToEraseInUDM.reserve(500);

                    {
                        //We're gonna do the block placing, then ask main to request the rebuilds because we won't know what chunks are active when we're done.
                        std::unordered_set<TwoIntTup, TwoIntTupHash> implicatedChunks;
                        std::shared_lock<std::shared_mutex> udmRL(world->userDataMap.mutex());
                        auto lock = world->nonUserDataMap.getUniqueLock();


                        IntTup offset = IntTup(vm.dimensions.x/-2, 0, vm.dimensions.z/-2) + request.vm.spot;
                        for ( auto & p : vm.points)
                        {
                            world->setNUDMLocked(offset+p.localSpot, p.colorIndex);
                            if (world->userDataMap.getUnsafe(offset+p.localSpot) != std::nullopt)
                            {
                                spotsToEraseInUDM.emplace_back(offset+p.localSpot);
                            }

                        }



                    }
                    {
                        auto lock = world->userDataMap.getUniqueLock();
                        for (auto & spot : spotsToEraseInUDM)
                        {
                            world->userDataMap.erase(spot, true);
                        }
                    }
                    if (rebuildToMainAreaNotifications.write_available() > 0)
                    {
                        IntTup corner1 = IntTup(vm.dimensions.x/-2, 0, vm.dimensions.z/-2) + request.vm.spot;
                        IntTup corner2 = IntTup(vm.dimensions.x/2, vm.dimensions.y, vm.dimensions.z/2) + request.vm.spot;

                        rebuildToMainAreaNotifications.push(BlockArea{
                            corner1, corner2, AIR, false
                        });
                    } else
                    {

                    }

                }else
                {

                    if(request.changeTo != std::nullopt)
                    {



                        auto blockThere = world->get(request.changeSpot);
                        // if (blockThere == LIGHT)
                        // {
                        //
                        //     //request.rebuild = true;
                        //     //lightpass = true;
                        //     //  std::vector<std::pair<IntTup, ColorPack>> thisspot = {
                        //     //      std::make_pair(request.changeSpot, SKYLIGHTVAL)
                        //     //  };
                        //     // unpropagateAllLightsLayered(thisspot, lightmap, request.chunkPos);
                        // }

                        // std::vector<std::pair<IntTup, int>> thisspot = {
                        //     std::make_pair(request.changeSpot + IntTup(0,1,0), 12)
                        // };
                        // unpropagateAllLightsLayered(thisspot, ambientlightmap, request.chunkPos);
                        if (blockThere != AIR  && (blockThere != (MaterialName)(request.changeTo.value() & BLOCK_ID_BITS)))
                        {
                            if (auto f = findSpecialRemoveBits((MaterialName)blockThere); f != std::nullopt)
                            {

                                f.value()(world, request.changeSpot);
                            }
                        }
                        //std::cout <<"Doing the fucking write to " << request.changeSpot.x << " " << request.changeSpot.y << " " << request.changeSpot.z << " \n";
                        if (auto sbf = findSpecialSetBits((MaterialName)request.changeTo.value()); sbf != std::nullopt)
                        {
                            sbf.value()(world, request.changeSpot, request.playerPosAtPlaceTime);
                        } else
                        {
                            world->set(request.changeSpot, request.changeTo.value());
                        }



                    }
                    if(request.rebuild)
                    {
                        UsableMesh mesh;

                        {

                            if (request.queueLightpassImplicated)
                            {
                                mesh = fromChunk(request.chunkPos, world, lightpass);
                            } else
                            {
                                mesh = fromChunk<false>(request.chunkPos, world, false, lightpass);
                            }


                        }


                        // Wait for available buffer using condition variable
                        size_t changeBufferIndex = -1;
                        {
                            std::unique_lock<std::mutex> lock(bufferMutex);
                            bufferCV.wait(lock, [this, &changeBufferIndex]() {


                                bool popped = freedUserChangeMeshBuffers.pop(changeBufferIndex);
                                                    if (popped)
                                                    {
                                                        auto & cb = userChangeMeshBuffers[changeBufferIndex];
                                                        cb.in_use.store(true, std::memory_order_release);
                                                    }
                                                    return popped || !rebuildThreadRunning;
                            });

                            if (!rebuildThreadRunning) break;
                        }

                        if (changeBufferIndex != -1)
                        {
                            auto& buffer = userChangeMeshBuffers[changeBufferIndex];
                            buffer.in_use.store(true, std::memory_order_release);
                            buffer.mesh = std::move(mesh);
                            buffer.chunkIndex = request.chunkIndex;
                            buffer.from = std::nullopt;
                            buffer.to = request.chunkPos;
                            buffer.ready.store(true, std::memory_order_release);
                            buffer.in_use.store(false, std::memory_order_release);
                        }
                        //
                        // if (lightpass)
                        // {
                        //     for (auto & chunk: implicatedChunks)
                        //     {
                        //
                        //     }
                        // }
                    }
                }





        }
    }
    NUM_THREADS_RUNNING.fetch_sub(1);

}

void WorldRenderer::generateChunk(World* world, const TwoIntTup& chunkSpot, std::unordered_set<TwoIntTup, TwoIntTupHash>* implicatedChunks)
{

    srand(chunkSpot.x * 73856093 ^ chunkSpot.z * 19349663);
    for (int o = 0; o < 3; o++)
    {
        auto offset1 = ((float)rand() / RAND_MAX);
        auto offset2 = ((float)rand() / RAND_MAX);
        auto therand = rand();
        auto thechange = world->worldGenMethod->getTemperatureNoise(IntTup(chunkSpot.x * 16, 60, chunkSpot.z * 16)) * -100000;
        //std::cout << thechange << std::endl;
        if (therand > 15000 + thechange)
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
                        auto chunkhere = WorldRenderer::stupidWorldRendererWorldToChunkPos(TwoIntTup(finalspot.x, finalspot.z));
                        if (implicatedChunks && chunkhere != chunkSpot)
                        {
                            implicatedChunks->insert(chunkhere);
                        }
                        world->setNUDM(finalspot, point.colorIndex);
                    }
                }
            }
        }
    }


    // for (auto & chunk: implicatedChunks)
    // {
    //
    // }


}

void WorldRenderer::clearInFlightMeshUpdates()
{
    for (auto & buffer : changeBuffers)
    {
        buffer.ready = false;
        buffer.in_use = false;
    }
    for (auto & buffer : userChangeMeshBuffers)
    {
        buffer.ready = false;
        buffer.in_use = false;
    }
    freedChangeBuffers.consume_all([](auto){});
    freedUserChangeMeshBuffers.consume_all([](auto){});
    rebuildToMainAreaNotifications.consume_all([](auto){});
    confirmedActiveChunksQueue.consume_all([](auto){});
    removeTheseFromMBTAC.consume_all([](auto){});
    generatedChunks.clear();
    litChunks.clear();

    // for (int i = 0; i < changeBuffers.size(); i++)
    // {
    //     freedChangeBuffers.push(i);
    // }
    // for (int i = 0; i < userChangeMeshBuffers.size(); i++)
    // {
    //     freedUserChangeMeshBuffers.push(i);
    // }
}





UsableMesh fromChunk(const TwoIntTup& spot, World* world, bool light)
{
    return fromChunk(spot, world, false, light);
}
UsableMesh fromChunkLocked(const TwoIntTup& spot, World* world, bool light)
{
    return fromChunk(spot, world, true, light);
}

//#define MEASURE_CHUNKREB



// This function calculates and adds ambient occlusion brightness values for a face
void calculateAmbientOcclusion(const IntTup& blockPos, Side side, World* world, bool locked, BlockType blockType, UsableMesh& mesh, float
                               blockandambbright)
{
    // float baseBrightness;
    // switch(side) {
    //     case Side::Top:    baseBrightness = 1.0f * (blockandambbright / 16.0f); break;
    //     case Side::Left:   baseBrightness = 0.7f * (blockandambbright / 16.0f); break;
    //     case Side::Bottom: baseBrightness = 0.4f * (blockandambbright / 16.0f); break;
    //     case Side::Right:  baseBrightness = 0.8f * (blockandambbright / 16.0f); break;
    //     default:           baseBrightness = 0.9f * (blockandambbright / 16.0f); break;
    // }

    float isGrass = grasstypes.test(blockType) ? 1.0f : 0.0f;

    // Calculate occlusion for each vertex of the face
    int occlusion[4];
    for (int v = 0; v < 4; v++) {
        // Get the 3 adjacent blocks for this vertex
        std::array<IntTup, 3> adjacentOffsets = getAdjacentOffsets(side, v);

        // Count solid adjacent blocks
        int solidCount = 0;
        for (const auto& offset : adjacentOffsets) {
            IntTup adjPos = blockPos + offset;
            BlockType adjBlock = locked ? world->getLocked(adjPos) : world->get(adjPos);
            if (adjBlock != AIR && !noAmbOccl.test(adjBlock)) {
                solidCount++;
            }
        }

        // Apply occlusion based on how many adjacent blocks are solid
        int occlusionValue = 0;

        switch (solidCount) {
            case 1: occlusionValue = 1; break;
            case 2: occlusionValue = 2; break;
            case 3: occlusionValue = 3; break;
            default: occlusionValue = 0; break;
        }

        occlusion[v] = occlusionValue;
    }
    static auto packonocclbits = [](int occlusion, float blockandambbright)
    {
        uint32_t packed;
        memcpy(&packed, &blockandambbright, sizeof(float));

        // Step 2: Set the occlusion bits (let's use bits 14-15)
        // First clear those bits
        packed &= ~(0x3 << 14);  // Clear bits 14-15
        packed |= (uint32_t(occlusion) << 14);  // Set occlusion in bits 14-15

        // Step 3: Convert back to float
        float result;
        memcpy(&result, &packed, sizeof(float));
        return result;
    };

    // Add brightness data to the appropriate arrays based on transparency
    bool isTransparent = transparents.test(blockType);
    if (isTransparent) {
        mesh.tbrightness.insert(mesh.tbrightness.end(), {
            packonocclbits(occlusion[0], blockandambbright), isGrass, packonocclbits(occlusion[1], blockandambbright), isGrass,
            packonocclbits(occlusion[2], blockandambbright), isGrass, packonocclbits(occlusion[3], blockandambbright), isGrass
        });
    } else {
        mesh.brightness.insert(mesh.brightness.end(), {
            packonocclbits(occlusion[0], blockandambbright), isGrass, packonocclbits(occlusion[1], blockandambbright), isGrass,
            packonocclbits(occlusion[2], blockandambbright), isGrass, packonocclbits(occlusion[3], blockandambbright), isGrass
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