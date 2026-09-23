import glob
import os
import subprocess
import sys

Import("env")

project_dir = env["PROJECT_DIR"]
gatt_path = os.path.join(project_dir, "src", "att_profile.gatt")
header_path = os.path.join(project_dir, "src", "att_profile.h")


def find_compile_gatt():
    framework_dir = env.PioPlatform().get_package_dir(
        "framework-arduinoespressif32"
    )

    if not framework_dir:
        return None

    matches = glob.glob(
        os.path.join(framework_dir, "**", "compile_gatt.py"),
        recursive=True
    )

    return matches[0] if matches else None


def header_is_current():
    if not os.path.exists(header_path):
        return False

    return os.path.getmtime(header_path) >= os.path.getmtime(gatt_path)


def generate_header():
    if header_is_current():
        return

    compile_gatt = find_compile_gatt()

    if compile_gatt is None:
        print(
            "compile_gatt.py was not found in the Arduino framework package. "
            f"Generate {header_path} from {gatt_path} manually using the "
            "compile_gatt.py tool shipped with BTstack."
        )
        env.Exit(1)
        return

    print(f"Generating {header_path} from {gatt_path}")

    subprocess.run(
        [sys.executable, compile_gatt, gatt_path, header_path],
        check=True
    )


generate_header()
