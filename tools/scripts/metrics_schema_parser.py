#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 권경환 Kyunghwan Kwon <k@libmcu.org>
#
# SPDX-License-Identifier: MIT
#
# metrics_schema_parser.py
#
# Parses a metrics.def file and outputs the schema as JSON.
# Used by the server to obtain metric type metadata for rendering.
#
# Usage:
#   python3 metrics_schema_parser.py metrics.def
#   python3 metrics_schema_parser.py metrics.def --format json
#   python3 metrics_schema_parser.py metrics.def --format csv

import re
import sys
import json
import argparse
from dataclasses import dataclass, asdict
from typing import Optional


INT32_MIN = -(2**31)
INT32_MAX = 2**31 - 1


@dataclass
class MetricSchema:
    key: int       # numeric index in .def order (matches CBOR/wire key)
    label: str     # human-readable name from .def
    type: str
    min: Optional[int] = None
    max: Optional[int] = None
    unit: Optional[str] = None


# Maps macro name → (type string, fixed min, fixed max, has_range_args, has_unit_arg)
MACRO_DEFS = {
    "METRICS_DEFINE":            ("untyped",    INT32_MIN, INT32_MAX, False, False),
    "METRICS_DEFINE_COUNTER":    ("counter",    0,         INT32_MAX, False, False),
    "METRICS_DEFINE_GAUGE":      ("gauge",      None,      None,      True,  False),
    "METRICS_DEFINE_PERCENTAGE": ("percentage", 0,         100,       False, False),
    "METRICS_DEFINE_TIMER":      ("timer",      0,         INT32_MAX, False, True),
    "METRICS_DEFINE_BYTES":      ("bytes",      0,         INT32_MAX, False, False),
}


def _parse_int(s: str) -> int:
    s = s.strip()
    if s in ("INT32_MAX", "INT_MAX"):
        return INT32_MAX
    if s in ("INT32_MIN", "INT_MIN"):
        return INT32_MIN
    return int(s, 0)


def parse_def(path: str) -> list[MetricSchema]:
    schemas: list[MetricSchema] = []
    pattern = re.compile(
        r'^\s*(METRICS_DEFINE(?:_\w+)?)\s*\(\s*(\w+)(?:\s*,([^)]*))?\)'
    )

    with open(path, encoding="utf-8") as f:
        index = 0
        for line in f:
            line = line.split("//")[0].strip()  # strip comments
            if not line:
                continue
            m = pattern.match(line)
            if not m:
                continue

            macro = m.group(1)
            label = m.group(2)
            rest = m.group(3)

            if macro not in MACRO_DEFS:
                continue

            type_name, fixed_min, fixed_max, has_range, has_unit = MACRO_DEFS[macro]
            args = [a.strip() for a in rest.split(",")] if rest else []

            entry = MetricSchema(key=index, label=label, type=type_name)
            index += 1

            if has_range and len(args) >= 2:
                entry.min = _parse_int(args[0])
                entry.max = _parse_int(args[1])
            else:
                if fixed_min is not None:
                    entry.min = fixed_min
                if fixed_max is not None:
                    entry.max = fixed_max

            if has_unit and args:
                entry.unit = args[0].strip().lower()

            schemas.append(entry)

    return schemas


def to_json(schemas: list[MetricSchema]) -> str:
    return json.dumps(
        [{k: v for k, v in asdict(s).items() if v is not None} for s in schemas],
        indent=2,
    )


def to_csv(schemas: list[MetricSchema]) -> str:
    lines = ["key,label,type,min,max,unit"]
    for s in schemas:
        lines.append(
            f"{s.key},{s.label},{s.type},"
            f"{'' if s.min is None else s.min},"
            f"{'' if s.max is None else s.max},"
            f"{'' if s.unit is None else s.unit}"
        )
    return "\n".join(lines)


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Parse a metrics.def file and output the schema."
    )
    parser.add_argument("def_file", help="Path to metrics.def")
    parser.add_argument(
        "--format",
        choices=["json", "csv"],
        default="json",
        help="Output format (default: json)",
    )
    args = parser.parse_args()

    try:
        schemas = parse_def(args.def_file)
    except FileNotFoundError:
        print(f"error: file not found: {args.def_file}", file=sys.stderr)
        sys.exit(1)

    if not schemas:
        print("warning: no metrics found", file=sys.stderr)

    if args.format == "csv":
        print(to_csv(schemas))
    else:
        print(to_json(schemas))


if __name__ == "__main__":
    main()
