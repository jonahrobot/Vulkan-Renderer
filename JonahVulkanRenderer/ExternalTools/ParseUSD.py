# Requires usd-core Python Package.
#"C:\map\caldera-main\map_source\prefabs\br\wz_vg\mp_wz_island\commercial\hotel_01.usd"
from pxr import Usd, UsdGeom
import argparse

def parse_scene(filepath):

    # Open the USD stage from the specified file
    stage: Usd.Stage = Usd.Stage.Open(filepath)

    # Traverse through each prim in the stage
    for prim in stage.Traverse():
        print(prim.GetPath())

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
