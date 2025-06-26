## Function names convention

**init** - sets up initial state and prepares something for use
**cleanup** - resets state and prepares something for reuse or destruction

* Commonly used for systems, contexts, temporary data, etc.


**create** - allocates memory and constructs a new resource or object
**delete** - deallocates memory and destructs an existing resource or object

* Commonly used for buffers, textures, meshes, handles, etc.


**spawn** - suggests bringing something into existence in the game world
**destroy** - removes it from existence

* Commonly used for entities, objects, particles, etc.
