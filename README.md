# Retro-vision ASCII Video Engine

A pure **C** pipeline that reads **JSON** configuration, **WAV** audio, and a **BMP** frame sequence, then produces **ASCII** output—both a human-readable render and a **compressed** stream you can **preview** in the terminal. The project is built for learning low-level multimedia handling without relying on black-box multimedia libraries.

---

## Acknowledgement

This work was carried out in **2026** as part of **50.051 Programming Language Concept** at the **Singapore University of Technology and Design (SUTD)**.

We thank **Professor Matthieu De Mari** and **Professor Dileepa Fernando** for the course framing, emphasis on disciplined C programming, parsers, state machines, and file I/O—and for the opportunity to connect those ideas to a concrete, end-to-end system.

---

## Quick start (three commands)

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

---

## Development notes

- **Strict flags:** `Makefile` uses `-Wall -Werror -ansi -pedantic`.
- **Tests:** e.g. `make run_test TEST=tests/parsers/test_wav.c` (see `Makefile` for the pattern).
- **Layout:** `src/` (implementation), `include/` (headers), `tests/` (unit tests), `output/demo/` (demos and sample artifacts).

---

## License

See [`LICENSE`](LICENSE) in this repository.
