import struct,argparse

"""
(dev.mp) Binary Format

Header
0x00  uint16    Verification bytes (0x4D50) (MP in Hex)
0x02  uint32    # of Objects
0x06  uint32[]  object pointers
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
"""

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        "-v", "--verbose",
        action="store_true",
        help="Enable verbose output"
    )

    opts = parser.parse_args()

    with open("dev.mp", "rb") as f:
        verification_bytes = struct.unpack("<h", f.read(2))[0]
        object_count = struct.unpack("<I", f.read(4))[0]
        pointers = struct.unpack(f"<{object_count}I", f.read(object_count * 4))

        print(f"Verification bytes: {hex(verification_bytes)}")

        if verification_bytes != 0x4D50:
            print("Verification failed, first 4 bytes of .mp should be 0x4D50.")
            return

        print(f"Object count: {object_count}")
        print(f"Pointers: {pointers}")

        for x in range(0, object_count):

            if opts.verbose:
                print(f"Object {x} printout --")

            num_vertices = struct.unpack("<I", f.read(4))[0]
            num_indices = struct.unpack("<I", f.read(4))[0]
            num_normals = struct.unpack("<I", f.read(4))[0]
            num_instances = struct.unpack("<I", f.read(4))[0]

            if opts.verbose:
                print(f"""Vertices: {num_vertices}, Indices: {num_indices}, 
                      Normals: {num_normals}, Instances: {num_instances}""")
            else:
                print(f"""Object {x}: Vertices: {num_vertices}, Indices: {num_indices}, 
                      Normals: {num_normals}, Instances: {num_instances}""")

            vertices = struct.unpack(f"<{num_vertices * 3}f", f.read(num_vertices * 3 * 4))
            indices = struct.unpack(f"<{num_indices}H", f.read(num_indices * 2))
            normals = struct.unpack(f"<{num_normals * 3}f", f.read(num_normals * 3 * 4))

            if opts.verbose:
                print(f"Vertices: {vertices}")
                print(f"Indices: {indices}")
                print(f"Normals: {normals}")

            instance_matrices = []

            if opts.verbose:
                print("Matrices:")

            for y in range(0, num_instances):
                matrix = struct.unpack("<16f", f.read(16 * 4))
                instance_matrices.append(matrix)
                if opts.verbose:
                    print(f"Matrix {y}: {matrix}")

        print("Finished printing objects")
        print(f"Object count: {object_count}")
        print(f"Pointers: {pointers}")


if __name__ == "__main__":
    main()