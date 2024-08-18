# Define the render function
from typing import Dict, Union
import cv2
import numpy as np


def render(input_array: np.ndarray, params: Dict[str, Union[int, float, bool, str, np.ndarray]]) -> np.ndarray:
    try :
        # Access your parameters
        blur_factor = params["SliderParam"]
        # Do some processing
        output_array = cv2.resize(input_array, None, fx=blur_factor, fy=blur_factor, interpolation=cv2.INTER_LINEAR)
        return output_array
    except Exception as e:
        print(e)
        return input_array
    