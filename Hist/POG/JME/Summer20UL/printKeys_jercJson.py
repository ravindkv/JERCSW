import sys
import gzip
import json

def find_edges(obj, path=""):
    """
    Recursively search obj for 'edges' keys.
    Whenever one is found, yield its JSON path and its value.
    """
    if isinstance(obj, dict):
        for k, v in obj.items():
            new_path = f"{path}.{k}" if path else k
            if k == "edges":
                yield new_path, v
            yield from find_edges(v, new_path)
    elif isinstance(obj, list):
        for idx, item in enumerate(obj):
            new_path = f"{path}[{idx}]"
            yield from find_edges(item, new_path)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python print_edges.py jet_jerc.json[.gz] [edge]")
        sys.exit(1)

    filename = sys.argv[1]
    # if the user passed "edge" as a second argument, we print edges too
    print_edges = len(sys.argv) >= 3 and sys.argv[2].lower() == "edge"

    open_func = gzip.open if filename.endswith(".gz") else open
    with open_func(filename, "rt") as f:
        data = json.load(f)

    # compute column width for the original name-printing
    name_width = max(len(corr.get('name', "")) for corr in data.get('corrections', []))

    for corr in data.get("corrections", []):
        name = corr.get("name", "<no-name>")
        input_names = ", ".join(inp.get("name", "") for inp in corr.get("inputs", []))
        if print_edges: print(f"\n{name:<{name_width}} : {input_names}")
        else: print(f"{name:<{name_width}} : {input_names}")

        if print_edges:
            for path, edges in find_edges(corr):
                print(f"    {path} : {edges}")

