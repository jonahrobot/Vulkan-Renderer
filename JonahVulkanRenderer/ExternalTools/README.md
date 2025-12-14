# How to parse Activision Caldera map.  

Source: https://github.com/Activision/caldera  
Uses: https://github.com/PixarAnimationStudios/OpenUSD  

# Mesh Data Example  

Source path: /world/hotel_01/geo/hotel_terrasse/...  

> misc_model_207 (XFORM)  
---- barrier_wooden_fence_01_mp (XFORM)  
-------- geo (Scope)  
------------ barrier_wooden_fence_01_mp_LOD_0 (Mesh)  

misc_model_207 (XFORM) holds rotation, scale, and translation data for the children.

barrier_wooden_fence_01_mp (XFORM) holds render metadata like (LOD of the child mesh, and if the mesh is instanceable). We could adjust a mesh's LOD here and get different vertex and index data from the child.    

geo (Scope) is simply a folder holding our geometry data. Some XFORMS also hold folders for audio and unlinked materials.  

barrier_wooden_fence_01_mp_LOD_0 (Mesh) holds all the data we need on the current model, unscaled to the parent XFORM adjustments. Vertex data is held in the "points" attribute, and index data is in the "faceVertexIndices" attribute.  

# Shared data

Another thing is that other models can hold the same data, for example:  

> misc_model_208 (XFORM)  
---- barrier_wooden_fence_01_mp (XFORM)  
-------- geo (Scope)  
------------ barrier_wooden_fence_01_mp_LOD_0 (Mesh)  

The Mesh data is the same as the above model. However, the LOD could be different, and the translations from the root XFORM may also differ. A LOD change would affect the mesh data.  

# Instancing

Repeated prims are actually proxy copies of one hidden prim storing the vertex and index data of the proxy copies.  

We can tell if a prim model is a proxy copy if the XFORM wrapping our model's data, like barrier_wooden_fence_01_mp, has the attribute instanceable set to true.  

If this is true, we can get the hidden proxy data instead by doing prim.GetPrototype()!  

We can then store this data, and the next time we get this same prototype, we can simply mark it as a reference to this cached prototype.  

Sources: 

https://docs.omniverse.nvidia.com/usd/latest/learn-openusd/independent/modularity-guide/instancing.html  

https://openusd.org/release/api/_usd__page__scenegraph_instancing.html#Usd_ScenegraphInstancing_TargetsAndConnections  
