# CubeCore CAD

Local parametric implementation using uv-managed Python 3.13, build123d,
OpenCascade, pytest and OCP Viewer. This branch provides the generic workspace,
package structure and headless environment smoke test. Component geometry belongs
on its own CAD branch. See [STATUS.md](STATUS.md) for the current branch state.

The requirements and engineering knowledge source is
[`../../../project-workbench/projects/kallax-microserver/`](../../../project-workbench/projects/kallax-microserver/),
in the sibling `project-workbench` repository. Read its requirements, decisions
and engineering records before CAD changes.

Build123d/OpenCascade B-Rep Python source will define the implementation geometry.
STEP is the neutral engineering export; STL/3MF are manufacturing outputs.
Onshape is downstream STEP import/review only, with no API workflow. This explicit
direction takes precedence over historical Onshape-authority statements in the
knowledge source and is recorded in its latest accepted decisions.

Run from this directory:

```sh
uv sync
uv run pytest
uv run ruff check .
uv run python examples/smoke_test.py
```

The example validates a **100 × 80 × 20 mm test box**, exports it to
`exports/step/cubecore-smoke-test.step`, then calls `ocp_vscode.show()`.
These are synthetic fixture dimensions, not CubeCore dimensions. Tests are
headless and do not require a viewer. Rendering/export success is not production
validation.

For interactive display, install/enable the **OCP CAD Viewer** VS Code extension,
open its viewer, select this project's `.venv` interpreter and rerun the example
in the same environment. Viewer connection failures do not undo the STEP export.
The example reports viewer availability separately from geometry/export success.
With ocp-vscode 4.1.0, missing viewer discovery can print a `port None` traceback,
collapse-value and camera warnings while the script still exits successfully.
An exit code of zero therefore does not confirm interactive display.

`src/cubecore_cad/` reserves packages for parameters, interfaces, parts/chassis,
assemblies and validation. Generated environments, caches and `exports/` are
ignored by Git; `uv.lock` is intended for version control. Export directories
are local outputs and are recreated by scripts as needed.
