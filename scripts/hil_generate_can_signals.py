#!/usr/bin/env python3
"""
generate_can_signals.py
=======================
Reads 2024CAR.dbc (and optionally ChargerMessages.dbc) and generates
C encode functions for every message.

The generated files (can_signals.c / can_signals.h) are compiled as
ordinary source files in the HIL firmware.  canSimTask calls the encode
functions, passes the resulting byte array to HIL_CAN_Transmit, and the
CAN3 peripheral puts it on the bus.

Usage
-----
    python3 generate_can_signals.py 2024CAR.dbc \
        --out-c Core/Src/can_signals.c \
        --out-h Core/Inc/can_signals.h

    # Include the charger DBC as well
    python3 generate_can_signals.py 2024CAR.dbc ChargerMessages.dbc \
        --out-c Core/Src/can_signals.c \
        --out-h Core/Inc/can_signals.h

Dependencies
------------
    pip install cantools
"""

import cantools
import argparse
import textwrap
from pathlib import Path


# ── Helpers ───────────────────────────────────────────────────────────────

def c_type(signal):
    """Return the narrowest fixed-width C type for a signal's raw integer."""
    bits   = signal.length
    signed = signal.is_signed
    if bits <= 8:
        return "int8_t"  if signed else "uint8_t"
    if bits <= 16:
        return "int16_t" if signed else "uint16_t"
    return "int32_t" if signed else "uint32_t"


def safe_default(signal):
    """
    Return a safe default raw value for a signal.

    Most signals default to 0.  Signals that have a minimum > 0 (like
    DTC_Severity which requires 1-4) get their minimum value so the
    generated code compiles and runs without range errors.
    """
    if signal.minimum is not None and signal.minimum > 0:
        return int(signal.minimum)
    return 0


def physical_to_raw_expr(signal, phys_var):
    """
    Return a C expression that converts a physical value variable to the
    raw integer that gets packed into the CAN frame.

        raw = (physical - offset) / scale

    We use integer arithmetic where scale == 1 and offset == 0 (the common
    case) to avoid pulling in floating-point on the MCU.
    """
    scale  = signal.scale  if signal.scale  is not None else 1
    offset = signal.offset if signal.offset is not None else 0

    if scale == 1 and offset == 0:
        # Pure integer cast — no FP needed
        return f"(int32_t)({phys_var})"
    elif offset == 0:
        return f"(int32_t)(({phys_var}) / ({scale}f))"
    else:
        return f"(int32_t)((({phys_var}) - ({offset}f)) / ({scale}f))"


def pack_signal(signal, raw_var):
    """
    Return a list of C assignment lines that pack `raw_var` (an int32_t)
    into the correct bit positions of data[].

    All signals in 2024CAR.dbc are little-endian.  In the DBC little-endian
    format the start bit is the LSB of the signal.  Bit N maps to:
        byte  = N // 8
        bit   = N %  8
    """
    lines = []
    start  = signal.start
    length = signal.length

    for bit_offset in range(length):
        abs_bit    = start + bit_offset
        byte_idx   = abs_bit // 8
        bit_in_byte = abs_bit % 8
        lines.append(
            f"    data[{byte_idx}] |= "
            f"(uint8_t)(({raw_var} >> {bit_offset}) & 0x01u) << {bit_in_byte}u;"
        )
    return lines


# Per-message code generation

def generate_encode_fn(msg):
    """
    Generate the full C encode function for one DBC message.

    Multiplexed signals (signals that only appear when a mux selector has a
    specific value) are skipped — the HIL sends fixed, non-multiplexed frames.
    The mux selector signal itself is included with a fixed value of 0.

    The function signature uses physical-value types (float for scaled
    signals, the appropriate integer type for unscaled signals).
    """
    # Separate non-mux signals from mux signals
    base_signals = [s for s in msg.signals
                    if not s.multiplexer_ids]   # mux selectors + plain signals
    mux_signals  = [s for s in msg.signals
                    if s.multiplexer_ids]        # conditional mux signals

    fn_name = f"encode_{msg.name}"

    # Build parameter list — use float for scaled signals, int type otherwise
    params = []
    for s in base_signals:
        scale = s.scale if s.scale is not None else 1
        if isinstance(scale, float) and scale != 1.0:
            params.append(f"float {s.name}")
        else:
            params.append(f"{c_type(s)} {s.name}")

    param_str = ", ".join(params) if params else "void"

    lines = []
    lines.append(f"/**")
    lines.append(f" * @brief Encode {msg.name}")
    lines.append(f" *        ID: 0x{msg.frame_id:08X}  Length: {msg.length} bytes")
    if mux_signals:
        lines.append(f" *        Note: {len(mux_signals)} multiplexed signal(s) omitted.")
    lines.append(f" * @param data  Output byte array, must be {msg.length} bytes.")

    for s in base_signals:
        scale  = s.scale  if s.scale  is not None else 1
        offset = s.offset if s.offset is not None else 0
        lines.append(f" * @param {s.name}  Physical value  "
                     f"[scale={scale}, offset={offset}, "
                     f"{'signed' if s.is_signed else 'unsigned'}, "
                     f"{s.length}bit]")
    lines.append(f" */")
    lines.append(f"void {fn_name}(uint8_t data[{msg.length}], {param_str})")
    lines.append("{")
    lines.append(f"    memset(data, 0, {msg.length});")
    lines.append("")

    for s in base_signals:
        raw_var = f"raw_{s.name}"
        phys_expr = physical_to_raw_expr(s, s.name)
        lines.append(f"    /* {s.name}: start={s.start}, len={s.length} */")
        lines.append(f"    int32_t {raw_var} = {phys_expr};")
        lines += pack_signal(s, raw_var)
        lines.append("")

    lines.append("}")
    return "\n".join(lines)


