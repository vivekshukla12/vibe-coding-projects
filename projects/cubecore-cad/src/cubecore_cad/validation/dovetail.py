"""Repeatable geometry checks; physical fit and strength remain unvalidated."""

from math import radians, tan

from build123d import Align, Box, Pos

from cubecore_cad.parts.chassis.dovetail_coupon import JointCoupon


def validate_coupon(coupon: JointCoupon):
    p = coupon.parameters
    p.validate()
    for part in (coupon.male, coupon.female):
        if not part.is_valid or len(part.solids()) != 1 or part.volume <= 0:
            raise ValueError("Expected one valid positive-volume solid per coupon part")
    if len(coupon.assembly.solids()) != 2:
        raise ValueError("Assembly must contain exactly two solids")
    if (coupon.male & coupon.female).volume > 1e-6:
        raise ValueError("Assembled parts intersect")

    def close(actual, expected, name):
        if abs(actual - expected) > 1e-5:
            raise ValueError(f"{name}: measured {actual}, expected {expected}")

    z = p.straight_engagement / 2
    y = p.depth / 2
    x = p.male_width / 2 + y * tan(radians(p.angle_deg))
    gaps = [coupon.female.distance_to((sign * x, y, z)) for sign in (-1, 1)]
    for gap in gaps:
        close(gap, p.side_clearance, "normal flank clearance")
    floor_gap = coupon.female.distance_to((0, p.depth, z))
    close(floor_gap, p.vertical_clearance, "tip-to-floor clearance")
    close(coupon.male.bounding_box().size.Z, p.engagement_length, "engagement length")
    close(
        coupon.female.bounding_box().size.Z,
        p.engagement_length + p.end_stop_thickness,
        "female length",
    )

    # Check actual material volumes, not only the formulas that constructed them.
    male_wall = Pos(Y=-p.surrounding_wall) * Box(
        p.coupon_width,
        p.surrounding_wall,
        p.straight_engagement,
        align=(Align.CENTER, Align.MIN, Align.MIN),
    )
    floor = Pos(Y=p.depth + p.vertical_clearance) * Box(
        p.coupon_width,
        p.surrounding_wall,
        p.engagement_length,
        align=(Align.CENTER, Align.MIN, Align.MIN),
    )
    stop = Pos(Y=p.vertical_clearance, Z=p.engagement_length) * Box(
        p.coupon_width,
        p.depth + p.surrounding_wall,
        p.end_stop_thickness,
        align=(Align.CENTER, Align.MIN, Align.MIN),
    )
    for region, part in (
        (male_wall, coupon.male),
        (floor, coupon.female),
        (stop, coupon.female),
    ):
        if (region - part).volume > 1e-5:
            raise ValueError("Minimum backing wall, floor or stop material missing")
    for sign in (-1, 1):
        side = Pos(
            X=sign * (p.coupon_width / 2 - p.surrounding_wall / 2),
            Y=p.vertical_clearance,
        ) * Box(
            p.surrounding_wall,
            p.depth + p.surrounding_wall,
            p.engagement_length,
            align=(Align.CENTER, Align.MIN, Align.MIN),
        )
        if (side - coupon.female).volume > 1e-5:
            raise ValueError("Minimum female side wall missing")

    # Assembly/withdrawal path stays free; the forward stop is positive geometry.
    for travel in (-p.engagement_length, -p.engagement_length / 2, -p.lead_in_length):
        if ((Pos(Z=travel) * coupon.male) & coupon.female).volume > 1e-6:
            raise ValueError("Obstructed insertion path")
    if ((Pos(Z=p.end_stop_thickness / 2) * coupon.male) & coupon.female).volume <= 0:
        raise ValueError("Missing positive forward stop")
    if ((Pos(Y=-p.depth / 2) * coupon.male) & coupon.female).volume <= 0:
        raise ValueError("Missing undercut engagement against seam separation")
    return {
        "valid_solids": 2,
        "intersection_volume_mm3": 0.0,
        "flank_clearances_mm": gaps,
        "tip_floor_clearance_mm": floor_gap,
        "engagement_mm": p.engagement_length,
        "straight_engagement_mm": p.straight_engagement,
        "minimum_surrounding_wall_mm": p.surrounding_wall,
        "insertion_path": "sampled collision-free",
        "physical_fit_and_strength": "NOT VALIDATED",
    }
