from typing import Callable, Dict, Any, Union, List, Optional, get_origin, get_args
import numpy as np

from .parameters import Parameter, Slider, Checkbox, Color, Point, Point3D, Popup

# Define the type for the render function
# The render function takes an input array and a dictionary of parameters, and returns an output array
# Paramaters are provided in order of appearance/creation in the plugin
# input np.ndarray is in format (height, width, channels), where channels = 4 = (R, G, B, A)
RenderFunc = Callable[[np.ndarray, Dict[str, Union[int, float, bool, str, np.ndarray]]], np.ndarray]

class Plugin:
    
    def __init__(self, name: str, src_folder: Optional[str] = None):
        self.name = name
        self.parameters: List[Any] = []
        self.src_folder = src_folder

    def add_parameter(self, param: Any):
        self.parameters.append(param)
        
    def set_src_folder(self, src_folder: str):
        self.src_folder = src_folder

if __name__ == "__main__":
    # Create a plugin
    plugin = Plugin("MyPlugin")
    
    # Add parameters to the plugin
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
    
    