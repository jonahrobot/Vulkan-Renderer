# Requires usd-core Python Package.
#"C:\map\caldera-main\map_source\prefabs\br\wz_vg\mp_wz_island\commercial\hotel_01.usd"
from pxr import Usd, UsdGeom, Sdf, Gf
from collections import defaultdict
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

    # Traverse through each prim in the stage
    for prim in stage.Traverse():
        if prim.IsA(UsdGeom.Mesh):
            model_name = prim.GetName()

            xform = UsdGeom.Xformable(prim)
            time = Usd.TimeCode.Default()
            world_transform: Gf.Matrix4d = xform.ComputeLocalToWorldTransform(time)
            #world_translation: Gf.Vec3d = world_transform.ExtractTranslation()
            #world_rotation: Gf.Rotation = world_transform.ExtractRotation()

            if model_name in scene_data["models"]:
                scene_data["models"][model_name]["instance_count"] += 1
                scene_data["models"][model_name]["instances"].append({
                    world_transform
                })
            else:
                scene_data["models"][model_name] = {
                    "vertices": list(UsdGeom.Mesh(prim).GetPointsAttr().Get()),
                    "indices": list(UsdGeom.Mesh(prim).GetFaceVertexIndicesAttr().Get()),
                    "instance_count": 1,
                    "instances": [world_transform]
                }

    with open("scene.json","w") as f:
        json.dump(scene_data, f, indent=4)

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
