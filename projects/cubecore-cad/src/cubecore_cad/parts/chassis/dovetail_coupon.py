"""Two-piece, dry-assembly dovetail coupon. No full chassis geometry."""

from copy import copy
from dataclasses import dataclass

from build123d import (
    Align,
    Box,
    Compound,
    Face,
    Plane,
    Polygon,
    Pos,
    Rot,
    Text,
    extrude,
    loft,
    offset,
    scale,
)

from cubecore_cad.parameters.dovetail import DovetailParameters


@dataclass
class JointCoupon:
    parameters: DovetailParameters
    male: object
    female: object

    @property
    def assembly(self):
        return Compound(
            label=f"CubeCore coupon {self.parameters.slug}",
            children=[copy(self.male), copy(self.female)],
        )

    def print_parts(self):
        """Travel axis vertical; female closed stop on bed, male trailing end on bed."""
        female = Rot(X=180) * self.female
        female = Pos(Z=-female.bounding_box().min.Z) * female
        return self.male, female


def male_profile(p: DovetailParameters) -> Face:
    """Shared nominal seam profile; rounded neck roots and head corners."""
    w, n, h = p.coupon_width / 2, p.male_width / 2, p.head_width / 2
    face = Polygon(
        (-w, -p.surrounding_wall - p.label_depth),
        (w, -p.surrounding_wall - p.label_depth),
        (w, 0),
        (n, 0),
        (h, p.depth),
        (-h, p.depth),
        (-n, 0),
        (-w, 0),
        align=None,
    ).faces()[0]
    roots_and_tips = [v for v in face.vertices() if abs(v.X) < w - 1e-6 and v.Y >= 0]
    return face.fillet_2d(p.root_radius, roots_and_tips)


def cavity_profile(p: DovetailParameters, profile: Face):
    # Offset the actual rounded profile, preserving root relief and normal flank gap.
    clearance_profile = offset(profile, amount=p.side_clearance)
    # Extend only toward the floor. Positive Y translation narrows the straight
    # flanks at a given Y; their governing clearance remains the original offset.
    extra_floor_gap = p.vertical_clearance - p.side_clearance
    if extra_floor_gap > 1e-9:
        clearance_profile = (
            clearance_profile + Pos(Y=extra_floor_gap) * clearance_profile
        )
    return clearance_profile


def engrave(part, p, male):
    back_y = (
        -p.surrounding_wall - p.label_depth
        if male
        else p.depth + p.vertical_clearance + p.surrounding_wall + p.label_depth
    )
    plane = Plane(
        origin=(0, back_y, p.engagement_length / 2),
        x_dir=(1, 0, 0) if male else (-1, 0, 0),
        z_dir=(0, -1, 0) if male else (0, 1, 0),
    )
    prefix = "M" if male else "F"
    for dz, text in (
        (2, f"{prefix} S{p.side_clearance:.2f}"),
        (-2, f"V{p.vertical_clearance:.2f}"),
    ):
        lettering = Pos(Z=dz) * (plane * Text(text, p.label_size, font="DejaVu Sans"))
        part = part - extrude(lettering, amount=-p.label_depth)
    return part


def build_coupon(p: DovetailParameters) -> JointCoupon:
    p.validate()
    profile = male_profile(p)
    main = extrude(profile, amount=p.straight_engagement)
    start = Pos(Z=p.straight_engagement) * profile
    tip = Pos(Z=p.engagement_length) * scale(profile, by=p.lead_in_scale)
    male = main + loft([start, tip], ruled=True)

    female_blank = Box(
        p.coupon_width,
        p.depth + p.vertical_clearance + p.surrounding_wall + p.label_depth,
        p.engagement_length + p.end_stop_thickness,
        align=(Align.CENTER, Align.MIN, Align.MIN),
    )
    cavity = extrude(cavity_profile(p, profile), amount=p.engagement_length)
    female = female_blank - cavity
    male = engrave(male, p, True)
    female = engrave(female, p, False)
    male.label = f"male-{p.slug}"
    female.label = f"female-{p.slug}"
    return JointCoupon(p, male, female)
