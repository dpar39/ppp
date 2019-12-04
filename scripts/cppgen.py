import os
import json
import argparse
from string import Template
import datetime

TEMPLATES = {
    "class": {
        "description": "Generates a C++ interface, implementation class and test suite",
        "files": [
            {
                "target": "${src_path}/include/I${class_name}.h",
                "template": "${script_dir}/cpp-templates/ClassTypeInterface.h"
            },
            {
                "target": "${src_path}/include/${class_name}.h",
                "template": "${script_dir}/cpp-templates/ClassTypeHeader.h"
            },
            {
                "target": "${src_path}/src/${class_name}.cpp",
                "template": "${script_dir}/cpp-templates/ClassTypeSource.cpp"
            },
            {
                "target": "${src_path}/test/${class_name}Tests.cpp",
                "template": "${script_dir}/cpp-templates/ClassTypeTests.cpp"
            }
        ]
    },
    "trivial": {
        "description": "Generates a simple class header and source file",
        "files": [
             {
                "target": "${src_path}/include/${class_name}.h",
                "template": "${script_dir}/cpp-templates/TrivialClassHeader.h"
            },
            {
                "target": "${src_path}/src/${class_name}.cpp",
                "template": "${script_dir}/cpp-templates/ClassTypeSource.cpp"
            },
        ]
    }

}


def get_display_name():
    if os.name == 'nt':
        import ctypes
        GetUserNameEx = ctypes.windll.secur32.GetUserNameExW
        NameDisplay = 3
        size = ctypes.pointer(ctypes.c_ulong(0))
        GetUserNameEx(NameDisplay, None, size)
        nameBuffer = ctypes.create_unicode_buffer(size.contents.value)
        GetUserNameEx(NameDisplay, nameBuffer, size)
        return nameBuffer.value
    else:  # posix
        import pwd
        pwd.getpwuid(os.getuid())[4].split(',')[0]


def write_file(file_path, file_content, overwrite=False):
    if os.path.exists(file_path):
        if not overwrite and os.path.exists(file_path):
            print('File already exists! Call with --overwrite  if you really intended to overwrite existing files')
            return
        os.remove(file_path)
    with open(file_path, 'w') as fp:
        fp.write(file_content)


def interpolate(template_string, values):
    template = Template(template_string)
    return template.substitute(values)


def main():
    parser = argparse.ArgumentParser(description='Generates C++ header/source files with boilerplate code')
    parser.add_argument('--template', '-t', required=False, default='class',
                        help='Template of the type to be generated. So far we support only "class"')
    parser.add_argument('--path', '-p', required=False, default='.', help='Where to generate the code')
    parser.add_argument('--name', '-n', required=True, help='Name of the C++ type to generate')
    parser.add_argument('--namespace', '-s', required=True, help='Namespace to use')
    parser.add_argument('--overwrite', '-o', action="store_true",
                        help='If a file already exists, use this option to overwrite its content with boilerplate code')
    args = parser.parse_args()

    cwd = os.getcwd()

    template_name = args.template
    if template_name not in TEMPLATES:
        raise Exception('Unknown template: ' + template_name)
    template = TEMPLATES[template_name]

    files = template.get('files')

    src_path = os.path.join(cwd, args.path)
    overwrite = args.overwrite

    params = {
        'src_path': src_path,
        'script_dir': os.path.dirname(os.path.realpath(__file__)),
        'class_name': args.name,
        'namespace': args.namespace,
        'username': get_display_name()
    }

    print('Resolved interpolation parameters')
    print(json.dumps(params, indent=4))

    for gen_file in files:
        template_file = interpolate(gen_file['template'], params)
        target_file = interpolate(gen_file['target'], params)
        print('Generating ' + target_file)
        with open(template_file) as fp:
            template_content = fp.read()
        # Render the template
        code = interpolate(template_content, params)
        # Write the code
        write_file(target_file, code, overwrite)

    print('Done!')


if __name__ == '__main__':
    main()
