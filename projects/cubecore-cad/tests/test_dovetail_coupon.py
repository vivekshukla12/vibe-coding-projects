"""Geometry, motion, parameter and interchange checks for trial-fit coupons."""

import struct
from collections import Counter
from dataclasses import replace

import pytest
from build123d import export_step, export_stl, import_step

from cubecore_cad.parameters.dovetail import BASELINE, VARIANTS
from cubecore_cad.parts.chassis.dovetail_coupon import build_coupon
from cubecore_cad.validation.dovetail import validate_coupon


@pytest.fixture(scope="module", params=VARIANTS, ids=lambda p: p.slug)
def coupon(request):
    return build_coupon(request.param)


def test_geometry_and_clearance(coupon):
    result = validate_coupon(coupon)
    assert result["valid_solids"] == 2
    assert len(coupon.assembly.children) == 2


@pytest.mark.parametrize("name", ["male", "female", "assembly"])
def test_step_round_trip(coupon, tmp_path, name):
    # Assembly creation must not reparent the individually exportable originals.
    assembly = coupon.assembly
    assert len(assembly.children) == 2
    assert coupon.male.parent is None
    assert coupon.female.parent is None
    shape = getattr(coupon, name)
    path = tmp_path / f"{name}.step"
    assert export_step(shape, path)
    restored = import_step(path)
    assert restored.is_valid
    assert len(restored.solids()) == (2 if name == "assembly" else 1)
    assert restored.volume == pytest.approx(shape.volume, rel=1e-7)


def test_printable_stls(coupon, tmp_path):
    for i, part in enumerate(coupon.print_parts()):
        assert part.bounding_box().min.Z == pytest.approx(0, abs=1e-7)
        path = tmp_path / f"{i}.stl"
        assert export_stl(part, path)
        data = path.read_bytes()
        triangles = struct.unpack_from("<I", data, 80)[0]
        assert triangles > 0
        assert len(data) == 84 + triangles * 50
        edges = Counter()
        for j in range(triangles):
            values = struct.unpack_from("<12fH", data, 84 + j * 50)
            points = [tuple(round(v, 5) for v in values[k : k + 3]) for k in (3, 6, 9)]
            for a, b in ((0, 1), (1, 2), (2, 0)):
                edges[tuple(sorted((points[a], points[b])))] += 1
        assert set(edges.values()) == {2}, "STL must be closed at shared mesh edges"


@pytest.mark.parametrize(
    "changes",
    [
        {"engagement_length": 36},
        {"male_width": 9, "depth": 5},
        {"angle_deg": 15, "root_radius": 0.5},
        {"vertical_clearance": 0.5, "surrounding_wall": 3.5},
    ],
)
def test_parameter_variations(changes):
    validate_coupon(build_coupon(replace(BASELINE, **changes)))


@pytest.mark.parametrize(
    "changes",
    [
        {"side_clearance": -0.1},
        {"vertical_clearance": 0.1},
        {"coupon_width": 15},
        {"lead_in_length": 30},
        {"root_radius": 5},
        {"end_stop_thickness": 1},
        {"depth": float("nan")},
    ],
)
def test_invalid_parameters_rejected(changes):
    with pytest.raises(ValueError):
        replace(BASELINE, **changes).validate()
