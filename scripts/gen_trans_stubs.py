#!/usr/bin/env python3
"""Generate stub trans_* implementations from a decodetree-generated decoder.

For every `static bool trans_X(...)` forward declaration in the input, emit a
matching definition that calls back to the C++ Lifter via the C bridge
`arm_lift_unhandled` and returns `true`. Names listed in the optional
`--exclude` file are skipped (they're hand-implemented in Semantics.def).
"""
import argparse
import re
import sys
from pathlib import Path


def main() -> int:
    p = argparse.ArgumentParser()
    p.add_argument("input", help="decodetree-generated *.c.inc")
    p.add_argument("output", help="output stubs *.c.inc")
    p.add_argument(
        "--exclude",
        help="text file with one trans_* name per line; those names are skipped",
    )
    args = p.parse_args()

    excluded = set()
    if args.exclude:
        for line in Path(args.exclude).read_text().splitlines():
            line = line.strip()
            if line and not line.startswith("#"):
                excluded.add(line)

    src = Path(args.input).read_text()
    pat = re.compile(r"^static bool (trans_\w+)\(([^)]+)\);$", re.M)

    out = [
        "/* Auto-generated. Do not edit. */\n",
        "/* Stub trans_* definitions: emit `arm.unhandled` and ack the dispatch. */\n",
        "/* Excluded (hand-implemented in Semantics.def): "
        f"{len(excluded)} names */\n\n",
    ]
    skipped = 0
    for m in pat.finditer(src):
        name = m.group(1)
        if name in excluded:
            skipped += 1
            continue
        sig_args = m.group(2)
        mnemonic = name[len("trans_"):]
        out.append(f"static bool {name}({sig_args}) {{\n")
        out.append(f'    arm_lift_unhandled(ctx, "{mnemonic}");\n')
        out.append(f"    (void)a;\n")
        out.append(f"    return true;\n")
        out.append(f"}}\n\n")

    Path(args.output).write_text("".join(out))
    if args.exclude and skipped != len(excluded):
        sys.stderr.write(
            f"warning: exclude list has {len(excluded)} names but "
            f"only {skipped} were found in the decoder\n"
        )
    return 0


if __name__ == "__main__":
    sys.exit(main())
