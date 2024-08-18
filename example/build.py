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
    slider_param = Slider("SliderParam", 0.0, 0.0, 10.0, 1.0, i_resize_buffer=True)
    plugin.add_parameter(slider_param)
    
    # Now, build the plugin with the updated build function
    build_plugin(output_folder, plugin)
