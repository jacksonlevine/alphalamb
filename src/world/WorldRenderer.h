//
// Created by jack on 1/27/2025.
//

//#define MARCHERS

#ifndef WORLDRENDERER_H
#define WORLDRENDERER_H

#include "MaterialName.h"
#include "World.h"
#include "../Camera.h"
#include "../IntTup.h"
#include "../PrecompHeader.h"
#include "RebuildQueue.h"
#include "../Helpers.h"
#include "../Light.h"
#include "../NetworkTypes.h"
#include "../MarchingCubesLookups.h"
using namespace physx;
constexpr float onePixel = 0.00183823529411764705882352941176f;     //  1/544      Padding
constexpr float textureWidth = 0.02941176470588235294117647058824f; // 16/544      16 pixel texture width
constexpr float texSlotWidth = 0.03308823529411764705882352941176f;
inline static PxVec3 cubefaces[6][4] = {
    {//front
        PxVec3(0.0, 0.0, 0.0),PxVec3(1.0, 0.0, 0.0),PxVec3(1.0, 1.0, 0.0),PxVec3(0.0, 1.0, 0.0)},
    {//right
        PxVec3(1.0, 0.0, 0.0),PxVec3(1.0, 0.0, 1.0),PxVec3(1.0, 1.0, 1.0),PxVec3(1.0, 1.0, 0.0)},
    {//back
        PxVec3(1.0, 0.0, 1.0),PxVec3(0.0, 0.0, 1.0),PxVec3(0.0, 1.0, 1.0),PxVec3(1.0, 1.0, 1.0)},
    {//left
        PxVec3(0.0, 0.0, 1.0),PxVec3(0.0, 0.0, 0.0),PxVec3(0.0, 1.0, 0.0),PxVec3(0.0, 1.0, 1.0)},
    {//top
        PxVec3(0.0, 1.0, 0.0),PxVec3(1.0, 1.0, 0.0),PxVec3(1.0, 1.0, 1.0),PxVec3(0.0, 1.0, 1.0)},
    {//bottom
        PxVec3(0.0, 0.0, 1.0),PxVec3(1.0, 0.0, 1.0),PxVec3(1.0, 0.0, 0.0),PxVec3(0.0, 0.0, 0.0)}
};



struct DrawInstructions
{
    GLuint vao = 0;
    int indiceCount = 0;
    GLuint tvao = 0;
    int tindiceCount = 0;
};

using namespace physx;
class UsableMesh
{
public:
    // Constructor that reserves space for 64 elements in each vector
    UsableMesh()
    {
        positions.reserve(64);
        texcoords.reserve(64);
        indices.reserve(64);
        brightness.reserve(64);
        tpositions.reserve(64);
        ttexcoords.reserve(64);
        tindices.reserve(64);
        tbrightness.reserve(64);
    }

    // Delete copy constructor and copy assignment to prevent copying
    UsableMesh(const UsableMesh&) = delete;
    UsableMesh& operator=(const UsableMesh&) = delete;

    // Move constructor and move assignment are implicitly defined (or can be defaulted)
    UsableMesh(UsableMesh&&) noexcept = default;
    UsableMesh& operator=(UsableMesh&&) noexcept = default;

    // Vectors with reserved capacity
    std::vector<PxVec3> positions;
    std::vector<glm::vec2> texcoords;
    std::vector<PxU32> indices;
    std::vector<float> brightness;

    std::vector<PxVec3> tpositions;
    std::vector<glm::vec2> ttexcoords;
    std::vector<PxU32> tindices;
    std::vector<float> tbrightness;
};

struct ChunkGLInfo
{
    GLuint vvbo{}, uvvbo{}, bvbo{}, ebo = 0;
    GLuint tvvbo{}, tuvvbo{}, tbvbo{}, tebo = 0;
    DrawInstructions drawInstructions = {};
};

struct SmallChunkGLInfo
{
    int indiceCount = 0;
    int tindiceCount = 0;
};

