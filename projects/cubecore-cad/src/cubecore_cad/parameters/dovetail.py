"""Provisional coupon dimensions in mm, not validated chassis dimensions."""

from dataclasses import dataclass, replace
from math import cos, isfinite, radians, tan


@dataclass(frozen=True)
class DovetailParameters:
    angle_deg: float = 20.0  # Flank angle from the seam normal (Y).
    depth: float = 6.0
    engagement_length: float = 30.0  # Includes the insertion lead-in.
    male_width: float = 10.0  # Theoretical sharp neck at Y=0.
    side_clearance: float = 0.25  # Normal to each sloping flank.
    vertical_clearance: float = 0.40  # Male tip to female floor, along Y.
    lead_in_length: float = 1.5
    lead_in_scale: float = 0.90
    root_radius: float = 0.6
    end_stop_thickness: float = 4.0
    surrounding_wall: float = 4.0
    coupon_width: float = 26.0
    label_depth: float = 0.4
    label_size: float = 2.5

    @property
    def female_width(self):
        """Derived theoretical neck width; never an independent conflicting input."""
        return self.male_width + 2 * self.side_clearance / cos(radians(self.angle_deg))

    @property
    def head_width(self):
        return self.male_width + 2 * self.depth * tan(radians(self.angle_deg))

    @property
    def straight_engagement(self):
        return self.engagement_length - self.lead_in_length

    @property
    def slug(self):
        return f"s{self.side_clearance:.2f}-v{self.vertical_clearance:.2f}"

    def validate(self):
        for name, value in vars(self).items():
            if not isfinite(value) or value <= 0:
                raise ValueError(f"{name} must be finite and positive")
        if not 5 <= self.angle_deg <= 35:
            raise ValueError("Coupon angle must be between 5 and 35 degrees")
        if self.vertical_clearance < self.side_clearance:
            raise ValueError(
                "This cavity construction requires vertical >= side clearance"
            )
        if not 0 < self.lead_in_scale < 1:
            raise ValueError("Lead-in scale must be between zero and one")
        if self.lead_in_length >= self.engagement_length / 2:
            raise ValueError("Lead-in must leave a substantial straight engagement")
        if self.root_radius >= min(self.depth, self.male_width / 2) / 3:
            raise ValueError("Root radius is too large for this coupon profile")
        cavity_width = self.head_width + 2 * self.side_clearance / cos(
            radians(self.angle_deg)
        )
        if self.coupon_width < cavity_width + 2 * self.surrounding_wall:
            raise ValueError("Insufficient female side wall")
        if self.end_stop_thickness < self.surrounding_wall:
            raise ValueError("End stop must preserve the minimum surrounding wall")
        if self.engagement_length < 6 * self.label_size:
            raise ValueError("Engagement too short for the engraved labels")


BASELINE = DovetailParameters()
VARIANTS = tuple(replace(BASELINE, side_clearance=c) for c in (0.15, 0.25, 0.35))
