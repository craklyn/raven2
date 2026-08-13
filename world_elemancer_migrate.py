import os
import glob

def process_file(path):
    with open(path, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()

    lines = content.split('\n')
    modified = False
    
    for i in range(len(lines)):
        line = lines[i].strip()
        parts = line.split()
        if len(parts) == 9:
            valid = True
            ints = []
            for p in parts:
                try:
                    ints.append(int(p))
                except ValueError:
                    valid = False
                    break
            
            if valid:
                extra0 = ints[1]
                extra1 = ints[2]
                
                # Check ITEM_ANTI_THIEF (bit 14)
                if (extra0 & (1 << 14)) != 0:
                    # Set ITEM_ANTI_ELEMANCER (bit 58 -> bit 26 in extra1)
                    if (extra1 & (1 << 26)) == 0:
                        ints[2] |= (1 << 26)
                        # We reconstruct the line
                        lines[i] = " ".join(map(str, ints))
                        modified = True

    if modified:
        with open(path, 'w', encoding='utf-8') as f:
            f.write('\n'.join(lines))
        print(f"Updated {path}")

if __name__ == "__main__":
    count = 0
    files = glob.glob('sys/world/obj/*.obj')
    print(f"Checking {len(files)} object files...")
    for f in files:
        process_file(f)
        count += 1
    print("World object migration complete.")