struct ChangeBuffer
{
    UsableMesh mesh = {};
    uint16_t chunkIndex : 14 = 0;
    std::optional<TwoIntTup> from = std::nullopt;
    TwoIntTup to = {};
    std::atomic<bool> ready = false;
    std::atomic<bool> in_use = false;
};


void modifyOrInitializeDrawInstructions(GLuint& vvbo, GLuint& uvvbo, GLuint& ebo, DrawInstructions& drawInstructions, UsableMesh& usable_mesh, GLuint& bvbo,
    GLuint& tvvbo, GLuint& tuvvbo, GLuint& tebo, GLuint& tbvbo);
inline void modifyOrInitializeChunkGLInfo(ChunkGLInfo& info, UsableMesh& mesh)
{
    modifyOrInitializeDrawInstructions(info.vvbo, info.uvvbo, info.ebo, info.drawInstructions, mesh, info.bvbo, info.tvvbo, info.tuvvbo, info.tebo, info.tbvbo);
}
void modifyOrInitializeChunkIndex(int chunkIndex, SmallChunkGLInfo& info, UsableMesh& usable_mesh);
void drawTransparentsFromDrawInstructions(const DrawInstructions& drawInstructions);
void drawFromDrawInstructions(const DrawInstructions& drawInstructions);

void drawFromChunkIndex(int chunkIndex, const SmallChunkGLInfo& cgl);
void drawTransparentsFromChunkIndex(int chunkIndex, const SmallChunkGLInfo& cgl);


enum class Side
{
    Front = 0,Right,Back,Left,Top,Bottom
};


inline static IntTup neighborSpots[6] = {
    IntTup(0,0,-1),
    IntTup(1, 0,0),
    IntTup(0,0,1),
    IntTup(-1,0,0),
    IntTup(0,1,0),
    IntTup(0,-1,0)
};
inline static IntTup neighsAndCorns[8+6] = {
    IntTup(0, 0, 0),   // Corner 0: BottomFrontLeft
    IntTup(1, 0, 0),   // Corner 1: BottomFrontRight
    IntTup(0, 0, 1),   // Corner 2: BottomBackLeft
    IntTup(1, 0, 1),   // Corner 3: BottomBackRight
    IntTup(0, 1, 0),   // Corner 4: TopFrontLeft
    IntTup(1, 1, 0),   // Corner 5: TopFrontRight
    IntTup(0, 1, 1),   // Corner 6: TopBackLeft
    IntTup(1, 1, 1),    // Corner 7: TopBackRight
    IntTup(0,0,-1),
    IntTup(1, 0,0),
    IntTup(0,0,1),
    IntTup(-1,0,0),
    IntTup(0,1,0),
    IntTup(0,-1,0)
};

enum class Corner
{
    BottomFrontLeft = 0,   // (0,0,0) - bit 0
    BottomFrontRight,      // (1,0,0) - bit 1
    BottomBackLeft,        // (0,0,1) - bit 2
    BottomBackRight,       // (1,0,1) - bit 3
    TopFrontLeft,          // (0,1,0) - bit 4
    TopFrontRight,         // (1,1,0) - bit 5
    TopBackLeft,           // (0,1,1) - bit 6
    TopBackRight           // (1,1,1) - bit 7
};

#ifdef MARCHERS

inline static IntTup cornerPositions[8] = {
    IntTup(0, 0, 0),   // Corner 0: BottomFrontLeft
    IntTup(1, 0, 0),   // Corner 1: BottomFrontRight
    IntTup(0, 0, 1),   // Corner 2: BottomBackLeft
    IntTup(1, 0, 1),   // Corner 3: BottomBackRight
    IntTup(0, 1, 0),   // Corner 4: TopFrontLeft
    IntTup(1, 1, 0),   // Corner 5: TopFrontRight
    IntTup(0, 1, 1),   // Corner 6: TopBackLeft
    IntTup(1, 1, 1)    // Corner 7: TopBackRight
};