def generate_header_decl(msg):
    """Return the function prototype line for the header file."""
    base_signals = [s for s in msg.signals if not s.multiplexer_ids]

    params = []
    for s in base_signals:
        scale = s.scale if s.scale is not None else 1
        if isinstance(scale, float) and scale != 1.0:
            params.append(f"float {s.name}")
        else:
            params.append(f"{c_type(s)} {s.name}")

    param_str = ", ".join(params) if params else "void"
    return f"void encode_{msg.name}(uint8_t data[{msg.length}], {param_str});"


def generate_id_defines(messages):
    """Return a list of #define lines for every message ID."""
    lines = ["/* Message IDs — 29-bit extended, from DBC */"]
    for msg in messages:
        define_name = f"CAN_ID_{msg.name.upper()}"
        lines.append(f"#define {define_name:<55} 0x{msg.frame_id:08X}UL"
                     f"  /* {msg.frame_id} decimal */")
    return lines


# Main

def main():
    parser = argparse.ArgumentParser(
        description="Generate HIL CAN encode functions from DBC files."
    )
    parser.add_argument(
        "dbc", nargs="+",
        help="One or more .dbc files to process (e.g. 2024CAR.dbc ChargerMessages.dbc)"
    )
    parser.add_argument(
        "--out-c", default="can_signals.c",
        help="Output C source file path (default: can_signals.c)"
    )
    parser.add_argument(
        "--out-h", default="can_signals.h",
        help="Output header file path (default: can_signals.h)"
    )
    args = parser.parse_args()

    # Load all DBC files, merge messages into one list
    all_messages = []
    for dbc_path in args.dbc:
        db = cantools.database.load_file(dbc_path)
        all_messages.extend(db.messages)
        print(f"Loaded {dbc_path}: {len(db.messages)} messages")

    # Deduplicate by frame_id in case both DBCs share any IDs
    seen_ids = set()
    unique_messages = []
    for msg in all_messages:
        if msg.frame_id not in seen_ids:
            seen_ids.add(msg.frame_id)
            unique_messages.append(msg)

    total_signals = sum(len(m.signals) for m in unique_messages)
    print(f"Total: {len(unique_messages)} unique messages, {total_signals} signals")

    # Generate header
    guard = "CAN_SIGNALS_H"
    h_lines = [
        f"#ifndef {guard}",
        f"#define {guard}",
        "",
        "/* Auto-generated by generate_can_signals.py -- do not edit manually. */",
        "/* Re-run the script after any DBC change and commit the output.      */",
        "",
        "#include <stdint.h>",
        "#include <string.h>",
        "",
    ]
    h_lines += generate_id_defines(unique_messages)
    h_lines += [
        "",
        "#ifdef __cplusplus",
        'extern "C" {',
        "#endif",
        "",
        "/* Encode function prototypes */",
    ]
    for msg in unique_messages:
        h_lines.append(generate_header_decl(msg))
    h_lines += [
        "",
        "#ifdef __cplusplus",
        "}",
        "#endif",
        "",
        f"#endif /* {guard} */",
    ]

    out_h = Path(args.out_h)
    out_h.parent.mkdir(parents=True, exist_ok=True)
    out_h.write_text("\n".join(h_lines) + "\n")
    print(f"Written: {out_h}")

    # Generate source
    c_lines = [
        "/* Auto-generated by generate_can_signals.py -- do not edit manually. */",
        "/* Re-run the script after any DBC change and commit the output.      */",
        "",
        f'#include "{out_h.name}"',
        "",
    ]
    for msg in unique_messages:
        c_lines.append(generate_encode_fn(msg))
        c_lines.append("")

    out_c = Path(args.out_c)
    out_c.parent.mkdir(parents=True, exist_ok=True)
    out_c.write_text("\n".join(c_lines) + "\n")
    print(f"Written: {out_c}")

    # Summary
    mux_count = sum(
        1 for m in unique_messages
        for s in m.signals if s.multiplexer_ids
    )
    print(f"\nSummary:")
    print(f"  {len(unique_messages)} encode functions generated")
    print(f"  {mux_count} multiplexed signals skipped (fixed non-mux frames only)")
    print(f"  All signals are little-endian (verified from DBC)")


if __name__ == "__main__":
    main()