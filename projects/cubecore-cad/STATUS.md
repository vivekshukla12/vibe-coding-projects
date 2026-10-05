# CubeCore CAD Status

Updated: 2026-10-05T18:55:45+00:00
Repository: vibe-coding-projects
Project: projects/cubecore-cad
Branch: setup/cubecore-cad-workspace
Commit: pending commit
Phase: setup
Current component: none
Current engineering gate: CAD workspace setup

## State
- Environment: PASS - build123d and ocp_vscode import successfully
- Build123d: 0.13.0
- Python: 3.13.16 (uv-managed)
- pytest: PASS - 1 scaffold smoke test; 27 tests passed in working tree including uncommitted component
- ruff: PASS - all checks passed
- STEP export: NOT RUN
- STL export: NOT RUN
- OCP Viewer: NOT RUN

## Current task
Separate and publish the generic CAD workspace without component geometry.

## Decisions in force
- Python 3.13, uv, Build123d/OpenCascade B-Rep source; STEP engineering output.
- STL/3MF are manufacturing outputs; Onshape is downstream STEP review only.
- One branch per component; no geometry development on main or local merges.
- Research reliable dimensional sources before requesting measurements.

## Parameters added/changed
- No product parameters; the smoke-test box is a synthetic 100 x 80 x 20 mm fixture.

## Files changed
- AGENTS.md, README.md, STATUS.md, pyproject.toml, uv.lock, .python-version, .gitignore.
- Generic src/cubecore_cad package initializers and py.typed.
- examples/smoke_test.py and tests/test_smoke.py.

## Outputs
- Reproducible Python workspace and generic headless smoke test.

## Validation
- uv run pytest; uv run pytest tests/test_smoke.py -q; uv run ruff check .
- Import check passed; component files remain excluded from this branch's commit.

## Blockers / user input needed
- none

## Next action
Merge workspace setup after review, then continue the dependent dovetail branch.

## Notes
Existing component work is preserved locally, uncommitted during setup.
Generated exports, environments and caches are ignored; uv.lock is tracked.
Commit identifies the validated implementation commit; a later status-only commit records it.
