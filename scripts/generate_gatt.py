import glob
import os
import subprocess
import sys

Import("env")

project_dir = env["PROJECT_DIR"]
gatt_path = os.path.join(project_dir, "src", "att_profile.gatt")
header_path = os.path.join(project_dir, "src", "att_profile.h")
compile_gatt_path = os.path.join(project_dir, "tools", "compile_gatt.py")


def find_btstack_headers():
    framework_dir = env.PioPlatform().get_package_dir(
        "framework-arduinoespressif32"
    )

    if not framework_dir:
        return None

    sdk_dir = os.path.join(framework_dir, "tools", "sdk")
    mcu = env.BoardConfig().get("build.mcu", "esp32")

    candidates = [os.path.join(sdk_dir, mcu, "include", "btstack", "src")]
    candidates += sorted(glob.glob(
        os.path.join(sdk_dir, "*", "include", "btstack", "src")
    ))

    for candidate in candidates:
        if os.path.exists(os.path.join(candidate, "bluetooth_gatt.h")):
            return candidate

    return None


def header_is_current():
    if not os.path.exists(header_path):
        return False

    return os.path.getmtime(header_path) >= os.path.getmtime(gatt_path)


def generate_header():
    if header_is_current():
        return

    btstack_headers = find_btstack_headers()

    if btstack_headers is None:
        print(
            "BTstack headers were not found in the Arduino framework "
            "package. Check that platform_packages points at the Bluepad32 "
            "framework."
        )
        env.Exit(1)
        return

    print(f"Generating {header_path} from {gatt_path}")

    subprocess.run(
        [
            sys.executable,
            compile_gatt_path,
            "-I", btstack_headers,
            gatt_path,
            header_path,
        ],
        check=True
    )


generate_header()
