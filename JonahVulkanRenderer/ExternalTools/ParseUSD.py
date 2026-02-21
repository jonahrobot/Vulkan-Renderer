# Requires usd-core Python Package.

from pxr import Usd, UsdGeom, Sdf, Gf, UsdSkel
import argparse, struct
from tqdm import tqdm
from pathlib import Path

def find_parent_with_skeleton(mesh):
    p = mesh
    while p and p.IsValid():
        if p.IsA(UsdSkel.Root):
            return p, True
        p = p.GetParent()
    return None, False


def find_binding_matching_mesh(bindings, mesh):
    for b in bindings:
        for target in b.GetSkinningTargets():
            if target.GetPrim() == mesh:
                return b, True
    return None, False

def parse_scene(filepath, scale):

    scene_data = {"models": {}}
    total_models = 0

    # Open the USD stage from the specified file
    stage: Usd.Stage = Usd.Stage.Open(filepath)

    scale_constant = scale

    total_prims = 0
    print("Finding prim count...")

    for prim in Usd.PrimRange(stage.GetPseudoRoot(), Usd.TraverseInstanceProxies()):

        if not prim.IsA(UsdGeom.Mesh):
            continue

        purpose = UsdGeom.Imageable(prim).GetPurposeAttr().Get()
        if purpose == "guide":
            continue

        total_prims += 1

    pbar = tqdm(
        total=total_prims,
        desc="Parsing primitives.",
        unit="primitives parsed"
    )

    current_index = 0

    # Traverse through each prim in the stage
    for prim in Usd.PrimRange(stage.GetPseudoRoot(), Usd.TraverseInstanceProxies()):
        if prim.IsA(UsdGeom.Mesh):
            purpose = UsdGeom.Imageable(prim).GetPurposeAttr().Get()

            if purpose == "guide":
                continue

            # Check if any skeleton maps to our prim
            p, has_skel = find_parent_with_skeleton(prim)

            if has_skel:
                skel_root = UsdSkel.Root(p)
                cache = UsdSkel.Cache()
                cache.Populate(skel_root, Usd.TraverseInstanceProxies())
                bindings = cache.ComputeSkelBindings(skel_root, Usd.TraverseInstanceProxies())

                binding, found_binding = find_binding_matching_mesh(bindings, prim)

                # Currently, we do not handle bone rigged meshes, so skip importing these.
                if found_binding:
                    continue

            time = Usd.TimeCode.Default()

            points = UsdGeom.Mesh(prim).GetPointsAttr().Get()
            indices = UsdGeom.Mesh(prim).GetFaceVertexIndicesAttr().Get()
            attr = prim.GetAttribute("primvars:normals")
            normals = []
            normal_indices = []
            if attr and attr.IsValid():
                normals = UsdGeom.Primvar(attr).Get(time)
                normal_indices = UsdGeom.Primvar(attr).GetIndicesAttr().Get(time)

            model_hash = prim.GetName() + "_" + str(len(points)) + "_" + str(len(indices))
            xform = UsdGeom.Xformable(prim)
            world_transform: Gf.Matrix4d = xform.ComputeLocalToWorldTransform(time)

            transform_write = [
                world_transform[0][0], world_transform[0][1], world_transform[0][2], world_transform[0][3],
                world_transform[1][0], world_transform[1][1], world_transform[1][2], world_transform[1][3],
                world_transform[2][0], world_transform[2][1], world_transform[2][2], world_transform[2][3],
                world_transform[3][0] * scale_constant, world_transform[3][1] * scale_constant,
                world_transform[3][2] * scale_constant, world_transform[3][3]
            ]
            if model_hash in scene_data["models"]:
                scene_data["models"][model_hash]["instance_count"] += 1
                scene_data["models"][model_hash]["instances"].append(transform_write)
                current_index += 1
            else:

                face_counts = UsdGeom.Mesh(prim).GetFaceVertexCountsAttr().Get()
                points_float = []
                indices_float = []
                normal_float = []
                normal_indices_float = []
                points_count = 0
                indices_count = 0
                normals_count = 0

                for x in points:
                    points_float.append(x[0] * scale_constant)
                    points_float.append(x[1] * scale_constant)
                    points_float.append(x[2] * scale_constant)
                    points_count += 1

                for x in normals:
                    normal_float.append(x[0])
                    normal_float.append(x[1])
                    normal_float.append(x[2])
                    normals_count += 1

                for x in normal_indices:
                    normal_indices_float.append(x)

                # OpenUSD models support non-tri mesh faces
                # So to render them we must triangulate them
                # Below is a convenient way to do so for any N-Gon! (In Counter-Clockwise Rotation)
                counter = 0
                for x in face_counts:
                    for i in range(x-2):
                        indices_float.append(indices[counter])
                        indices_float.append(indices[counter + i + 1])
                        indices_float.append(indices[counter + i + 2])
                        indices_count += 3

                    counter += x

                scene_data["models"][model_hash] = {
                    "vertices": points_float,
                    "vertex_count": points_count,
                    "indices": indices_float,
                    "indices_count": indices_count,
                    "normals": normal_float,
                    "normals_count": normals_count,
                    "normal_indices": normal_indices_float,
                    "instance_count": 1,
                    "instances": [transform_write]
                }

                total_models += 1
                current_index += 1

            pbar.update(1)
            pbar.set_postfix_str(f"current model: {current_index}")

    # Pack data into buffer
    name = Path(filepath).stem

    with open(f"{name}.mp", "wb") as f:

        # Add Verification bytes (0x4D50) (MP in Hex)
        f.write(struct.pack('<h', 0x4D50))

        # Add object count
        f.write(struct.pack('<I', total_models))

        # Add object pointers
        offset = 0

        for x in scene_data["models"]:
            f.write(struct.pack('<I', offset))
            object_header = 16
            vertices = 4 * 3 * scene_data["models"][x]["vertex_count"]      # float * 3 per vertex
            indices = 2 * scene_data["models"][x]["indices_count"]          # uint16t per index
            normals = 4 * 3 * scene_data["models"][x]["normals_count"]      # float * 3 per normal
            instances = 4 * 16 * scene_data["models"][x]["instance_count"]  # float * 16 per matrix

            offset += object_header + vertices + indices + normals + instances

        # Add objects
        for x in scene_data["models"]:

            # Write object header
            f.write(struct.pack(
                '<4I',
                scene_data["models"][x]["vertex_count"],
                scene_data["models"][x]["indices_count"],
                scene_data["models"][x]["normals_count"],
                scene_data["models"][x]["instance_count"]))

            # Write vertices
            f.write(struct.pack(f'<{scene_data["models"][x]["vertex_count"] * 3}f',
                                *scene_data["models"][x]["vertices"]))

            # Write indices
            f.write(struct.pack(f'<{scene_data["models"][x]["indices_count"]}H',
                                *scene_data["models"][x]["indices"]))

            # Write normals
            f.write(struct.pack(f'<{scene_data["models"][x]["normals_count"] * 3}f',
                                *scene_data["models"][x]["normals"]))

            # Write instance matrices
            for y in scene_data["models"][x]["instances"]:
                f.write(struct.pack('<16f', *y))

    return 0


def main():
    parser = argparse.ArgumentParser(description="Convert USD Scene into JSON for Vulkan Rendering.")
    parser.add_argument(
        "--f",
        "--filepath",
        required=True,
        dest="filepath",
        type=str,
        help="The path to the target .usd file that needs converting.",
    )
    parser.add_argument(
        "--s",
        "--scale",
        required=True,
        dest="scale",
        type=float,
        help="The scale all meshes will be increased by.",
    )
    opts = parser.parse_args()
    result = parse_scene(opts.filepath, opts.scale)


if __name__ == "__main__":
    main()
