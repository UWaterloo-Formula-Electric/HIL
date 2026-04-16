#!/usr/bin/env python3
"""
generate_can_signals.py
=======================
Reads 2024CAR.dbc (and optionally ChargerMessages.dbc) and generates
C encode functions for every message the VCU_F7 receives.

Approach matches the UWFE generateCANHeader.py:
  - C bitfield struct per message (with FILLER fields for gaps)
  - Value table enums for signals with choices
  - encode_<Msg> packs physical values into the struct
  - sendCAN_<Msg> calls HIL_CAN_Transmit internally

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
import operator
from pathlib import Path


# ── Type helpers ──────────────────────────────────────────────────────────

def raw_c_type(signal):
    """C integer type for the signal's raw (unscaled) value inside the struct."""
    if signal.is_signed:
        return "int64_t"
    return "uint64_t"

def phys_c_type(signal):
    """C type for the physical value passed to the encode function."""
    if signal.scale is not None and signal.scale != 1:
        return "float"
    if signal.is_signed:
        return "int64_t"
    return "uint64_t"


# ── Struct generation (matches UWFE writeStructForMsg) ────────────────────

def generate_struct(msg):
    """
    Generate a packed C bitfield struct for a message.

    Signals are sorted by start bit. Gaps between signals become FILLER
    fields. This matches what UWFE's generateCANHeader.py produces and lets
    the compiler handle all bit layout — no manual bit manipulation needed.
    Mux-conditional signals (multiplexer_ids set) are included in the struct
    using the stripped signal name; the mux selector itself is also included.
    """
    lines = []
    lines.append(f"struct {msg.name} {{")

    signals = sorted(msg.signals, key=operator.attrgetter('start'))
    pos = 0
    seen_starts = set()
    mux_count = 1

    for sig in signals:
        if sig.start in seen_starts:
            continue
        seen_starts.add(sig.start)

        # Fill any gap before this signal
        if sig.start > pos:
            lines.append(f"    uint64_t FILLER_{pos} : {sig.start - pos};")

        # Name: strip trailing digits for mux-conditional signals
        name = sig.name
        if sig.multiplexer_ids is not None:
            import re
            name = re.sub(r'\d+$', '', name) + str(mux_count)
            mux_count += 1

        if sig.is_signed:
            lines.append(f"    int64_t  {name} : {sig.length};")
        else:
            lines.append(f"    uint64_t {name} : {sig.length};")

        pos = sig.start + sig.length

    # Pad to full message length
    total_bits = msg.length * 8
    if pos < total_bits:
        lines.append(f"    uint64_t FILLER_END : {total_bits - pos};")

    lines.append("};")
    lines.append("")
    return "\n".join(lines)


# ── Value table enums (matches UWFE writeValueTableEnum) ─────────────────

def generate_enum(signal):
    """
    Generate a C enum for a signal that has named values (choices).
    Matches the UWFE approach: enum <SignalName>_Values { <Name> = <Value> }.
    """
    if not signal.choices:
        return None
    lines = []
    lines.append(f"typedef enum {{")
    for value, name in signal.choices.items():
        safe_name = str(name).replace(' ', '_').replace('/', '_')
        lines.append(f"    {signal.name}_{safe_name} = {value},")
    lines.append(f"}} {signal.name}_Values;")
    lines.append("")
    return "\n".join(lines)


# ── Encode function (struct-based, matches UWFE approach) ─────────────────

