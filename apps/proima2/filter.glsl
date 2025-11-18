#version 430

layout(std430, binding = 0) readonly buffer entreeBuffer
{
    int entree[];
};

layout(std430, binding = 1) buffer sortieBuffer
{
    uint n;
    int sortie[];
};

uniform int min;

layout(local_size_x = 256) in;
void main()
{
    //    uint id = gl_GlobalInvocationID.x;
    //    if (id == 0) {
    //        n = 0;
    //    }

    if (id < entree.length()) {
        if (entree[id] >= min) {
            uint pos = atomicAdd(n, 1);
            sortie[pos] = entree[id];
        }
    }
}
