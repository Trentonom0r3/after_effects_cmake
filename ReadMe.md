
# After Effects Python Plugin Builder

This project provides a means to configure `.aex` effect plugins and build them using Python. By embedding the Python interpreter, users can write custom render scripts to use in an effect plugin, similar to a native effect. This enables near real-time processing and previews of the Python code you're working with.

Plugins appear under `Effect -> Python Plugins` in Adobe After Effects.

## DO NOTE
This is still to be considered a "work-in-progress". There are a few issues with the embedded interpreter setup, which will be fixed in the first official release. 

## Dependencies

- [CMake](https://cmake.org/)
- [Visual Studio 2022](https://visualstudio.microsoft.com/vs/)
- [pybind11 (x64 static)](https://pybind11.readthedocs.io/en/stable/) (Recommended install via VCPKG)
- [Python 3.11.9](https://www.python.org/downloads/release/python-3119/)

## Goals

- Provide a mechanism to configure `.aex` effect plugins and build them using Python.
- Enable users to write custom render scripts for effect plugins, allowing near real-time processing and previews.

## TODO

- Improve build process, add more options for effect plugins (parameter supervision, global setup, setdown, etc)
- Improve Documentation and examples.


## Usage

### Adjust CMakeLists.txt

Adjust the paths to include directories. Remove all hard-coded paths for your own. Change the following lines:

```cmake
# Set the path to Python include and library directories
set(PYTHON_INCLUDE_DIR "C:/Users/your_username/AppData/Local/Programs/Python/Python311/include")
set(PYTHON_LIBRARY "C:/Users/your_username/AppData/Local/Programs/Python/Python311/libs")
set(VCPKG_INCLUDE_DIR "C:/Users/your_username/vcpkg/installed/x64-windows-static/include")
```

### Write Configuration Script

Users write a configuration script to set and define parameters and the render function.

#### Example Configuration Script

Create a Python script to configure and build your plugin. Here is an example:

```python
import os
import sys

# Add the parent directory to the Python path
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'python')))

from After_Effects.plugin import Plugin, Slider, Checkbox, Color, Point, Point3D, Popup
from After_Effects.build import build_plugin

if __name__ == "__main__":
    output_folder = os.path.abspath(os.path.join(os.path.dirname(__file__), "build"))
    src_folder = os.path.abspath(os.path.join(os.path.dirname(__file__), "ExamplePlugin"))

    plugin = Plugin("ExamplePlugin")

    # Set the source folder and automatically configure requirements and render function
    plugin.set_src_folder(src_folder)

    # Add parameters to the plugin
    slider_param = Slider("SliderParam", 0.0, 0.0, 100.0, 1.0)
    plugin.add_parameter(slider_param)
    
    build_plugin(output_folder, plugin)
```

## More Parameters Options:
```py
class Slider(Parameter):
    def __init__(self, name, default, min_val, max_val, param_id):
        super().__init__(name, "slider", default, param_id, min=min_val, max=max_val)

class Checkbox(Parameter):
    def __init__(self, name, default, param_id):
        super().__init__(name, "checkbox", default, param_id)

class Color(Parameter):
    def __init__(self, name, default, param_id):
        super().__init__(name, "color", default, param_id)

class Point(Parameter):
    def __init__(self, name, default, param_id):
        super().__init__(name, "point", default, param_id)

class Point3D(Parameter):
    def __init__(self, name, default, param_id):
        super().__init__(name, "point3d", default, param_id)

class Popup(Parameter):
    def __init__(self, name, choices, default, param_id):
        super().__init__(name, "popup", default, param_id, choices=choices)
```

### Render Function

The render function should have the following format:

```python
RenderFunc = Callable[[np.ndarray, Dict[str, Union[int, float, bool, str, np.ndarray]]], np.ndarray]
```

- The first argument is always an RGBA numpy array.
- The `params` dictionary contains all parameters by the names you set in your plugin configuration.

### Directory Structure

The `src` folder contains all of your necessary processing scripts.
Initial build structure should follow that of the provided example:

```
EXAMPLE/build.py
EXAMPLE/your_plugin_name
EXAMPLE/your_plugin_name/render.py
EXAMPLE/your_plugin_name/requirements.txt
and whatever other files you need.
```

Upon building, you'll have an output like:

```
.build\your_plugin_name\your_plugin_name.aex
.build\your_plugin_name\src\your_plugin_name.py
.build\your_plugin_name\src\requirements.txt
```

## Building the Plugin

Follow these steps to build your plugin:

1. **Install Dependencies**: Ensure you have CMake, Visual Studio 2022, pybind11, and Python 3.11.9 installed on your system.
2. **Write Configuration Script**: Create your configuration script as shown in the example above. This script will define your plugin's parameters and the render function.
3. **Run the Configuration Script**: Execute the script to build your plugin. The script will configure the source folder, set up the necessary parameters, and build the plugin into the specified output folder.

## Finally, Install to After Effects.
Move the entire `YOUR_PLUGIN_NAME` folder from `build/YOUR_PLUGIN_NAME` into 
`DRIVE:\Program Files\Adobe\Adobe After Effects 202X\Support Files\Plug-ins\Effects`

## Contributing

Contributions are welcome! Please feel free to submit a Pull Request or open an Issue on GitHub if you have any suggestions or improvements.
