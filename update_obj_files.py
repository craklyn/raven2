import os
import glob
import re

def process_file(filepath):
    with open(filepath, 'r', encoding='latin-1') as f:
        lines = f.readlines()

    modified = False
    new_lines = []
    
    # Regex to match the flags line: 9 numbers separated by spaces
    # Example: 17 0 0 0 0 1 0 0 0
    flags_pattern = re.compile(r'^\s*(\d+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s+(\S+)\s*$')

    for line in lines:
        match = flags_pattern.match(line)
        if match:
            groups = list(match.groups())
            # groups[0] = Type
            # groups[1] = Extra1 (0-31)
            # groups[2] = Extra2 (32-63)
            # ...
            
            f1 = groups[1] # Extra1
            f2 = groups[2] # Extra2
            
            # Check conditions:
            # ANTI_MAGE (12) -> 'm'
            # ANTI_THIEF (14) -> 'o'
            # ANTI_NECRO (37) -> 'f' (in f2)
            
            # Note: 0 means no flags. '0' string.
            
            trigger = False
            
            if 'm' in f1 or 'o' in f1:
                trigger = True
            if 'f' in f2:
                trigger = True
                
            if trigger:
                # Add ANTI_ELEMANCER (58 -> 26 in f2 -> 'A')
                if 'A' not in f2:
                    if f2 == '0':
                        f2 = 'A'
                    else:
                        f2 = f2 + 'A'
                    
                    # Sort logic? Usually not required, but cleaner. 
                    # Assuming raw string append is fine. 
                    
                    groups[2] = f2
                    # Reconstruct line
                    new_line = " " + " ".join(groups) + "\n"
                    new_lines.append(new_line)
                    modified = True
                    continue

        new_lines.append(line)

    if modified:
        print(f"Updating {filepath}")
        with open(filepath, 'w', encoding='latin-1') as f:
            f.writelines(new_lines)

def main():
    files = glob.glob('sys/world/obj/*.obj')
    for file in files:
        process_file(file)

if __name__ == "__main__":
    main()
