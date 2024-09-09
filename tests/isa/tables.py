def generate_table(ranges):
    # Separate ranges into columns
    header = []
    border = []
    
    for r in ranges:
        if ':' in r:
            high, low = r.split(':')
            high, low = int(high), int(low)
            col_width = len(f"{high}   {low}")
            header.append(f"{high:<2}   {low:2}")
            border.append('-' * (col_width + 2))
        else:
            header.append(f"{r:^5}")
            border.append('-' * 5)
    
    border_line = '+' + '+'.join(border) + '+'
    content_line = border_line.replace('-', '.').replace('+', '|')  # Substituting "-" with ".", "+" with "|"
    empty_line = border_line.replace('-', ' ').replace('+', '|')     # Substituting "-" with " ", "+" with "|"
    
    # Build the table
    header_line = '|' + '|'.join(f"{col:^{len(b)}}" for col, b in zip(header, border)) + '|'
    
    # Print the table
    print(header_line)
    print(border_line)
    print(content_line)
    print(empty_line)
    print(border_line)


rtype = ["31:25", "24:20", "19:15", "14:12", "11:7", "6:0"]

#print("## MAX ")
generate_table(rtype)

#print("\n## MAXU")
#print("\n## BSET ")
#print("\n## BSETI ")
#print("\n## SH2ADD ")
#print("\n## SH1ADD ")
#print("\n## ANDN ")

