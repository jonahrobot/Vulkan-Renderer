# Two sections
## Table of Contents
*   [Using Python Scripts](#using-python-scripts)
*   [MP File type](#mp-file-type)
*   [How to parse the Activision Caldera map](#how-to-parse-the-activision-caldera-map)

## Using Python Scripts

*  ```ParseUSD.py --f [path to .usd file] ```
*  ```PrintMP.py [-v for verbose printout]```

Example call: ```python ./ParseUSD.py --f "C:\map\caldera-main\map_source\prefabs\br\wz_vg\mp_wz_island\commercial\hotel_01.usd"```

This will output a dev.mp file. This is a binary representation of all the scene geometry data and is structured to support parallel processing of its contents.

To see the dev.mp file contents in a readable format, use the included PrintMP.py command. Use -v for verbose output of each mesh's vertex, index, and normal data.

Example call: ```python ./PrintMP.py dev.mp```

Note: ParseUSD.py is designed around the COD Caldera map OpenUSD file, but it can work with any other by removing the population_mask on lines 49-51. To use it, call ParseUSD.py with the file path to the USD scene you want to parse. 

## MP File Type

The Vulkan renderer takes these .mp files and displays them in 3D space.

(dev.mp) Binary Format
```
Header
0x00  uint32    # of Objects
0x04  uint32[]  object pointers
...   Object[]  object data

Object
0x00  uint32      # of Vertices ( [x,y,z] = 1 )
0x04  uint32      # of Indices
0x08  uint32      # of Normals  ( [x,y,z] = 1 )
0x0C  uint32      # of Instances ( 1 Mat4 per instance )
...   float[]     Vertices [x,y,z,x,y,z,...]
...   uint16[]    Indices  [0,1,2,3,...]
...   float[]     Normals  [x,y,z,x,y,z,...]
...   mat4[]      Instance Matrices (row-major order) (mat4 = float x 16)

(Little Endian)
```

Having pointers to each object allows multiple threads to parse mesh data synchronously without data conflicts and minor cache invalidations. 

## How to parse the Activision Caldera map.  

Source: https://github.com/Activision/caldera  
Uses: https://github.com/PixarAnimationStudios/OpenUSD  

### Mesh Data Example  

Source path: /world/hotel_01/geo/hotel_terrasse/...  

> misc_model_207 (XFORM)  
---- barrier_wooden_fence_01_mp (XFORM)  
-------- geo (Scope)  
------------ barrier_wooden_fence_01_mp_LOD_0 (Mesh)  

misc_model_207 (XFORM) holds rotation, scale, and translation data for the children.

barrier_wooden_fence_01_mp (XFORM) holds render metadata like (LOD of the child mesh, and if the mesh is instanceable). We could adjust a mesh's LOD here and get different vertex and index data from the child.    

geo (Scope) is simply a folder holding our geometry data. Some XFORMS also hold folders for audio and unlinked materials.  

barrier_wooden_fence_01_mp_LOD_0 (Mesh) holds all the data we need on the current model, unscaled to the parent XFORM adjustments. Vertex data is held in the "points" attribute, and index data is in the "faceVertexIndices" attribute.  

### Shared data

Another thing is that other models can hold the same data, for example:  

> misc_model_208 (XFORM)  
---- barrier_wooden_fence_01_mp (XFORM)  
-------- geo (Scope)  
------------ barrier_wooden_fence_01_mp_LOD_0 (Mesh)  

The Mesh data is the same as the above model. However, the LOD could be different, and the translations from the root XFORM may also differ. A LOD change would affect the mesh data.  

### Instancing

Repeated prims are actually proxy copies of one hidden prim storing the vertex and index data of the proxy copies.  

We can tell if a prim model is a proxy copy if the XFORM wrapping our model's data, like barrier_wooden_fence_01_mp, has the attribute instanceable set to true.  

If this is true, we can get the hidden proxy data instead by doing prim.GetPrototype()!  

We can then store this data, and the next time we get this same prototype, we can simply mark it as a reference to this cached prototype.  

Sources: 

https://docs.omniverse.nvidia.com/usd/latest/learn-openusd/independent/modularity-guide/instancing.html  

https://openusd.org/release/api/_usd__page__scenegraph_instancing.html#Usd_ScenegraphInstancing_TargetsAndConnections  
