import csv
import json

def convert_csv_to_json(input_csv, output_json):
    # Initialize an empty dictionary to hold the data
    data = {}
    
    # Open and read the CSV file
    with open(input_csv, 'r') as csvfile:
        # Read all lines, skipping those that start with '#'
        lines = [line.strip() for line in csvfile if not line.startswith('#')]
        
        # Use csv.reader to handle CSV parsing
        reader = csv.reader(lines)
        
        for row in reader:
            # Ensure the row has exactly 6 columns
            if len(row) != 6:
                print(f"Skipping malformed row: {row}")
                continue
            
            # Extract fields from the row
            run_fill = row[0]  # e.g., "273158:4915"
            run = run_fill.split(':')[0]  # Extract only the run number
            fill = run_fill.split(':')[1]  # Extract only the run number
            time = row[1]
            try:
                ncms = int(row[2])
                delivered = float(row[4])
                recorded = float(row[5].replace('\r', '').replace('\n', ''))
            except ValueError as e:
                print(f"Error parsing numeric values in row {row}: {e}")
                continue
            
            hltpath = row[3]
            
            # Create a record for this run
            record = {
                #"fill": fill,
                #"time": time,
                #"ncms": ncms,
                #"delivered": delivered,
                "recorded": recorded
            }
            
            # Add the record to the corresponding hltpath and run in the dictionary
            if hltpath not in data:
                data[hltpath] = {}
            if run not in data[hltpath]:
                data[hltpath][run] = []
            data[hltpath][run].append(record)
    
    # Write the dictionary to a JSON file with indentation for readability
    with open(output_json, 'w') as jsonfile:
        #json.dump(data, jsonfile, indent=1)
        json.dump(data, jsonfile)
    
    print(f"Conversion complete. JSON data saved to '{output_json}'.")

if __name__ == "__main__":

    inputs = [
            "HLT/LumiForTrig_2016Post_HLT_AK8PFJet.csv",
            "HLT/LumiForTrig_2017_HLT_AK8PFJet.csv",
            "HLT/LumiForTrig_2016Pre_HLT_AK8PFJet.csv",   
            "HLT/LumiForTrig_2018_HLT_AK8PFJet.csv"
            ]
    for input_csv in inputs:
        output_json = input_csv.replace("csv", "json")
        convert_csv_to_json(input_csv, output_json)