def generate_encode_fn(msg):
    """
    Generate an encode function that:
      1. Takes physical signal values as arguments
      2. Builds a zero-initialised struct instance
      3. Assigns the raw (unscaled) value of each signal into the struct field
      4. Memcpys the struct into the caller-supplied data[] array

    Using memcpy instead of a pointer cast avoids strict-aliasing UB in C99.
    The struct bitfield layout is identical to the UWFE approach.

    Mux-conditional signals are excluded from the parameter list (the HIL
    sends fixed non-mux frames). The mux selector field is set to 0.
    """
    # Only base (non-mux-conditional) signals as parameters
    base_sigs = [s for s in msg.signals if not s.multiplexer_ids]
    # Mux selector signals are in base_sigs already (they have multiplexer_ids=None)

    fn_name = f"encode_{msg.name}"
    params = ", ".join(
        f"{phys_c_type(s)} {s.name}" for s in base_sigs
    )
    if not params:
        params = "void"

    lines = []
    lines.append(f"/**")
    lines.append(f" * @brief  Encode {msg.name}")
    lines.append(f" *         ID: 0x{msg.frame_id:08X}  ({msg.frame_id})  Length: {msg.length}B")
    lines.append(f" * @param  data  Output buffer, must be >= {msg.length} bytes")

    for s in base_sigs:
        scale  = s.scale  if s.scale  is not None else 1
        offset = s.offset if s.offset is not None else 0
        lines.append(
            f" * @param  {s.name:<40} "
            f"[scale={scale}, offset={offset}, "
            f"{'signed' if s.is_signed else 'unsigned'}, {s.length}bit]"
        )
    lines.append(f" */")
    lines.append(f"void {fn_name}(uint8_t data[{msg.length}], {params})")
    lines.append("{")
    lines.append(f"    struct {msg.name} frame = {{0}};")
    lines.append("")

    for s in base_sigs:
        scale  = s.scale  if s.scale  is not None else 1
        offset = s.offset if s.offset is not None else 0

        if scale == 1 and offset == 0:
            # Integer signal — cast directly
            raw_expr = f"({raw_c_type(s)})({s.name})"
        elif offset == 0:
            raw_expr = f"({raw_c_type(s)})(({s.name}) / ({scale}f))"
        else:
            if isinstance(offset, int): # if offset is an int, add .0 to ensure float division in C
                raw_expr = f"({raw_c_type(s)})((({s.name}) - ({offset}.0f)) / ({scale}f))" 
            else:
                raw_expr = f"({raw_c_type(s)})((({s.name}) - ({offset}f)) / ({scale}f))"

        lines.append(f"    frame.{s.name} = {raw_expr};")

    lines.append(f"    memcpy(data, &frame, {msg.length});")
    lines.append("}")
    return "\n".join(lines)


# ── sendCAN_ wrapper (matches UWFE writeMessageSendFunction) ──────────────

def generate_send_fn(msg):
    """
    Generate a sendCAN_<Msg>() function that calls HIL_CAN_Transmit.

    The caller sets signal values in gHILState, then canSimTask calls
    sendCAN_<Msg>(gHILState.field, ...) — this matches the UWFE pattern
    where sendCAN_X() reads from global variables and calls sendCanMessage().

    For the HIL the physical values come in as parameters rather than globals
    so canSimTask controls exactly which values go out.
    """
    base_sigs = [s for s in msg.signals if not s.multiplexer_ids]
    params = ", ".join(
        f"{phys_c_type(s)} {s.name}" for s in base_sigs
    )
    if not params:
        params = "void"

    arg_names = ", ".join(s.name for s in base_sigs)

    lines = []
    lines.append(f"int sendCAN_{msg.name}({params})")
    lines.append("{")
    lines.append(f"    uint8_t data[{msg.length}];")
    lines.append(f"    encode_{msg.name}(data, {arg_names});")
    lines.append(
        f"    return HIL_CAN_Transmit(&hcan3, "
        f"0x{msg.frame_id:08X}UL, {msg.length}, data);"
    )
    lines.append("}")
    return "\n".join(lines)


# ── Header declarations ───────────────────────────────────────────────────

def generate_encode_decl(msg):
    base_sigs = [s for s in msg.signals if not s.multiplexer_ids]
    params = ", ".join(
        f"{phys_c_type(s)} {s.name}" for s in base_sigs
    ) or "void"
    return f"void encode_{msg.name}(uint8_t data[{msg.length}], {params});"

def generate_send_decl(msg):
    base_sigs = [s for s in msg.signals if not s.multiplexer_ids]
    params = ", ".join(
        f"{phys_c_type(s)} {s.name}" for s in base_sigs
    ) or "void"
    return f"int sendCAN_{msg.name}({params});"

def generate_id_define(msg):
    return (
        f"#define CAN_ID_{msg.name.upper():<50} "
        f"0x{msg.frame_id:08X}UL  /* {msg.frame_id} */"
    )


# ── Configuration — update these paths if files move ──────────────────────

DBC_PATH = r"C:\Users\Joeyl\OneDrive\Desktop\Coding Projects\UWFE\HIL\HIL_2026\Data\2024CAR.dbc"

OUT_C = r"C:\Users\Joeyl\OneDrive\Desktop\Coding Projects\UWFE\HIL\HIL_2026\Core\Src\can_signals.c"
OUT_H = r"C:\Users\Joeyl\OneDrive\Desktop\Coding Projects\UWFE\HIL\HIL_2026\Core\Inc\can_signals.h"

# ── Main ──────────────────────────────────────────────────────────────────

