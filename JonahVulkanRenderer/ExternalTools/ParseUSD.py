# Requires usd-core Python Package.
#"C:\map\caldera-main\map_source\prefabs\br\wz_vg\mp_wz_island\commercial\hotel_01.usd"
from pxr import Usd, UsdGeom, Sdf, Gf
import argparse
from collections import defaultdict

def parse_scene(filepath):

    found_models = defaultdict(list)

    # Open the USD stage from the specified file
    stage: Usd.Stage = Usd.Stage.Open(filepath)

    #population_mask = Usd.StagePopulationMask()
    #population_mask.Add(Sdf.Path("/world/hotel_01/geo"))
    #stage.SetPopulationMask(population_mask)

    predicate = Usd.PrimIsActive & Usd.PrimIsDefined
    predicate = Usd.TraverseInstanceProxies(predicate)

    number_of_models = 0
    # Traverse through each prim in the stage
    for prim in stage.Traverse(predicate):
        if(prim.IsA(UsdGeom.Mesh)):

            model_name = prim.GetName()
            number_of_models += 1
            if model_name not in found_models:
                xform = UsdGeom.Xformable(prim)
                time = Usd.TimeCode.Default()
                world_transform: Gf.Matrix4d = xform.ComputeLocalToWorldTransform(time)
                world_translation: Gf.Vec3d = world_transform.ExtractTranslation()
                world_rotation: Gf.Rotation = world_transform.ExtractRotation()
                found_models[prim.GetName()] = 1
            else:
                found_models[prim.GetName()] += 1

    number_of_repeats = 0
    number_of_models_covered = 0
    for key, value in found_models.items():
        if(value > 1):
            number_of_repeats += 1
            number_of_models_covered += value

    print(f"Number of reuses: {number_of_repeats} vs Number of unique models: {len(found_models)}")
    print(f"Total models: {number_of_models}")
    print(f"Precent reuse: {number_of_models_covered / number_of_models}")

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
