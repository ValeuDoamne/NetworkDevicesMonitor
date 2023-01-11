# VirusTotal

This module is written in Python to show the versatility for the project and the VirusTotal official API is written for it.

This module can monitor directories for file downloads and send the downloaded files to VirusTotal for scanning.
The scan results will be stored in the database.

## Building
To run this module the packages for python must be downloaded using the command
```
pip install -r requirements.txt
```

For running the program is:
```
python virustotal.py
```

## Configuration

The file `env.py` must contain the VirusTotal API key.

The configuration of the program uses TOML files.
An example:
```
[virus]
pid_file = "/tmp/agent.pid"
timeout  = 1
files    = ["/home/cosmin/Downloads/"]
table_name = "virustotal"

create_table = '''[
		          {"column_name" : "file",    "type": "TEXT"}, 
		          {"column_name" : "scan_results", "type": "TEXT"} 
		 ]'''
```
