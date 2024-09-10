def bin_to_hex(binary_str):
    # Convert binary string to integer
    decimal_value = int(binary_str, 2)
    # Convert integer to hexadecimal and prefix with '0x'
    hex_value = hex(decimal_value)
    return hex_value

def generate_table(ranges, values):
    # Convert hex_value to binary string, padded
    total_bits = sum(int(high) - int(low) + 1
                     for high, low in (r.split(':') for r
                     in ranges if ':' in r))
    total_bits += sum(1 for r in ranges if ':' not in r)

    # Separate ranges into columns
    header = []
    border = []
    binary_values = []
    hex_values = []
    current_index = 0

    for r, value in zip(ranges, values):
        if ':' in r:
            high, low = r.split(':')
            high, low = int(high), int(low)
            bit_length = high - low + 1
            col_width = len(f"{high}   {low}")
            header.append(f"{high:<2}   {low:2}")
            border.append('-' * (col_width + 2))

            # Extract the corresponding value for this range
            binary_values.append(f"{value:^{col_width}}")

            # If value is binary, convert to hex and store
            if all(c in '01' for c in value):
                hex_val = f"0x{int(value, 2):X}"
                hex_values.append(f"{hex_val:^{col_width}}")
            else:
                hex_values.append(' ' * col_width)
            
            current_index += bit_length
        else:
            header.append(f"{r:^5}")
            border.append('-' * 5)

            # Extract the corresponding value for a single bit
            binary_values.append(f"{value:^5}")

            # If value is binary, convert to hex and store
            if all(c in '01' for c in value):
                hex_val = f"0x{int(value, 2):X}"
                hex_values.append(f"{hex_val:^5}")
            else:
                hex_values.append(' ' * 5)

            current_index += 1

    # Build the table rows
    border_line = '+' + '+'.join(border) + '+'
    content_line = border_line.replace('-', '.').replace('+', '|')
    empty_line = border_line.replace('-', ' ').replace('+', '|')

    # Build the header, binary value, and hex value lines
    header_line = '|' + '|'.join(f"{col:^{len(b)}}" \
                  for col, b in zip(header, border)) + '|'
    binary_line = '|' + '|'.join(f"{binary_val:^{len(b)}}"  \
                   for binary_val, b in zip(binary_values, border)) + '|'
    hex_line    = '|' + '|'.join(f"{hex_val:^{len(b)}}" \
                   for hex_val, b in zip(hex_values, border)) + '|'

    print(header_line)
    print(border_line)
    print(binary_line)
    print(hex_line)
    #print(empty_line)
    print(border_line)

# ---------------------------------------------------------------------
# ---------------------------------------------------------------------
# R and S have the same field sizes
rtype  = ["31:25", "24:20", "19:15", "14:12", "11:7", "6:0"]

print("# ZBB")

values = ["0100000", "rs2", "rs1", "111", "rd", "0110011"]
print("## ANDN   ZBB  RV32 RV64")
print("'''")
print("usage:   andn rd, rs1, rs2")
print("opc:     %s" % bin_to_hex(values[5]))
print("func7:   %s" % bin_to_hex(values[0]))
print("func3:   %s" % bin_to_hex(values[3]))
print("pcode: ")
print("    X(rd) = X(rs1) & ~X(rs2);")
print("")
generate_table(rtype, values)
print("'''")

values = ["0110000", "00000", "rs1", "001", "rd", "0010011"]
print("## CLZ   ZBB  RV32 RV64")
print("'''")
print("usage:   clz rd, rs ")
print("opc:     %s" % bin_to_hex(values[5]))
print("func7:   %s" % bin_to_hex(values[0]))
print("func3:   %s" % bin_to_hex(values[3]))
print("pcode: ")
print("    COUNT LEADING ZEROS")
print("")
generate_table(rtype, values)
print("'''")

values = ["0110000", "00000", "rs1", "001", "rd", "0011011"]
print("## CLZW   ZBB  RV64")
print("'''")
print("usage:   clzw rd, rs ")
print("opc:     %s" % bin_to_hex(values[5]))
print("func7:   %s" % bin_to_hex(values[0]))
print("func3:   %s" % bin_to_hex(values[3]))
print("pcode: ")
print("    COUNT LEADING ZEROS WORD")
print("")
generate_table(rtype, values)
print("'''")

values = ["0110000", "00010", "rs1", "001", "rd", "0010011"]
print("## CPOP   ZBB  RV32 RV64")
print("'''")
print("usage:   cpop rd, rs ")
print("opc:     %s" % bin_to_hex(values[5]))
print("func7:   %s" % bin_to_hex(values[0]))
print("func3:   %s" % bin_to_hex(values[3]))
print("pcode: ")
print("    COUNT ONES")
print("")
generate_table(rtype, values)
print("'''")

values = ["0110000", "00010", "rs1", "001", "rd", "0011011"]
print("## CPOPW   ZBB  RV64")
print("'''")
print("usage:   cpopw rd, rs ")
print("opc:     %s" % bin_to_hex(values[5]))
print("func7:   %s" % bin_to_hex(values[0]))
print("func3:   %s" % bin_to_hex(values[3]))
print("pcode: ")
print("    COUNT ONES WORD")
print("")
generate_table(rtype, values)
print("'''")

values = ["0110000", "00001", "rs1", "001", "rd", "0010011"]
print("## CTZ   ZBB  RV32 RV64")
print("'''")
print("usage:   ctz rd, rs ")
print("opc:     %s" % bin_to_hex(values[5]))
print("func7:   %s" % bin_to_hex(values[0]))
print("func3:   %s" % bin_to_hex(values[3]))
print("pcode: ")
print("    COUNT TRAILING ZEROS")
print("")
generate_table(rtype, values)
print("'''")

