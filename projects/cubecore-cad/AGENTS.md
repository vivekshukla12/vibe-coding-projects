# CubeCore CAD instructions

- `../../../project-workbench/` is the requirements and engineering knowledge
  source. The project record is `projects/kallax-microserver/` within that repo.
- Before CAD edits, read its `AGENTS.md`, `knowledge/cad/`, and the project's
  `README.md`, `requirements.md`, `design.md`, `decisions.md`, `next-steps.md`,
  and relevant engineering parameters, interfaces, CAD plan and validation.
- Never invent unverified physical dimensions. Ask the user only for blocking
  measurements or decisions; keep non-blocking unknowns explicitly provisional.
- Use named parameters and derived dimensions; avoid repeated hard-coded
  dimensions. Preserve stable interfaces and datums.
- Make small coherent changes. Run tests after meaningful changes and validate
  geometry and dimensions programmatically where practical.
- Use uv-managed Python 3.13. Build123d/OpenCascade B-Rep Python source is the
  implementation geometry. STEP is the preferred neutral engineering export;
  STL/3MF are downstream manufacturing outputs, not the source of truth.
- Rendering success does not mean production-ready. Follow the knowledge
  source's applicable dimensional, interface and manufacturing validation gates.
- Onshape is for downstream STEP import/review only. Do not use an Onshape API
  or continuous Onshape synchronization workflow. This explicit implementation
  direction overrides older Onshape-authority statements in the knowledge source.
- The active gate is an isolated PETG-CF sliding-dovetail fit coupon. Do not
  propagate it to the full chassis before physical validation. Use no primary
  metal fasteners; optional cyanoacrylate is secondary to a successful dry fit.
  Coupon dimensions and clearances remain provisional. The smoke-test box is
  only an environment fixture, never product geometry.
- Keep this project inside the existing `vibe-coding-projects` Git repository;
  do not initialize a standalone or nested repository.
- Do not modify `project-workbench` unless explicitly authorized for the current
  task. Keep `.venv`, caches and exports ignored; retain `uv.lock` in Git.

## Git workflow and status

- Never develop CAD components on main. Verify the branch at session start before
  changing geometry. If on main, create/switch to the appropriate component branch.
- Initial workspace work uses `setup/cubecore-cad-workspace`.
- Use one branch per component/coupon/subassembly: `cad/cubecore/<component-slug>`.
- Do not reuse a completed component branch for another component.
- Main receives component work only after review/merge. Do not merge locally.
- Update `STATUS.md` with the actual branch and validation results before every
  push; commit and push it on every active CubeCore development branch.
- The status Commit field may identify the validated implementation commit; a
  following status-only commit records that SHA without a self-reference loop.
- Commit/push when authorized by the current task; do not discard existing work.

## Dimension research

- Before requesting measurements, search official manufacturer/standards
  documentation for the exact product/model. Record sources for nominal inputs.
- If official data is unavailable, search credible community sources. Mark these
  values provisional and confirm critical values before locking production geometry.
- Ask for measurement only if reliable data is unavailable, conflicting, ambiguous,
  or the exact product variant is unknown; do not substitute convenience for research.
