# Retro-vision ASCII Video Engine

A pure **C** pipeline that reads **JSON** configuration, **WAV** audio, and a **BMP** frame sequence, then produces **ASCII** output—both a human-readable render and a **compressed** stream you can **preview** in the terminal. The project is built for learning low-level multimedia handling without relying on black-box multimedia libraries.

---

## Acknowledgement

This work was carried out in **2026** as part of **50.051 Programming Language Concept** at the **Singapore University of Technology and Design (SUTD)**.

We thank **Professor Matthieu De Mari** and **Professor Dileepa Fernando** for the course framing, emphasis on disciplined C programming, parsers, state machines, and file I/O—and for the opportunity to connect those ideas to a concrete, end-to-end system.

---

## Quick start

Run these **from the repository root** so paths in the demo config (e.g. `./frames`) resolve correctly. You need a C toolchain (**`gcc`**, **`make`**) and the frame assets expected by `output/demo/demo_engine/test_config.json` (BMPs under `./frames` for the configured frame range).

### 1. Build

```bash
make
```

This compiles the full project and produces two executables in the project root:

| Binary        | Role |
|---------------|------|
| **`retrovision`** | End-to-end engine: load config + audio + frames → plain render + compressed output + process log |
| **`preview`**     | Terminal playback: read a compressed render file and animate ASCII |

### 2. Render (plain + compressed + log)

```bash
./retrovision output/demo/demo_engine/test_config.json output/demo/demo_engine/videoplayback.wav output/demo/demo_engine/render.txt output/demo/demo_engine/render_compress.txt output/demo/demo_engine/process.log
```

On success you should see `engine completed successfully` and updated files under `output/demo/demo_engine/`.

### 3. Preview the compressed output

```bash
./preview output/demo/demo_engine/render_compress.txt
```

This decodes the compressed timeline and prints frames in the terminal (timing uses short sleeps; behavior may differ slightly on Windows vs POSIX builds).

### 4. Run unit tests (optional)

Tests live under `tests/` as standalone C files with their own `main()`. The `Makefile` builds **one** test at a time: it links the chosen test against the same object files as `retrovision` (full library build), plus `tests/tests_helper.c`.

| Target | What it does |
|--------|----------------|
| **`make test_all`** | Runs **every** `tests/**/test_*.c` in sorted order (uses `find` and `sort`; needs a POSIX shell). Stops at the first failing test. |
| **`make run_test TEST=…`** | Builds everything needed (`make all`), compiles the test, links `build/tests/<name>`, then **runs** that binary. |
| **`make test TEST=…`** | Same build and link, but **does not run** the test (only prints `Built: build/tests/<name>`). |

**`TEST` is required** for `test` / `run_test`. If you run `make test` with no `TEST`, Make prints usage and exits with an error.

Sweep the whole suite:

```bash
make test_all
```

Run a single test (from the repository root; other tests under `tests/` use the same pattern):

```bash
make run_test TEST=tests/common/test_io_utils.c
make run_test TEST=tests/parsers/test_wav.c
make run_test TEST=tests/parsers/json/test_json.c
make run_test TEST=tests/compressions/algorithms/test_huffman_roundtrip.c
```

Each `run_test` (and each step inside `test_all`) links one test against the full library build; after the first run, incremental builds are usually small.

---

## Project structure

The codebase is organized by **layer**: shared utilities, **parsers** for external formats, **components** that implement the video pipeline, and **compressions** (codec stack and algorithms). Public headers under `include/` mirror `src/` so other translation units and tests include a single, consistent API.

### Top-level layout

```text
.
├── Makefile              # Builds `retrovision`, `preview`, and one-off test binaries
├── LICENSE
├── README.md
├── .gitignore            # Ignores `build/`, demo outputs, LaTeX aux files, etc.
├── .gitattributes        # Git attributes (see file for patterns such as LFS on WAV demos)
├── include/              # Headers (.h), grouped like `src/`
├── src/                  # Library and program implementation (.c)
├── tests/                # Unit tests + `tests_helper` shared by tests
├── docs/                 # Course documentation; LaTeX report under `docs/report/` when present
├── output/demo/          # Small standalone demos and the main engine demo assets
├── frames/               # BMP frame sequence used by the engine demo (paths in JSON config)
├── build/                # Created by `make` — objects, `build/tests/*` (listed in `.gitignore`)
├── retrovision           # Appears in the repo root after `make` (not always committed)
└── preview               # Appears in the repo root after `make` (not always committed)
```

**`docs/report/`** (when present) holds the course LaTeX report: `main.tex`, chapter files under `sections/`, figures under `figures/`, and the built PDF. Intermediate LaTeX artifacts are typically gitignored.

### `include/` and `src/` (modules)

Headers live in `include/`; implementations with the same basename live under `src/`. The table below summarizes responsibilities; filenames follow the stem in the second column.

| Directory | C sources / headers (stems) | Role |
|-----------|-----------------------------|------|
| **`common/`** | `io_utils`, `minheap` | Binary I/O helpers, small heap utility used by compression code |
| **`parsers/`** | `wav`, `bmp` | WAV and BMP readers (no external multimedia libs) |
| **`parsers/json/`** | `json`, `config` | JSON tokenizer/parser and higher-level config loading |
| **`components/`** | `sequence`, `ascii`, `render`, `render_compress`, `engine`, `preview` | Frame timeline, raster→ASCII, plain and compressed render writers, orchestration, terminal preview player |
| **`compressions/`** | `bitstream`, `compress`, `decompress` | Bit-level I/O and encode/decode entry points for the compressed render format |
| **`compressions/algorithms/`** | `rle`, `delta`, `huffman` | Concrete codecs used inside the compression layer |

The **main engine executable** is not under `src/` as a generic `main.c`: the `Makefile` compiles `output/demo/demo_engine/demo_engine.c` into `demo_engine.o` and links it with the component library to produce **`retrovision`**.

### `tests/`

- **`tests_helper.c` / `tests_helper.h`** — shared helpers for assertions and reporting used by multiple tests.
- Subfolders mirror the code under test: **`common/`**, **`parsers/`**, **`parsers/json/`**, **`components/`**, **`compressions/`**, **`compressions/algorithms/`** — each file is a separate test program with its own `main()`.

### `output/demo/`

| Subfolder | Contents |
|-----------|----------|
| **`demo_engine/`** | `demo_engine.c` (engine `main`), `test_config.json`, `videoplayback.wav`, and generated outputs such as `render.txt` / `render_compress.txt` / `process.log` when you run the pipeline (some generated files may be gitignored). |
| **`demo_bmp/`**, **`demo_json/`**, **`demo_wav/`** | Focused demos for BMP, JSON, and WAV handling respectively (small C programs and sample binary/JSON inputs). |

Paths in `test_config.json` are relative to the **repository root** (for example `./frames`), so run `retrovision` from the project root as in Quick start.

### `frames/`

Numbered **`.bmp`** frames consumed by the engine demo. This directory can be large; it may be shipped separately or generated for your environment. The JSON config selects which frame indices to read.

---

## Development notes

- **Strict flags:** `Makefile` uses `-Wall -Werror -ansi -pedantic`.
- **Overview of directories:** see **Project structure** above.

---

## License

See [`LICENSE`](LICENSE) in this repository.