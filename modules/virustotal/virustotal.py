import tomli
import sys
import argparse
import json
import vt
import time
import hashlib

# SELF MADE
import communication
import env


def command_line(config: str):
    parser = argparse.ArgumentParser();
    parser.add_argument("--config", type=str, help="Give a toml configuration file to parse [default: virus.toml]")
    arguments = parser.parse_args(sys.argv[1:]) 
    if arguments.config != None:
        config = arguments.config
    return config

def parse_configuration(config: str):
    config = command_line(config)
    
    with open(config, "rb") as config_file:
        configuration_read = tomli.load(config_file)
        
        return configuration_read

    return ""


def create_table(connection, table_name, columns):
    json_msg = {}
    json_msg["cmd"] = "create_table"
    json_msg["table_name"] = table_name
    json_msg["columns"]    = json.JSONDecoder().decode(columns) 

    communication.send_data(connection, json.dumps(json_msg))


def send_insert(connection, table_name, file, message):
    root = {}
    root["cmd"] = "insert_table"
    root["table_name"] = table_name
    column1 = {}
    column1["column_name"] = "scan_results"
    column1["information"] = message
    
    column2 = {}
    column2["column_name"] = "file"
    column2["information"] = file 

    root["columns"] = [column1, column2]
    
    communication.send_data(connection, json.dumps(root))

def send_to_scan(VTClient, exact_file):
    with open(exact_file, "rb") as file:
        hash_of_the_file = hashlib.sha256(file.read()).hexdigest()
        file.seek(0)
        analysis = VTClient.scan_file(file, wait_for_completion=True)
        file.close()
         
        return VTClient.get_object("/files/{}".format(hash_of_the_file))

def watch_files(connection, VTClient, files_and_dirs, table_name):
    import inotify.adapters
    i = inotify.adapters.Inotify()

    for file_or_dir in files_and_dirs:
        i.add_watch(file_or_dir)

    event_happen = 0
    for event in i.event_gen(yield_nones=False):
        (_, type_names, path, filename) = event

        for event_type in type_names:
            if (event_type == 'IN_MODIFY' or event_type == 'IN_MOVED_TO'):
                if '.part' in filename:
                    continue
                exact_file = path+filename
                
                print("Begin scan for: "+exact_file)
                analysis = send_to_scan(VTClient, exact_file)
                analysis_message = analysis.last_analysis_stats
                print("Scan done: ") 
                data_needed = {}
                data_needed['harmless'] = analysis_message['harmless']
                data_needed['undetected'] = analysis_message['undetected']
                data_needed['suspicious'] = analysis_message['suspicious']
                data_needed['malicious']  = analysis_message['malicious']
                print(data_needed)

                send_insert(connection, table_name, exact_file, json.dumps(data_needed))
    
def main():
    configuration = parse_configuration("./virus.toml")["virus"] 
    connection    = communication.client(configuration["pid_file"])
    create_table(connection, configuration["table_name"], configuration["create_table"])
    VTClient = vt.Client(env.API_KEY)
    watch_files(connection, VTClient, configuration["files"], configuration["table_name"]);

if __name__ == "__main__":
    main()
