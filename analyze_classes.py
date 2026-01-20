import re
import os

file_path = "src/lib/magic/spell_parser.c"

classes = ["Magic User", "Cleric", "Thief", "Warrior", "Ranger", "Assassin", "Shou-Lin", "Solamnic Knight", "Death Knight", "Shadow Dancer", "Necromancer"]
class_codes = ["Mu", "Cl", "Th", "Wa", "Ra", "As", "Sl", "Kn", "Dk", "Sd", "Nm"]

class_spells = {c: [] for c in classes}

def parse_spells():
    with open(file_path, 'r') as f:
        content = f.read()

    lines = content.split('\n')
    current_spell = None
    
    for i, line in enumerate(lines):
        line = line.strip()
        if line.startswith('spello('):
            # Extract spell name
            match = re.search(r'spello\(([^,]+)', line)
            if match:
                current_spell = match.group(1).replace("SPELL_", "").replace("SKILL_", "").replace("SONG_", "")
                
                # Look ahead for the level definitions
                for j in range(1, 10): # Look at next few lines
                    if i+j >= len(lines): break
                    next_line = lines[i+j].strip()
                    if next_line.startswith('/*'): continue
                    
                    # Check if this line looks like the level definition
                    # It should contain XX or numbers separated by commas
                    if 'XX' in next_line or any(c.isdigit() for c in next_line):
                        parts = [p.strip() for p in next_line.split(',')]
                        # We need the first 11 values
                        if len(parts) >= 11:
                            for k in range(11):
                                val = parts[k]
                                if val != 'XX':
                                    try:
                                        lvl = int(val)
                                        class_spells[classes[k]].append((lvl, current_spell))
                                    except ValueError:
                                        pass 
                            break 

    # Sort spells by level for each class
    for c in classes:
        class_spells[c].sort(key=lambda x: x[0])

    # Output Markdown
    print("# Class Spell/Skill Map\n")
    for c in classes:
        print(f"## {c}")
        if not class_spells[c]:
            print("No spells/skills found.")
        else:
            print("| Level | Spell/Skill |")
            print("| :--- | :--- |")
            for lvl, spell in class_spells[c]:
                print(f"| {lvl} | {spell} |")
        print("\n")

if __name__ == "__main__":
    parse_spells()
