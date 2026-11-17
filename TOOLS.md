# Tools and Libraries

| Tool | Project use | Changes made after use |
|---|---|---|
| OpenAI Codex | Reviewed and edited C++ source, test/benchmark runners, application interface, and project documentation. | Team members must review generated edits, verify their assigned modules, and add their own work and review history. |
| GCC / `g++` | Builds the C++17 programs with `-O2` in `run_all.sh`. | Build commands and flags are recorded in `README.md` and `REPORT.md`. |
| Bash and core utilities (`date`, `tee`, `rm`) | `run_all.sh` clears generated outputs, captures logs, and records a UTC run timestamp. | Outputs are kept under `results/raw/`; the script does not invoke the reporting scripts. |
| Python 3 standard library | `csv`, `re`, and `pathlib` parse saved logs; `statistics` and `collections.defaultdict` aggregate points; `html.escape` creates safe SVG labels. | `collect_measurements.py` and `make_figures.py` only process saved output. No third-party Python packages are required. |
| Git | Tracks project changes and supports review of the source and documentation. | Add the required individual commits, reviewed pull requests, and final release tag. |
| `pdftotext` | Extracted text from the supplied project brief and reference material for inspection. | Requirements were reflected in the code and documentation; the original PDFs remain the authoritative sources. |
| `groff` | Typeset an earlier proof draft during document preparation. | The current proof source is maintained separately under `docs/derivations/`; review the final submitted PDF against it. |
| LibreOffice command-line conversion | Tried for a derivation PDF export. | Conversion did not produce the submitted artifact; no LibreOffice-generated file is part of the run pipeline. |

No Python plotting package is used: the figure script writes SVG directly. Record any additional tools used by team members and the changes they made before submission.
