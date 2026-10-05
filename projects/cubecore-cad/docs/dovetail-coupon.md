# PETG-CF sliding dovetail fit coupon

Accepted direction: PETG-CF, dry mechanical dovetail engagement, no primary metal
fasteners, optional cyanoacrylate reinforcement only after successful dry testing.
This is an isolated coupon, not a chassis model or a structural qualification.

## Architecture and coordinates

X is lateral across the key, Y is normal to the seam, and Z is insertion travel.
The male has a backing strip, rounded neck roots and rounded head corners. The
female has an offset matching cavity, side walls, a floor and a closed end stop.
The male leading end reduces over a short lead-in; the remaining engagement is
constant-section, with no lengthwise wedging or intentional interference.

Undercut flanks geometrically oppose lateral movement and separation normal to the
seam. The closed end stop opposes forward travel. **Reverse axial withdrawal is
intentionally possible.** This supports repeated dry tests; it is not a claim of
positive locking in both axial directions. Resolve chassis-level axial escape and
load paths before reusing this joint in a complete structure. Do not count on
friction or glue as the primary transverse retention mechanism.

The coupon's nominal profile is the candidate for the eventual seam, implemented
once in `male_profile()`. It must pass tests before chassis reuse. No full chassis
geometry has been created.

## Provisional parameters

The executable definitions are in
[`dovetail.py`](../src/cubecore_cad/parameters/dovetail.py). All numeric dimensions
are provisional coupon design choices. They are not measured CubeCore dimensions.

| Parameter | Initial value | Meaning |
|---|---:|---|
| Angle | 20° | Each flank from Y (seam normal), not an included angle |
| Depth | 6 mm | Key depth along Y |
| Engagement | 30 mm | Includes 1.5 mm leading end; 28.5 mm straight engagement |
| Male neck width | 10 mm | Theoretical unrounded width at Y=0 |
| Female neck width | Derived | Male width + `2 × side clearance / cos(angle)` |
| Side clearance | 0.15 / 0.25 / 0.35 mm | Normal to **each** flank |
| Vertical clearance | 0.40 mm | Along Y, male tip to channel floor |
| Lead-in length / scale | 1.5 mm / 0.90 | Reduced leading profile |
| Root radius | 0.6 mm | Rounded male neck roots/head corners; offset cavity follows |
| End stop | 4 mm | Full-width closed end; female total length 34 mm |
| Surrounding wall | 4 mm minimum | Backing, side walls, floor and stop |
| Coupon width | 26 mm | Stock envelope shared by all variants |
| Engraving depth / font size | 0.4 / 2.5 mm | Added backing/floor allowance preserves minimum wall |

The female profile is derived from the rounded male, offset by side clearance and
extended toward the floor to obtain vertical clearance. This implementation
requires vertical clearance >= side clearance. Nominal neck widths are reference
values: root rounding modifies the local mouth. Engraved labels `M/F S0.xx` and
`V0.40` distinguish gender and both clearances. Male nominal geometry is the same
across the three variants; labels change.

The 0.15–0.35 mm matrix is a provisional experiment, **not a published PETG-CF
clearance recommendation**. CAD gaps do not directly predict assembled play.
The final clearance must be chosen from print results; if all three fail, generate
a new matrix. Do not use glue to fill a poor fit.

## Generate and verify

From the project directory:

```sh
uv run pytest
uv run ruff check .
uv run python examples/dovetail_coupon.py
```

For each `s0.15-v0.40`, `s0.25-v0.40`, `s0.35-v0.40` variant:

- `exports/step/dovetail/cubecore-dovetail-<variant>-male.step`
- `exports/step/dovetail/cubecore-dovetail-<variant>-female.step`
- `exports/step/dovetail/cubecore-dovetail-<variant>-assembled.step`
- `exports/stl/dovetail/cubecore-dovetail-<variant>-male.stl`
- `exports/stl/dovetail/cubecore-dovetail-<variant>-female.stl`

STEP preserves assembly coordinates. STL files are individually oriented and
placed on Z=0 for printing, in millimetres. The female STL is rotated so its closed
stop is on the bed. Do not use the assembled STEP as a print-in-place model.
`exports/dovetail-validation.json` records parameters and geometric checks.
Exports are ignored by Git and reproducible from Python source.

Automated checks cover two valid solids, actual flank and floor distances,
engagement, backing/floor/side/stop material volumes, zero assembled intersection,
sampled insertion travel, forward stop contact, transverse undercut engagement,
parameter variations, STEP round-trip and watertight STL edges.
These are geometry checks, not a simulation of stress, friction, wear or fit.

## Proposed print orientation

For the first fit comparison, print with the **slide axis vertical**:

- Male: trailing flat end on bed, reduced lead-in at top.
- Female: closed stop on bed, insertion opening at top (already set in STL).

This keeps the dovetail cross-section in the XY layer plane and the channel open
upward, avoiding support inside the sliding surfaces. The female closed stop
requires no bridge over a cavity in this orientation. Axial loading and stop
attachment still involve layer-direction strength; this is not a universal
strength-optimal orientation for every eventual chassis panel.

Use the same orientation, filament and slicer profile for all variants. Follow the
selected PETG-CF manufacturer's nozzle and drying guidance. Filled materials can
have poorer interlayer adhesion, so qualify the eventual chassis orientation
separately; see [Prusa's composite-material guidance](https://help.prusa3d.com/article/composite-materials-filled-with-carbon-kevlar-or-glass_167387).
No material-specific load rating or temperature profile is assumed here.

## Dry-fit procedure

1. Record filament brand/batch, drying, nozzle, layer height, wall count, infill,
   temperatures, orientation and dimensional/elephant-foot compensation. Keep
   these fixed across the matrix. Verify millimetres and 100% scale in the slicer.
2. Print all three pairs. Inspect labels, roots, channel and stop. Remove only
   loose strings/brim; record any deburring. Do not sand a tight fit into passing.
3. Start with the largest clearance. Slide by hand toward the closed stop. Stop
   at excessive force, cracking or chipping; do not hammer. Keep all pairs dry.
4. Record smoothness, whether the stop seats, lateral and seam-normal play,
   alignment and insertion/removal force if you can measure it. Compare the
   other variants without forcing them.
5. Run 10 assembly/removal cycles as an initial screening test. Record wear,
   looseness, cracks and chips after each cycle. This is not a lifetime test.
6. Select the smallest clearance that remains smooth and repeatable with minimal
   play. If none qualifies, adjust side and/or vertical clearance and reprint;
   do not adopt a numerical final clearance based on CAD alone.
7. Test shear, separation and end-stop loads only against an agreed load target
   before structural qualification. Preserve a dry reference pair. Optional glue
   evaluation uses a separate pair after dry fit succeeds.

Use [`dovetail-results.csv`](dovetail-results.csv) for observations. The exact
filament/process, physical results, final clearance and acceptance loads remain
open. None blocks generating the initial coupons. Full-chassis joint placement,
print orientation and axial escape control remain later integration decisions.
