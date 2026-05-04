import json
import sys
import re
import os

def extract_text(data, keywords):
    """Recursively search for text containing keywords in a JSON object."""
    matches = []
    
    if isinstance(data, dict):
        for k, v in data.items():
            matches.extend(extract_text(v, keywords))
    elif isinstance(data, list):
        for item in data:
            matches.extend(extract_text(item, keywords))
    elif isinstance(data, str):
        # We found a string, check if it contains any of our keywords
        for keyword in keywords:
            if re.search(keyword, data, re.IGNORECASE):
                matches.append(data)
                break
                
    return matches

def scan_file(file_path, keywords):
    try:
        # Using encoding='utf-8' and errors='replace' to handle potential corruption
        with open(file_path, 'r', encoding='utf-8', errors='replace') as f:
            content = f.read()
            # Fast check before parsing JSON
            if not any(re.search(k, content, re.IGNORECASE) for k in keywords):
                return

            # Reset pointer and parse
            f.seek(0)
            data = json.load(f)
            
        results = extract_text(data, keywords)
        
        if results:
            print(f"[+] FOUND {len(results)} MATCHES IN: {file_path}")
            print("="*60)
            
            seen = set()
            for i, text in enumerate(results):
                if text in seen: continue
                seen.add(text)
                
                # Basic filter to skip very short snippets
                if len(text) < 20: continue
                
                print(f"--- Segment {i+1} ---")
                print(text.strip()[:500] + "..." if len(text) > 500 else text.strip())
                print("\n")
            print("="*60 + "\n")
            
    except (json.JSONDecodeError, UnicodeError):
        pass # Skip unreadable files
    except Exception as e:
        print(f"Error scanning {file_path}: {e}")

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 extract_design.py <path_to_scan>")
        sys.exit(1)
        
    path_arg = os.path.expanduser(sys.argv[1])
    
    # Keywords to search for relevant design discussion
    keywords = [
        r"\bbard\b", 
        r"\bventriloquate\b", 
        r"\bclass design\b", 
        r"\bproposed plan\b"
    ]
    
    print(f"Scanning {path_arg} for Bard design discussions...")

    if os.path.isfile(path_arg):
        scan_file(path_arg, keywords)
    else:
        for root, dirs, files in os.walk(path_arg):
            for file in files:
                if file.endswith(".json"):
                    scan_file(os.path.join(root, file), keywords)

if __name__ == "__main__":
    main()