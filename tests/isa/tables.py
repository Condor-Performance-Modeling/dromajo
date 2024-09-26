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
# r-type template
# ---------------------------------------------------------------------
#values = ["0", "rs2", "rs1", "0", "rd", "0"]
#print("## TBD")
#print("```")
#print("usage:   TBD rd, rs1, rs2")
#print("opc:     %s" % bin_to_hex(values[5]))
#print("func7:   %s" % bin_to_hex(values[0]))
#print("func3:   %s" % bin_to_hex(values[3]))
#print("ext:     ???")
#print("arch:    ???")
#print("pcode: ")
#print("         ???")
#print("")
#generate_table(rtype, values)
#print("```")
# ---------------------------------------------------------------------
# R and S have the same field sizes
rtype  = ["31:25", "24:20", "19:15", "14:12", "11:7", "6:0"]
# i64 widens the shamt field 
i64type = ["31:26", "25:20", "19:15", "14:12", "11:7", "6:0"]
i32type = ["31:25", "24:20", "19:15", "14:12", "11:7", "6:0"]

# Example
#print("# ZBC")
#values = ["0000101", "rs2", "rs1", "001", "rd", "0110011"]
#print("## CLMUL")
#print("```")
#print("usage:   clmul rd, rs1, rs2")
#print("opc:     %s" % bin_to_hex(values[5]))
#print("func7:   %s" % bin_to_hex(values[0]))
#print("func3:   %s" % bin_to_hex(values[3]))
#print("ext:     ZBC")
#print("arch:    RV32 RV64")
#print("pcode: ")
#print("         URV v1 = intRegs_.read(di->op1());")
#print("         URV v2 = intRegs_.read(di->op2());")
#print("       ")
#print("         URV x = 0;")
#print("         for (unsigned i = 0; i < mxlen_; ++i)")
#print("           if ((v2 >> i) & 1)")
#print("             x ^= v1 << i;")
#print("       ")
#print("         intRegs_.write(di->op0(), x);")
#print("")
#generate_table(rtype, values)
#print("```")
