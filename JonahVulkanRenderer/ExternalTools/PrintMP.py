import struct

"""
(dev.mp) Binary Format

Header
0x00  uint32    # of Objects
0x04  uint32[]  object pointers
...   Object[]  object data

Object
0x00  uint32      # of Vertices ( [x,y,z] = 1 )
0x04  uint32      # of Indices
0x08  uint32      # of Normals
0x0C  uint32      # of Instances ( 1 Mat4 per instance )
...   float[]     Vertices [x,y,z,x,y,z,...]
...   uint16[]    Indices  [0,1,2,3,...]
...   float[]     Normals  [x,y,z,x,y,z,...]
...   mat4[]      Instance Matrices (row-major order) (mat4 = float x 16)

(Little Endian)
"""

def main():
    with open("dev.mp", "rb") as f:
        object_count = struct.unpack("<I", f.read(4))[0]
        pointers = struct.unpack(f"<{object_count}I", f.read(object_count * 4))

        print(f"Object count: {object_count}")

        for x in range(0, object_count):
            num_vertices = struct.unpack("<I", f.read(4))[0]
            num_indices = struct.unpack("<I", f.read(4))[0]
            num_normals = struct.unpack("<I", f.read(4))[0]
            num_instances = struct.unpack("<I", f.read(4))[0]

            vertices = struct.unpack(f"<{num_vertices * 3}f", f.read(num_vertices * 3 * 4))
            indices = struct.unpack(f"<{num_indices}H", f.read(num_indices * 2))
            normals = struct.unpack(f"<{num_normals}f", f.read(num_normals * 3 * 4))

            instance_matrices = []

            for y in range(0, num_instances):
                matrix = struct.unpack("<16f", f.read(16 * 4))
                instance_matrices.append(matrix)



if __name__ == "__main__":
    main()