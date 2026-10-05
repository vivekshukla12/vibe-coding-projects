"""Headless environment checks using synthetic, non-product geometry."""

import build123d
import pytest


def test_box_geometry():
    length, width, height = 100, 80, 20  # Fixture dimensions only, in mm.
    solid = build123d.Box(length, width, height)

    assert solid.is_valid
    assert len(solid.solids()) == 1
    size = solid.bounding_box().size
    assert (size.X, size.Y, size.Z) == pytest.approx((length, width, height))
    assert solid.volume == pytest.approx(length * width * height)
