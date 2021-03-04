## Engine Outline



The [Main thread](../Src/Main.cpp) for the object should look something like this
* Init Objects(init opengl, create window, setup world and the like, and init any important settings)
* MainLoop():
  * Poll for input events
  * Check for deletable objects (Chunks/Entities out of range)
  * Run physics tick
  * Pre render all shadows/reflections
  * Render all viewable entities
  * Render all opaque world objects
  * Render all translucent world objects
* Cleanup Objects (terminate opengl)


The main loop acts as a controller for the World Object 
Main is not responsible for any memory management, all memory management should be done dynamically through the World object

### [World.h](../Src/Include/World.h)
World encapsulates the game world, and is responsible for all memory management of entities and chunks, as well as some global draw settings and initilaizing shaders

Using glm::ivec3 as the key, all chunks in memory can be found in the chunks object for fast lookup time.

All loaded entities can be found in the entityList array 

Similarily, the pointlight list keeps track of all relevant pointlights for the next draw frame, clears each frame

Furthermore there all miscellaneous structs for global illumination, as well as the objects keep track of of the world physics
   
### [Chunk.h](../Src/Include/Chunk.h)

Chunks contain 32x32x32 arrays of voxels, they are responsible for containing the chunk data, as well as handling the multithreading

Chunks contain 2 ASYNC tasks, the mesh task, managed by the meshTask future contained in the chunk, and the chunk generation/buildTask, managed by the shared future meshTask

Generation is a WRITE only task, the goal is to create a chunk generation algorithm that eliminates the need to read any neighbouring chunks data, 
this allows for safe multithreading, as a chunk can all be read from once the generation is completed

Building mesh is a READ only task, using the weak pointers stored in each chunk of its surrounding relevant neighbours the chunk is able to complete the meshing task by 
locking and reading neighbouring chunks (for as long as they are free from the chunk generation task)

For performance reasons, it is the users job to check that getVoxel is safe to you (aka the chunk has finished generating)

#### Chunk Generation
Chunk generation, found in [Chunk.cpp](../Src/Chunk.cpp) generateChunk function, implements noise from [Auburns FastNoise 2 library](https://github.com/Auburn/FastNoise2/)
(Although a much slower and drepreciated attempt can still be found in [Perlin.h](../Include/perlin.h), simd commands are difficult to compete with)
The current height map uses Open Simplex 2 with some fractalization to create a very interesting heightmap

The basestring for use in the FastNoise library is: ``EQADAAAAAAAAQBAAAAAAPxkAEwCamRk/DQAEAAAAAAAgQAkAAAAAAD8AAAAAPwEEAAAAAAAAAEBAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAgD8AAAAAPwAAAAAA``

3d noise will be used for cave generation, however not yet implemented

#### Chunk Meshing
Chunk meshing uses the following 3 tables found similarily in [Chunk.h](../Src/Include/Chunk.h)

```C++
//Table for the 7 chunk neighbours in the postive x,y,z directions respectively
static const glm::ivec3 chunkNeighboursTable[7] = { {1,1,1},{0,1,1},{1,0,1},{1,1,0},{1,0,0},{0,1,0},{0,0,1}, };

//Table for the 8 corners of a cube
static const glm::ivec3 cubeCorners[8] = { {0,0,0},{1,0,0},{0,1,0},{1,1,0}, {0,0,1},{1,0,1},{0,1,1},{1,1,1} };

//Table for the 12 edges of a cube with respect to the corners
static const std::pair<int, int> cubeEdges[12] = { {0,1}, {0,2}, {1,3}, {2,3}, {4,5}, {4,6}, {5,7}, {6,7}, {0,4}, {1,5}, {2,6}, {3,7} };
```


Chunk meshing is implemented us a Surface Net algorithm. Each and every voxel gives a signed distance value, negative values are inside objects(dirt), 
positive values are outside objects(air), and attempts to build the surface were those values would meet at 0.

The first step to the meshing algorithm is to build an array of all relevant voxels. The algorithm is FORWARD only and begins at the BOTTOM FRONT LEFT of a cube.
As such the algorithm does not need to know any voxels in the negative.  Ex: a voxel at (0,0,0) only accounts for the voxels 
at(0,0,1),(0,1,0),(1,1,0),(0,0,1),(0,0,1),(0,1,1),(1,1,1) to determine its face(which is are the same corners as defined in the cube table)

Therefore, in order to find all relevant voxels, we require a reference to all positive neighbours, which can be found in chunks chunkNeighbours array
In order to calculate all relevant faces, we are required to know N + 1 surface points, therefore to know all surfacepoint we need to know N + 2 voxels;

getAllRelevantVoxels() returns a N+2 cube array of all relevant voxels for further use in the algorithm

The second step is to find all surface points, this is done by averaging the value of all 12 edges of the cube, first by calculating the alpha between the two values
```
alpha = v1/(v1-v2) where sign(v1) != sign(v2)
```
and using that alpha to determine the relevant position of that edge. Repeat for every edge with differeing signs, then the surface point for the given chunk can be determined

With the array of surfacepoints, we can now calculate the corresponding tris for each vertice.

Each voxel has 6 possible faces it can create, 2 in each positive axis direction. A face exists if and only if the signs of the voxel are differing. 
We check cube edges (3,7) for  the Z direction, (5,7) for the Y direction, and (6,7) for the Z direction. For each differing sign we take the 6 indices of the relevant
tris and push them back.

Once all tris are obtained we can then send this list of vertices and indices to the gpu for graphics rendering, as well as to the CPU for use in the physics mesh

#### Vertices
Defined as:
```c++
struct ChunkVertex {
	glm::vec3 pos;
	glm::vec3 norm;
	uint32_t color;
};
```

Vertices use single precision floats for the (x,y,z) coordinates, as well as there normal values, and used a compressed 32 bit color for its color value

#### Entities
Found in [Entity.h](../Include/Entity.h) are kept track of globally by the World object. While entities can be created
anywhere, it is the Worlds job to keep track of them and delete them if need be. Memory management is not done by the user at all, simply pass a 
new entity to world.addEntity() and the world will keep track of it