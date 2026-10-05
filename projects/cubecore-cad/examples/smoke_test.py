"""Validate the CAD environment with a synthetic box, not CubeCore geometry."""

from pathlib import Path

import ocp_vscode
from build123d import Box, export_step


def main():
    # User-specified fixture dimensions in mm; not product parameters.
    length, width, height = 100, 80, 20
    solid = Box(length, width, height)
    if not solid.is_valid or len(solid.solids()) != 1:
        raise RuntimeError("Smoke-test box is not a valid single solid")

    project_root = Path(__file__).resolve().parents[1]
    step_path = project_root / "exports" / "step" / "cubecore-smoke-test.step"
    step_path.parent.mkdir(parents=True, exist_ok=True)
    if not export_step(solid, step_path) or step_path.stat().st_size == 0:
        raise RuntimeError("STEP export failed")
    print(f"Geometry and STEP export successful: {step_path}")

    ocp_vscode.show(solid)
    print("Viewer display requested; check OCP Viewer and any connection warnings.")
    print("CubeCore CAD smoke test successful (environment fixture only).")


if __name__ == "__main__":
    main()
