# Vendored QEMU files

These files are vendored verbatim from upstream QEMU and are licensed under
the **GNU Lesser General Public License v2.1 or later** (decode files) and
**GNU General Public License v2 or later** (decodetree.py).

| File              | Source path                            | Vendored from commit          |
| ----------------- | -------------------------------------- | ----------------------------- |
| `decodetree.py`   | `scripts/decodetree.py`                | `b79f944e0965` (2025-07-23)   |
| `a32.decode`      | `target/arm/tcg/a32.decode`            | `b79f944e0965` (2025-07-23)   |
| `a32-uncond.decode` | `target/arm/tcg/a32-uncond.decode`   | `b79f944e0965` (2025-07-23)   |

Upstream: <https://github.com/qemu/qemu>. To refresh, run from this directory:

```bash
curl -sSL -o decodetree.py    https://raw.githubusercontent.com/qemu/qemu/master/scripts/decodetree.py
curl -sSL -o a32.decode       https://raw.githubusercontent.com/qemu/qemu/master/target/arm/tcg/a32.decode
curl -sSL -o a32-uncond.decode https://raw.githubusercontent.com/qemu/qemu/master/target/arm/tcg/a32-uncond.decode
```

and update the commit table above.

## Why vendored

Per the project plan, `decodetree.py` is the most QEMU-faithful decoder
generator that is also self-contained (no dependency on QEMU runtime).
We invoke it at build time to produce a C dispatch table; we then provide
`trans_*` implementations that emit MLIR `arm.*` ops instead of TCG ops.

## License consequence

Vendoring `decodetree.py` (GPLv2-or-later) and the `.decode` files
(LGPLv2.1-or-later) requires this project to be GPLv2-compatible. We pick
**GPLv2** for the project as a whole — see the top-level `LICENSE`.
