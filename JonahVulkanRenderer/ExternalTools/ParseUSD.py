# Requires usd-core Python Package.

from pxr import Usd, UsdGeom, Sdf, Gf
import argparse
import json

def parse_scene(filepath):

    scene_data = {"models": {}}

    """
    {
        models: [
           ModelNAME: {
                vertices: [xxxxxx]
                indices: [xxxxxx]
                instance_count = 3
                instances:{
                    Transformation Matrix (Rotation, Translation, Scale),
                    Transformation Matrix (Rotation, Translation, Scale),
                    Transformation Matrix (Rotation, Translation, Scale)
                }
            },
            {
            }
        ]
    }
    """

    # Open the USD stage from the specified file
    stage: Usd.Stage = Usd.Stage.Open(filepath)

    population_mask = Usd.StagePopulationMask()
    population_mask.Add(Sdf.Path("/world/hotel_01/geo"))
    stage.SetPopulationMask(population_mask)

    scale_constant = 100

    # Traverse through each prim in the stage
    for prim in Usd.PrimRange(stage.GetPseudoRoot(), Usd.TraverseInstanceProxies()):
        if prim.IsA(UsdGeom.Mesh):
            purpose = UsdGeom.Imageable(prim).GetPurposeAttr().Get()

            if purpose == "guide":
                continue

            points = UsdGeom.Mesh(prim).GetPointsAttr().Get()
            indices = UsdGeom.Mesh(prim).GetFaceVertexIndicesAttr().Get()
            model_hash = prim.GetName() + "_" + str(len(points)) + "_" + str(len(indices))
            xform = UsdGeom.Xformable(prim)
            time = Usd.TimeCode.Default()
            world_transform: Gf.Matrix4d = xform.ComputeLocalToWorldTransform(time)

            transform_write = [
            [world_transform[0][0], world_transform[0][1], world_transform[0][2], world_transform[0][3]],
            [world_transform[1][0], world_transform[1][1], world_transform[1][2], world_transform[1][3]],
            [world_transform[2][0], world_transform[2][1], world_transform[2][2], world_transform[2][3]],
            [world_transform[3][0] / scale_constant, world_transform[3][1] / scale_constant,
             world_transform[3][2] / scale_constant, world_transform[3][3]]
            ]
            if model_hash in scene_data["models"]:
                scene_data["models"][model_hash]["instance_count"] += 1
                scene_data["models"][model_hash]["instances"].append(transform_write)
            else:

                face_counts = UsdGeom.Mesh(prim).GetFaceVertexCountsAttr().Get()
                points_float = []
                indices_float = []

                for x in points:
                    points_float.append(x[0] / scale_constant)
                    points_float.append(x[1] / scale_constant)
                    points_float.append(x[2] / scale_constant)

                # OpenUSD models support non-tri mesh faces
                # So to render them we must triangulate them
                # Below is a convenient way to do so for any N-Gon! (In Counter-Clockwise Rotation)
                counter = 0
                for x in face_counts:
                    for i in range(x-2):
                        indices_float.append(indices[counter])
                        indices_float.append(indices[counter + i + 1])
                        indices_float.append(indices[counter + i + 2])

                    counter += x

                scene_data["models"][model_hash] = {
                    "vertices": points_float,
                    "indices": indices_float,
                    "instance_count": 1,
                    "instances": [transform_write]
                }
    with open("scene.json","w") as f:
        json.dump(scene_data, f)
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
    opts = parser.parse_args()
    result = parse_scene(opts.filepath)


if __name__ == "__main__":
    main()