#endif

inline static PxU32 cwindices[6] = {
    0, 3, 2, 2, 1, 0
};


inline static PxU32 ccwindices[6] = {
    0, 1, 2, 2, 3, 0
};

inline static boost::unordered_map<IntTup, Side, IntTupHash> neigh2Side = {
    {neighborSpots[0], (Side)0},
    {neighborSpots[1], (Side)1},
    {neighborSpots[2], (Side)2},
    {neighborSpots[3], (Side)3},
    {neighborSpots[4], (Side)4},
    {neighborSpots[5], (Side)5},
};


template<bool doBrightness = true>
__inline void addFace(PxVec3 offset, Side side, MaterialName material, int sideHeight, UsableMesh& mesh, uint32_t& index, uint32_t& tindex, float offsety = 0.f, float pushIn = 0.f)
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
    if (transparents.test(material))
    {
        std::ranges::transform(cubefaces[static_cast<int>(side)], std::back_inserter(mesh.tpositions), [offset, sideHeight, offsety, pushIn](const auto& v) {
            auto newv = PxVec3(pushIn, pushIn, pushIn) + v*(1.f- pushIn);
            newv.y *= sideHeight;
            return newv + offset + PxVec3(0.f, offsety, 0.f);
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
            float isGrass = grasstypes.test(material) ? 1.0f : 0.0f;

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
        std::ranges::transform(cubefaces[static_cast<int>(side)], std::back_inserter(mesh.positions), [pushIn, offset, sideHeight, offsety](const auto& v) {
            auto newv = PxVec3(pushIn, pushIn, pushIn) + v*(1.f- pushIn);
            newv.y *= sideHeight;
            return newv + offset + PxVec3(0.f, offsety, 0.f);
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
            float isGrass = grasstypes.test(material) ? 1.0f : 0.0f;

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



#ifdef MARCHERS

template<bool doBrightness = true>
__inline void addMarcher(PxVec3 offset, uint8_t configindex, MaterialName material, int sideHeight, UsableMesh& mesh, uint32_t& index, uint32_t& tindex, float offsety = 0.f, float pushIn = 0.f, float
                         blockAndAmbBright = 0)
{
    auto & tex = TEXS[material];
    auto faceindex = 0;
    auto & face = tex[faceindex];

    float uvoffsetx = static_cast<float>(face.first) * texSlotWidth;
    float uvoffsety = 1.0f - (static_cast<float>(face.second) * texSlotWidth);

    static glm::vec2 texOffsets[] = {
        glm::vec2(onePixel, -onePixel),
        glm::vec2(onePixel + textureWidth, -onePixel),
        glm::vec2(onePixel + textureWidth, -(onePixel + textureWidth)),
        glm::vec2(onePixel, -(onePixel + textureWidth))
    };

    auto positiveMod = [](float a, float b) {
        return std::fmod(std::fmod(a, b) + b, b);
    };

    {
        std::ranges::transform(marchingCubeVertices[configindex], std::back_inserter(mesh.positions), [offset, sideHeight, offsety, pushIn](const auto& v) {
            auto newv = PxVec3(pushIn, pushIn, pushIn) + v*(1.f- pushIn);
            newv.y *= sideHeight;
            return newv + offset + PxVec3(0.5f, offsety+0.5f, 0.5f);
        });

        std::ranges::transform(marchingCubeIndices[configindex], std::back_inserter(mesh.indices), [&index](const auto& i) {
            return index + i;
        });

        index += marchingCubeVertices[configindex].size();

        float isGrass = grasstypes.test(material) ? 1.0f : 0.0f;



    auto vals = getBlockAmbientLightVals(blockAndAmbBright);



    // Calculate occlusion for each vertex of the face
    int occlusion[4] = {0,0,0,0};
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
    // bool isTransparent = transparents.test(blockType);
    // if (isTransparent) {
    //     mesh.tbrightness.insert(mesh.tbrightness.end(), {
    //         packonocclbits(occlusion[0], newbaab), isGrass,
    //     });
    // } else {

    static auto getDirectionFactor = [](glm::vec3 direction) {
        return 1.0 - ( glm::dot(glm::normalize(direction), glm::vec3(0.0, -1.0, 0.0)) * 0.5 + 0.5);
    };

    std::vector<float> brightnesses(marchingCubeVertices[configindex].size() * 2);
    std::vector<glm::vec2> texcoordsfr(marchingCubeVertices[configindex].size());

    static auto calculateTriangleNormal = [](const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
        glm::vec3 ab = b - a;
        glm::vec3 ac = c - a;
        glm::vec3 normal = glm::cross(ab, ac);
        return glm::normalize(normal);
    };

        auto uvoffset = glm::vec2(uvoffsetx, uvoffsety);

    for (int i = 0; i < marchingCubeIndices[configindex].size(); i+=3)
    {
        auto idx1 = marchingCubeIndices[configindex][i];
        auto idx2 = marchingCubeIndices[configindex][i+1];
        auto idx3 = marchingCubeIndices[configindex][i+2];

        auto tri1 = marchingCubeVertices[configindex][idx1];
        auto tri2 = marchingCubeVertices[configindex][idx2];
        auto tri3 = marchingCubeVertices[configindex][idx3];

        auto normal = calculateTriangleNormal(glm::vec3(tri1.x, tri1.y, tri1.z),
            glm::vec3(tri2.x, tri2.y, tri2.z), glm::vec3(tri3.x, tri3.y, tri3.z));

        auto factor = (float)getDirectionFactor(normal);

        brightnesses[idx1*2 + 0] = packonocclbits(occlusion[0], getBlockAmbientLightVal(ColorPack(vals.first) * factor, ColorPack(vals.second) * factor)
);
        brightnesses[idx1*2 + 1] = isGrass;

        brightnesses[idx2*2 + 0] = packonocclbits(occlusion[0], getBlockAmbientLightVal(ColorPack(vals.first) * factor, ColorPack(vals.second) * factor)
);
        brightnesses[idx2*2 + 1] = isGrass;


        brightnesses[idx3*2 + 0] = packonocclbits(occlusion[0], getBlockAmbientLightVal(ColorPack(vals.first) * factor, ColorPack(vals.second) * factor)
);
        brightnesses[idx3*2 + 1] = isGrass;

        auto absNormal = glm::abs(normal);

        // Project triangle to 2D and use its bounding box
        auto minX = std::min({tri1.x, tri2.x, tri3.x});
        auto maxX = std::max({tri1.x, tri2.x, tri3.x});
        auto minZ = std::min({tri1.z, tri2.z, tri3.z});
        auto maxZ = std::max({tri1.z, tri2.z, tri3.z});

        // Handle degenerate cases where triangle is a line
        float rangeX = maxX - minX;
        float rangeZ = maxZ - minZ;
        if (rangeX == 0) rangeX = 1.0f;
        if (rangeZ == 0) rangeZ = 1.0f;

        auto tw = textureWidth - (onePixel*2.f);

        texcoordsfr[idx1] = uvoffset + glm::vec2(onePixel, -onePixel) + glm::vec2((tri1.x - minX)/rangeX * tw, -(tri1.z - minZ)/rangeZ * tw);
        texcoordsfr[idx2] = uvoffset  + glm::vec2(onePixel, -onePixel)+ glm::vec2((tri2.x - minX)/rangeX * tw, -(tri2.z - minZ)/rangeZ * tw);
        texcoordsfr[idx3] = uvoffset + glm::vec2(onePixel, -onePixel) + glm::vec2((tri3.x - minX)/rangeX * tw, -(tri3.z - minZ)/rangeZ * tw);

    }
    mesh.brightness.insert(mesh.brightness.end(), brightnesses.begin(), brightnesses.end());
        mesh.texcoords.insert(mesh.texcoords.end(), texcoordsfr.begin(), texcoordsfr.end());



    }


}

#endif










glm::vec3 calculateTriangleNormal(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

// This function calculates and adds ambient occlusion brightness values for a marcher
void calculateMarcherAmbientOcclusion(const IntTup& blockPos, uint8_t configindex, BlockType blockType, UsableMesh& mesh, float
                               blockandambbright);



void genCGLBuffers();

std::array<IntTup, 3> getAdjacentOffsets(Side side, int vertexIndex);
void calculateAmbientOcclusion(const IntTup& blockPos, Side side, World* world, bool locked, BlockType blockType, UsableMesh& mesh, float
                               blockandambbright);



template<bool queueimplics = true, bool useraskedfor = false>
UsableMesh fromChunk(const TwoIntTup& spot, World* world, bool locked, bool light);

UsableMesh fromChunk(const TwoIntTup& spot, World* world, bool light = false);

UsableMesh fromChunkLocked(const TwoIntTup& spot, World* world, bool light = false);


struct UsedChunkInfo
{
    uint16_t chunkIndex : 14 = 0;
    bool dontRepurpose : 1 = false;
    bool confirmedByMainThread : 1 = false;
    explicit UsedChunkInfo(size_t index) : chunkIndex(index) {}
};



struct ReadyToDrawChunkInfo
{
    uint16_t chunkIndex : 14 = 0;
    float timeBeenRendered = 0.0f;
    explicit ReadyToDrawChunkInfo(size_t index) : chunkIndex(index) {}
    ReadyToDrawChunkInfo(size_t index, float tbr) : chunkIndex(index), timeBeenRendered(tbr) {}
};



extern std::atomic<int> NUM_THREADS_RUNNING;

extern LightMapType lightmap;
extern LightMapType ambientlightmap;
extern std::shared_mutex lightmapMutex;
extern tbb::concurrent_hash_map<TwoIntTup, bool, TwoIntTupHashCompare> litChunks;

class WorldRenderer {
public:
    static constexpr int chunkSize = 16;
    static constexpr int maxRenderDistance = 64;
    static constexpr int defaultRenderDistance = 17; 
    int currentRenderDistance = defaultRenderDistance;
    static constexpr int maxChunks = ((maxRenderDistance *2) * (maxRenderDistance *2));
    int lastMaxChunks = maxChunks;


    int currentMinDistance() const
    {
        return std::round((float)currentRenderDistance * 1.4f) + 1;
    }
    int currentMaxChunks() const
    {
        int r = currentMinDistance();
        return std::round(J_PI * r * r);
    }



    std::vector<SmallChunkGLInfo> chunkPool = {};

    std::atomic<size_t> chunkPoolSize = {0};

    RebuildQueue rebuildQueue;
    std::thread rebuildThread;
    std::thread chunkWorker;
    std::atomic<int> rebuildThreadRunning = false;
    std::atomic<int> meshBuildingThreadRunning = false;



    //For the user
    std::mutex bufferMutex = {};
    std::condition_variable bufferCV = {};

    //For the mbt
    std::mutex mbtBufferMutex = {};
    std::condition_variable mbtBufferCV = {};


    void notifyBufferFreed() {
        bufferCV.notify_one();
    }

    ///Only for Main Thread access. The main thread will make changes to this as it buffers incoming geometry, so they will only be on this once theyre officially ready to be drawn.
    ///The main thread will also use this as its list of what chunks to draw
    boost::unordered_flat_map<TwoIntTup, ReadyToDrawChunkInfo, TwoIntTupHash, std::equal_to<>> activeChunks = {};

    ///Only for Mesh Building thread access. Its internal record of what spots it has generated & sent out the geometry for.
    boost::unordered_map<TwoIntTup, UsedChunkInfo, TwoIntTupHash, std::equal_to<>> mbtActiveChunks = {};

    ///Chunks that have had their terrain features generated already.
    tbb::concurrent_hash_map<TwoIntTup, bool, TwoIntTupHashCompare> generatedChunks;



    static std::optional<std::vector<SpawnGuy>>
    generateChunk(World* world, const TwoIntTup& chunkSpot,
                  std::unordered_set<TwoIntTup, TwoIntTupHash>* implicatedChunks = nullptr, bool
                  doSpawns = false);

    WorldRenderer()
    {
        mbtActiveChunks.reserve(maxChunks);
        activeChunks.reserve(maxChunks);
        chunkPool.resize(maxChunks);
        //Generate them all at the beginning so we know the buffer names for sure
        std::ranges::for_each(std::views::iota(0, maxChunks), [](auto) {
            genCGLBuffers();
        });
    }

    void clearInFlightMeshUpdates();

    void setRenderDistance(int newone, jl::Camera* camera, World* world)
    {

        stopThreads();

        lastMaxChunks = currentMaxChunks();
        currentRenderDistance = newone;

        if (lastMaxChunks > currentMaxChunks())
        {
            for (int i = lastMaxChunks-1; i > currentMaxChunks(); i--)
            {
                auto sgl = SmallChunkGLInfo{};
                auto mesh = UsableMesh{};
                modifyOrInitializeChunkIndex(i, sgl, mesh);
            }
        }

        clearInFlightMeshUpdates();

        chunkPoolSize.store(0, std::memory_order_release);
        activeChunks.clear();
        mbtActiveChunks.clear();

        lightmapMutex.lock();
            lightmap.lm.clear();
            ambientlightmap.lm.clear();
        lightmapMutex.unlock();





        launchThreads(camera, world);

    }


    ///A limited list of atomic "Change Buffers" that the mesh building thread can reserve and write to, and the main thread will "check its mail", do the necessary GL calls, and re-free the Change Buffers
    ///by adding its index to freeChangeBuffers.
    std::array<ChangeBuffer, 32> changeBuffers = {};
    ///One way queue, from main thread to mesh building thread, to notify of freed Change Buffers
    boost::lockfree::spsc_queue<size_t, boost::lockfree::capacity<32>> freedChangeBuffers = {};

    boost::lockfree::spsc_queue<TwoIntTup, boost::lockfree::capacity<32>> removeTheseFromMBTAC = {};

    ///A limited list of atomic "Change Buffers" that the mesh building thread can reserve and write to, and the main thread will "check its mail", do the necessary GL calls, and re-free the Change Buffers
    ///by adding its index to freeChangeBuffers.
    std::array<ChangeBuffer, 16> userChangeMeshBuffers = {};
    ///One way queue, from main thread to mesh building thread, to notify of freed Change Buffers
    boost::lockfree::spsc_queue<size_t, boost::lockfree::capacity<16>> freedUserChangeMeshBuffers = {};

    ///After being added to mbtActiveChunks, we await a confirmation back in this before we know we can reuse that chunk again
    ///One way queue, from main thread to mesh building thread, to notify of mbtActiveChunks entries that have been confirmed/entered into activeChunks.
    boost::lockfree::spsc_queue<TwoIntTup, boost::lockfree::capacity<5>> confirmedActiveChunksQueue = {};



    //A way for the rebuild thread to ask the main thread "What chunks are active from this area?" so that the main thread can submit rebuild requests for them
    boost::lockfree::spsc_queue<BlockArea, boost::lockfree::capacity<5>> rebuildToMainAreaNotifications = {};


    double getDewyFogFactor(float temperature_noise, float humidity_noise);
    void mainThreadDraw(const jl::Camera* playerCamera, GLuint shader, WorldGenMethod* worldGenMethod, float deltaTime, bool actuallyDraw);
    void meshBuildCoroutine(jl::Camera* playerCamera, World* world);

    ///Add a chunk gl info with the vao == 0 (not generated yet). Calling modifyOrInitializeDrawInstructions with it and geometry will initialize it.
    ///Returns the index in chunkPool of the new chunk.
    size_t addUninitializedChunkBuffer()
    {
        size_t val = chunkPoolSize.load(std::memory_order_acquire);
        size_t ret = val;
        chunkPoolSize.store(val+1, std::memory_order_release);
        return ret;
    }

    void launchThreads(jl::Camera* camera, World* world)
    {

        meshBuildingThreadRunning = true;
        chunkWorker = std::thread(&WorldRenderer::meshBuildCoroutine, this,
            camera, world);
        //chunkWorker.detach();
        rebuildQueue.shouldExit = false;
        rebuildThreadRunning = true;
        rebuildThread = std::thread(&WorldRenderer::rebuildThreadFunction,
            this,
            world
        );
        //rebuildThread.detach();
    }

    void stopThreads()
    {
        rebuildThreadRunning = false;
        meshBuildingThreadRunning = false;
        chunkWorker.join();
        rebuildQueue.signalExit();
        rebuildThread.join();
    }

    void rebuildThreadFunction(World* world);

    void requestBlockBulkPlaceFromMainThread(const BlockArea& area)
    {
        rebuildQueue.push(ChunkRebuildRequest(area
                ));
    }

    void requestVoxelModelPlaceFromMainThread(const PlacedVoxModel& vox)
    {
        rebuildQueue.push(ChunkRebuildRequest(vox));
    }

    void requestChunkRebuildFromMainThread(const IntTup& spot, std::optional<BlockType> changeTo = std::nullopt, bool rebuild = true, const glm::vec3& pp = glm::vec3(0.f), bool lightpass = false, bool queuelightimplic = true)
    {
        //TwoIntTup version of spot
        auto titspot = TwoIntTup(spot.x, spot.z);
        auto chunkspot = WorldRenderer::stupidWorldRendererWorldToChunkPos(titspot);
        if (activeChunks.contains(chunkspot))
        {
            //std::cout << "Requesting rebuild for spot: " << chunkspot.x << " " << chunkspot.z << std::endl;
            if(changeTo != std::nullopt)
            {
                auto req = ChunkRebuildRequest(
                    chunkspot,
                    activeChunks.at(chunkspot).chunkIndex,
                    true,
                    spot,
                    changeTo.value(),
                    rebuild,
                    pp
                );
                req.doLightPass = lightpass;
                req.queueLightpassImplicated = queuelightimplic;
                req.useraskedfor = true;
                rebuildQueue.push(req);
            } else
            {
                auto req = ChunkRebuildRequest(
                    chunkspot,
                    activeChunks.at(chunkspot).chunkIndex,
                    true
                );
                req.doLightPass = lightpass;
                req.queueLightpassImplicated = queuelightimplic;
                req.useraskedfor = true;
                rebuildQueue.push(req);
            }

        } else
        {

        }
    }


    void requestChunkSpotRebuildFromMainThread(const TwoIntTup& chunkspot, bool priority = true, bool light = false)
    {
        if (activeChunks.contains(chunkspot))
        {
            //std::cout << "Requesting rebuild for spot: " << chunkspot.x << " " << chunkspot.z << std::endl;
            if (light)
            {
                auto cbr = ChunkRebuildRequest(
                    chunkspot,
                    activeChunks.at(chunkspot).chunkIndex,
                    priority,
                    true
                );
                cbr.useraskedfor = priority;
                rebuildQueue.push(cbr);
            } else
            {
                auto cbr = ChunkRebuildRequest(
                    chunkspot,
                    activeChunks.at(chunkspot).chunkIndex,
                    priority
                );
                cbr.useraskedfor = priority;
                rebuildQueue.push(cbr);
            }



        } else
        {

        }
    }

    static TwoIntTup stupidWorldRendererWorldToChunkPos(const TwoIntTup& worldPos)
    {
        return worldToChunkPos(worldPos);
    }

};



#endif //WORLDRENDERER_H
