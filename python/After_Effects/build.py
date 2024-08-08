import os
import re
import shutil
import subprocess
import numpy as np
from typing import Dict, Union
from .plugin import Plugin, Slider, Checkbox, Color, Point, Point3D, Popup
from .utils import generate_cpp_params_setup, copy_and_replace_folder

def replace_text_in_file(file_path, search_text, replace_text):
    try:
        with open(file_path, 'r', encoding='utf-8') as file:
            content = file.read()
    except Exception as e:
        print(f"Error reading file {file_path}: {e}")
        return

    new_content = content.replace(search_text, replace_text)

    try:
        with open(file_path, 'w', encoding='utf-8') as file:
            file.write(new_content)
    except UnicodeEncodeError:
        try:
            with open(file_path, 'w', encoding='latin-1') as file:
                file.write(new_content)
        except Exception as e:
            print(f"Error writing file {file_path}: {e}")

def update_cmake_lists(file_path, plugin_name):
    search_text = "Skeleton"
    replace_text = plugin_name
    replace_text_in_file(file_path, search_text, replace_text)

def rename_skeleton_files_and_folder(dest_folder, plugin_name):
    template_folder = os.path.join(dest_folder, "Template")
    skeleton_folder = os.path.join(template_folder, "Skeleton")
    
    if not os.path.exists(skeleton_folder):
        print(f"Skeleton folder not found at {skeleton_folder}")
        return
    
    new_skeleton_folder = os.path.join(template_folder, plugin_name)
    shutil.move(skeleton_folder, new_skeleton_folder)
    
    for root, dirs, files in os.walk(new_skeleton_folder):
        for filename in files:
            if "Skeleton" in filename or "skeleton" in filename:
                new_filename = re.sub(r'Skeleton|skeleton', plugin_name, filename)
                old_file = os.path.join(root, filename)
                new_file = os.path.join(root, new_filename)
                print(f"Renaming file {old_file} to {new_file}")
                shutil.move(old_file, new_file)
                replace_text_in_file(new_file, "Skeleton", plugin_name)
                replace_text_in_file(new_file, "skeleton", plugin_name)
                replace_text_in_file(new_file, "SKELETON", plugin_name)

        for dirname in dirs:
            if "Skeleton" in dirname or "skeleton" in dirname:
                new_dirname = re.sub(r'Skeleton|skeleton', plugin_name, dirname)
                old_dir = os.path.join(root, dirname)
                new_dir = os.path.join(root, new_dirname)
                print(f"Renaming folder {old_dir} to {new_dir}")
                shutil.move(old_dir, new_dir)
                
                for root_sub, dirs_sub, files_sub in os.walk(new_dir):
                    for filename in files_sub:
                        file_path = os.path.join(root_sub, filename)
                        print(f"Processing file {file_path}")
                        replace_text_in_file(file_path, "Skeleton", plugin_name)
                        replace_text_in_file(file_path, "skeleton", plugin_name)
                        replace_text_in_file(file_path, "SKELETON", plugin_name)

def write_render_function_file(dest_folder, plugin_name: str, render_func_src):
    plugin_file = os.path.join(dest_folder, plugin_name.lower() + ".py")
    os.makedirs(os.path.dirname(plugin_file), exist_ok=True)
    with open(plugin_file, "w") as f:
        f.write(render_func_src)
    print(f"Render function written to {plugin_file}")

def build_plugin(output_folder, plugin: Plugin = None):
    plugin = plugin or Plugin("MyPlugin")
    plugin_name = plugin.name

    src_folder = os.path.abspath("./AE/")
    dest_folder = os.path.abspath(output_folder)

    # Clean up the existing build directory if needed
    if os.path.exists(dest_folder):
        shutil.rmtree(dest_folder)

    # Copy and replace the template folder
    copy_and_replace_folder(src_folder, dest_folder, generate_cpp_params_setup(plugin.parameters))
    rename_skeleton_files_and_folder(dest_folder, plugin_name)

    cmake_file = os.path.join(dest_folder, "Template", plugin_name, "CMakeLists.txt")
    if os.path.exists(cmake_file):
        update_cmake_lists(cmake_file, plugin_name)

    build_dir = os.path.join(dest_folder, "Template", plugin_name, "build")
    print(f"Configuring and building plugin in {build_dir}")
    os.makedirs(build_dir, exist_ok=True)

    # Ensure CMakeCache.txt and CMakeFiles directory are removed
    cmake_cache_file = os.path.join(build_dir, "CMakeCache.txt")
    cmake_files_dir = os.path.join(build_dir, "CMakeFiles")
    if os.path.exists(cmake_cache_file):
        os.remove(cmake_cache_file)
    if os.path.exists(cmake_files_dir):
        shutil.rmtree(cmake_files_dir)

    src_dir = os.path.join(dest_folder, "Template", plugin_name)

    # Configure the project
    subprocess.run(['cmake', '..', '-G', 'Visual Studio 17 2022', '-DUSE_MT=ON'], cwd=build_dir, check=True)

    # Build the project
    subprocess.run(['cmake', '--build', '.', '--config', 'Release'], cwd=build_dir, check=True)

    # Define the plugin output directory
    plugin_output_dir = os.path.join(output_folder, plugin_name)
    os.makedirs(plugin_output_dir, exist_ok=True)

    # Locate the built binary
    binary_name = f"{plugin_name}.aex"
    build_binary_path = os.path.join(build_dir, "Release", binary_name)
    
    if os.path.exists(build_binary_path):
        shutil.copy(build_binary_path, plugin_output_dir)
        print(f"Copied binary to {plugin_output_dir}")
    else:
        print(f"Binary not found at {build_binary_path}")

    src_folder_path = os.path.abspath(plugin.src_folder)
    
    if os.path.exists(src_folder_path):
        shutil.copytree(src_folder_path, os.path.join(plugin_output_dir, "src"))
        print(f"Copied source folder to {plugin_output_dir}/src") 
        
    # Rename the render.py file to plugin_name.py (lowercase) if it exists
    render_file = os.path.join(plugin_output_dir, "src", "render.py")
    if os.path.exists(render_file):
        new_render_file = os.path.join(plugin_output_dir, "src", plugin_name.lower() + ".py")
        shutil.move(render_file, new_render_file)
        print(f"Renamed render.py to {plugin_name.lower()}.py")
        
if __name__ == "__main__":
    output_folder = os.path.abspath("./build")
    plugin = Plugin("MyPlugin")

    render_function = ""

    plugin.set_src_folder(render_function)
    
    slider_param = Slider("SliderParam", 0.5, 0.0, 1.0, 1)
    checkbox_param = Checkbox("CheckboxParam", True, 2)
    color_param = Color("ColorParam", (1.0, 0.0, 0.0), 3)
    point_param = Point("PointParam", (0.5, 0.5), 4)
    point3d_param = Point3D("Point3dParam", (1.0, 1.0, 1.0), 5)
    popup_param = Popup("PopupParam", ["Option1", "Option2", "Option3"], 0, 6)
    
    plugin.add_parameter(slider_param)
    plugin.add_parameter(checkbox_param)
    plugin.add_parameter(color_param)
    plugin.add_parameter(point_param)
    plugin.add_parameter(point3d_param)
    plugin.add_parameter(popup_param)
    
    build_plugin(output_folder, plugin)