def main():
    # Load and merge all DBC files
    db = cantools.database.load_file(DBC_PATH)
    all_messages = db.messages
    print(f"Loaded {DBC_PATH}: {len(db.messages)} messages")

    # Deduplicate by frame_id
    seen_ids = set()
    unique_messages = []
    for msg in all_messages:
        if msg.frame_id not in seen_ids:
            seen_ids.add(msg.frame_id)
            unique_messages.append(msg)

    # Filter to VCU_F7 receivers only.
    # Step 1: keep messages where at least one signal lists VCU_F7.
    # Step 2: within each kept message, drop signals not received by VCU_F7
    #         so encode functions only pack signals the VCU actually uses.
    TARGET_NODE = "VCU_F7"

    class FilteredMsg:
        """Thin wrapper around a cantools Message with a filtered signal list."""
        def __init__(self, m, sigs):
            self.name     = m.name
            self.frame_id = m.frame_id
            self.length   = m.length
            self.signals  = sigs

    vcu_messages = []
    total_sigs_dropped = 0
    for msg in unique_messages:
        vcu_sigs = [s for s in msg.signals if TARGET_NODE in s.receivers]
        if not vcu_sigs:
            continue
        dropped = len(msg.signals) - len(vcu_sigs)
        total_sigs_dropped += dropped
        if dropped:
            vcu_messages.append(FilteredMsg(msg, vcu_sigs))
        else:
            vcu_messages.append(msg)

    skipped = len(unique_messages) - len(vcu_messages)
    total_signals = sum(len(m.signals) for m in vcu_messages)
    print(f"Kept {len(vcu_messages)} messages received by {TARGET_NODE} "
          f"(skipped {skipped})")
    print(f"Dropped {total_sigs_dropped} signals not received by {TARGET_NODE}")
    print(f"Total: {len(vcu_messages)} messages, {total_signals} signals")

    # Collect all unique enums across all messages (avoid duplicates for
    # shared value tables like DC_Channel used by multiple PDU signals)
    seen_enums = set()

    # ── Generate header ───────────────────────────────────────────────────
    guard = "CAN_SIGNALS_H"
    h_lines = [
        f"#ifndef {guard}",
        f"#define {guard}",
        "",
        "/* Auto-generated by generate_can_signals.py — do not edit manually. */",
        "/* Re-run after any DBC change and commit the output.                */",
        "",
        "#include <stdint.h>",
        "#include <string.h>",
        "#include \"can.h\"        /* hcan3 */",
        "",
        "/* ── Message ID defines ───────────────────────────────────────────── */",
    ]
    for msg in vcu_messages:
        h_lines.append(generate_id_define(msg))

    h_lines += [
        "",
        "/* ── Value table enums ────────────────────────────────────────────── */",
    ]
    for msg in vcu_messages:
        for sig in msg.signals:
            if sig.choices and sig.name not in seen_enums:
                seen_enums.add(sig.name)
                enum_str = generate_enum(sig)
                if enum_str:
                    h_lines.append(enum_str)

    h_lines += [
        "/* ── Function prototypes ──────────────────────────────────────────── */",
        "#ifdef __cplusplus",
        'extern "C" {',
        "#endif",
        "",
    ]
    for msg in vcu_messages:
        h_lines.append(generate_encode_decl(msg))
        h_lines.append(generate_send_decl(msg))
        h_lines.append("")

    h_lines += [
        "#ifdef __cplusplus",
        "}",
        "#endif",
        "",
        f"#endif /* {guard} */",
    ]

    out_h = Path(OUT_H)
    out_h.parent.mkdir(parents=True, exist_ok=True)
    out_h.write_text("\n".join(h_lines) + "\n", encoding='utf-8') # need to specify encoding to avoid ascii errors on '-' character
    print(f"Written: {out_h}")

    # ── Generate source ───────────────────────────────────────────────────
    c_lines = [
        "/* Auto-generated by generate_can_signals.py — do not edit manually. */",
        "/* Re-run after any DBC change and commit the output.                */",
        "",
        f'#include "{out_h.name}"',
        "",
    ]
    for msg in vcu_messages:
        c_lines.append(f"/* ── {msg.name} ── */")
        c_lines.append(generate_struct(msg))
        c_lines.append(generate_encode_fn(msg))
        c_lines.append("")
        c_lines.append(generate_send_fn(msg))
        c_lines.append("")

    out_c = Path(OUT_C)
    out_c.parent.mkdir(parents=True, exist_ok=True)
    out_c.write_text("\n".join(c_lines) + "\n", encoding='utf-8')
    print(f"Written: {out_c}")

    mux_skipped = sum(
        1 for m in vcu_messages for s in m.signals if s.multiplexer_ids
    )
    enum_count = len(seen_enums)
    print(f"\nSummary:")
    print(f"  {len(vcu_messages)} messages")
    print(f"  {mux_skipped} mux-conditional signals excluded from encode params")
    print(f"  {enum_count} value table enums generated")
    print(f"  Struct-based packing (matches UWFE generateCANHeader.py)")


if __name__ == "__main__":
    main()