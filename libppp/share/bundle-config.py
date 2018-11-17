import os
import json
import base64


THIS_DIR = os.path.dirname(os.path.realpath(__file__))

def expand_node(node):
    if not isinstance(node, dict):
        return
    for key in node:
        if key == 'file' and not node.get('data', ''):
            file_name = node['file']
            file_path = os.path.join(THIS_DIR, file_name)
            with open(file_path, 'rb') as fp:
                content = base64.b64encode(fp.read()).decode('ascii')
            node['data'] = content
        else:
            expand_node(node[key])

def bundle_config():
    config_input_file = os.path.join(THIS_DIR, 'config.json')
    with open(config_input_file) as fp:
        config_data = json.load(fp)

    for key in config_data:
        expand_node(config_data[key])

    config_bundle_file = os.path.join(THIS_DIR, 'config.bundle.json')
    with open(config_bundle_file, 'w') as fp:
        json.dump(config_data, fp)

if __name__ == '__main__':
    bundle_config()