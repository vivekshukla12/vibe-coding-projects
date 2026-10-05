# CubeCore CAD Status

Updated: 2026-10-05T19:15:27+00:00
Repository: vibe-coding-projects
Project: projects/cubecore-cad
Branch: cad/cubecore/chassis-dovetail-joint
Commit: pending commit
Phase: component prototype
Current component: chassis dovetail joint
Current engineering gate: chassis joining/alignment

## State
- Environment: PASS - build123d and ocp_vscode import successfully
- Build123d: 0.13.0
- Python: 3.13.16 (uv-managed)
- pytest: PASS - 27 passed
- ruff: PASS - all checks passed
- STEP export: PASS - 9 files; valid-solid/count re-import checks passed
- STL export: PASS - 6 files; closed shared-edge mesh tests passed
- OCP Viewer: NOT RUN

## Current task
Publish the preserved, parameterized dovetail fit coupon on its component branch.
Three labelled clearance variants are ready for physical PETG-CF testing.

## Decisions in force
- Confirmed: PETG-CF baseline; sliding dovetail primary chassis joint.
- No primary screws, heat-set inserts, captive nuts or other metal fasteners.
- Dry mechanical operation required; CA glue optional secondary reinforcement only.
- Coupon only; no full-chassis propagation or physical-fit approval.

## Parameters added/changed
- All numeric coupon dimensions remain provisional; no geometry changed during branch split.
- Per-flank clearance: 0.15 / 0.25 / 0.35 mm; tip-floor clearance: 0.40 mm.
- Angle: 20 degrees; depth: 6 mm; engagement: 30 mm; male neck: 10 mm; female width derived.
- Lead-in: 1.5 mm at 0.90 scale; root radius: 0.6 mm; stop/minimum wall: 4 mm.
- Coupon width: 26 mm; engraving depth/font size: 0.4/2.5 mm.

## Files changed
- parameters/dovetail.py, parts/chassis/dovetail_coupon.py, validation/dovetail.py under src/cubecore_cad/.
- examples/dovetail_coupon.py; tests/test_dovetail_coupon.py.
- docs/dovetail-coupon.md; docs/dovetail-results.csv; AGENTS.md; README.md; STATUS.md.

## Outputs
- exports/step/dovetail/: male, female and assembled STEP for each of 3 variants.
- exports/stl/dovetail/: male and female print-oriented STL for each variant.
- exports/dovetail-validation.json; generated outputs remain ignored by Git.

## Validation
- uv run pytest; uv run ruff check .; uv run python examples/dovetail_coupon.py.
- Valid B-Rep, 2 parts/variant, measured clearances/engagement, minimum walls,
  no assembled interference, sampled insertion path, positive stop and transverse capture.
- STEP round-trips, printable closed STL meshes, parameter variation and rejection checks.
- All seven component files match the pre-split safety snapshot byte-for-byte.

## Blockers / user input needed
- No publication blocker. Physical PETG-CF fit/strength and final clearance are unvalidated.
- Record filament/print process and test results; acceptance loads remain undefined.

## Next action
Review/merge workspace setup first, then review this dependent component branch.

## Notes
Based on setup/cubecore-cad-workspace at 1282476f5d43560cfd97b096127dcaea5ff2e194; no local merges.
Commit identifies the validated implementation commit; a later status-only commit records it.
Reverse axial withdrawal remains possible; resolve axial escape before chassis integration.
Exporter emitted a non-fatal system Fontconfig warning; all exports completed.