values = ["0110000", "00001", "rs1", "001", "rd", "0011011"]
print("## CTZW   ZBB  RV64")
print("'''")
print("usage:   ctzw rd, rs ")
print("opc:     %s" % bin_to_hex(values[5]))
print("func7:   %s" % bin_to_hex(values[0]))
print("func3:   %s" % bin_to_hex(values[3]))
print("pcode: ")
print("    COUNT TRAILING ZEROS WORD")
print("")
generate_table(rtype, values)
print("'''")

values = ["0000101", "rs2", "rs1", "110", "rd", "0110011"]
print("## max   ZBB  RV32 RV64")
print("'''")
print("usage:   max rd, rs1, rs2 ")
print("opc:     %s" % bin_to_hex(values[5]))
print("func7:   %s" % bin_to_hex(values[0]))
print("func3:   %s" % bin_to_hex(values[3]))
print("pcode: ")
print("    X(rd) = X(rs1) <s X(rs2) ? X(rs2) : X(rs1)")
print("")
generate_table(rtype, values)
print("'''")

values = ["0000101", "rs2", "rs1", "111", "rd", "0110011"]
print("## maxu   ZBB  RV32 RV64")
print("'''")
print("usage:   maxu rd, rs1, rs2 ")
print("opc:     %s" % bin_to_hex(values[5]))
print("func7:   %s" % bin_to_hex(values[0]))
print("func3:   %s" % bin_to_hex(values[3]))
print("pcode: ")
print("    X(rd) = X(rs1) <u X(rs2) ? X(rs2) : X(rs1)")
print("")
generate_table(rtype, values)
print("'''")

values = ["0000101", "rs2", "rs1", "100", "rd", "0110011"]
print("## min   ZBB  RV32 RV64")
print("'''")
print("usage:   min rd, rs1, rs2 ")
print("opc:     %s" % bin_to_hex(values[5]))
print("func7:   %s" % bin_to_hex(values[0]))
print("func3:   %s" % bin_to_hex(values[3]))
print("pcode: ")
print("    X(rd) = X(rs1) <s X(rs2) ? X(rs1) : X(rs2)")
print("")
generate_table(rtype, values)
print("'''")

values = ["0000101", "rs2", "rs1", "101", "rd", "0110011"]
print("## minu   ZBB  RV32 RV64")
print("'''")
print("usage:   minu rd, rs1, rs2 ")
print("opc:     %s" % bin_to_hex(values[5]))
print("func7:   %s" % bin_to_hex(values[0]))
print("func3:   %s" % bin_to_hex(values[3]))
print("pcode: ")
print("    X(rd) = X(rs1) <u X(rs2) ? X(rs1) : X(rs2)")
print("")
generate_table(rtype, values)
print("'''")

values = ["0100000", "rs2", "rs1", "110", "rd", "0110011"]
print("## orn   ZBB  RV32 RV64")
print("'''")
print("usage:   orn rd, rs1, rs2 ")
print("opc:     %s" % bin_to_hex(values[5]))
print("func7:   %s" % bin_to_hex(values[0]))
print("func3:   %s" % bin_to_hex(values[3]))
print("pcode: ")
print("    X(rd) = X(rs1) | ~X(rs2)")
print("")
generate_table(rtype, values)
print("'''")

values = ["0110000", "00100", "rs1", "001", "rd", "0010011"]
print("## sext.b   ZBB  RV32 RV64")
print("'''")
print("usage:   sext.b rd, rs")
print("opc:     %s" % bin_to_hex(values[5]))
print("func7:   %s" % bin_to_hex(values[0]))
print("func3:   %s" % bin_to_hex(values[3]))
print("pcode: ")
print("    SIGN EXTEND BYTE");
print("")
generate_table(rtype, values)
print("'''")

values = ["0110000", "00101", "rs1", "001", "rd", "0010011"]
print("## sext.h   ZBB  RV32 RV64")
print("'''")
print("usage:   sext.h rd, rs")
print("opc:     %s" % bin_to_hex(values[5]))
print("func7:   %s" % bin_to_hex(values[0]))
print("func3:   %s" % bin_to_hex(values[3]))
print("pcode: ")
print("         X(rd) = EXTS(X(rs)[15..0]);");
print("")
generate_table(rtype, values)
print("'''")

values = ["0100000", "rs2", "rs1", "100", "rd", "0110011"]
print("## xnor   ZBB  RV32 RV64")
print("'''")
print("usage:   xnor rd, rs1, rs2")
print("opc:     %s" % bin_to_hex(values[5]))
print("func7:   %s" % bin_to_hex(values[0]))
print("func3:   %s" % bin_to_hex(values[3]))
print("pcode: ")
print("         X(rd) = ~(X(rs1) ^ X(rs2));");
print("")
generate_table(rtype, values)
print("'''")

values = ["0000100", "00000", "rs1", "100", "rd", "0111011"]
print("## zext.h   ZBB  RV32 RV64")
print("'''")
print("usage:   zext.h rd, rs")
print("opc:     %s" % bin_to_hex(values[5]))
print("func7:   %s" % bin_to_hex(values[0]))
print("func3:   %s" % bin_to_hex(values[3]))
print("pcode: ")
print("         X(rd) = EXTZ(X(rs)[15..0]);");
print("")
generate_table(rtype, values)
print("'''")

