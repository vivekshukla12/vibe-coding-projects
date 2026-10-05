"""Export three labelled PETG-CF trial fits, without requiring a viewer."""

import json
from dataclasses import asdict
from pathlib import Path

from build123d import export_step, export_stl, import_step

from cubecore_cad.parameters.dovetail import VARIANTS
from cubecore_cad.parts.chassis.dovetail_coupon import build_coupon
from cubecore_cad.validation.dovetail import validate_coupon


def main():
    root = Path(__file__).resolve().parents[1] / "exports"
    step_dir, stl_dir = root / "step" / "dovetail", root / "stl" / "dovetail"
    for directory in (step_dir, stl_dir):
        directory.mkdir(parents=True, exist_ok=True)
    reports = []
    for parameters in VARIANTS:
        coupon = build_coupon(parameters)
        report = validate_coupon(coupon)
        prefix = f"cubecore-dovetail-{parameters.slug}"
        for name, shape in (
            ("male", coupon.male),
            ("female", coupon.female),
            ("assembled", coupon.assembly),
        ):
            path = step_dir / f"{prefix}-{name}.step"
            if not export_step(shape, path):
                raise RuntimeError(f"STEP export failed: {path}")
            restored = import_step(path)
            expected_count = 2 if name == "assembled" else 1
            if not restored.is_valid or len(restored.solids()) != expected_count:
                raise RuntimeError(f"STEP re-import failed: {path}")
        for name, shape in zip(("male", "female"), coupon.print_parts(), strict=True):
            path = stl_dir / f"{prefix}-{name}.stl"
            if not export_stl(shape, path, tolerance=0.001, angular_tolerance=0.1):
                raise RuntimeError(f"STL export failed: {path}")
            if path.stat().st_size <= 84:
                raise RuntimeError(f"Empty STL: {path}")
        reports.append(
            {
                "variant": parameters.slug,
                "parameters": asdict(parameters),
                "female_width_mm": parameters.female_width,
                "checks": report,
            }
        )
        print(f"Validated/exported {prefix}: 3 STEP, 2 STL")
    (root / "dovetail-validation.json").write_text(json.dumps(reports, indent=2) + "\n")
    print("Dry-fit coupons only. Physical PETG-CF fit/strength testing still required.")


if __name__ == "__main__":
    main()
