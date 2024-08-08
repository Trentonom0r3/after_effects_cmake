# Define the render function
from typing import Dict, Union
import numpy as np


def render(input_array: np.ndarray, params: Dict[str, Union[int, float, bool, str, np.ndarray]]) -> np.ndarray:
    try :
        # Access your parameters
        # blur_factor = params["SliderParam"]
        # Do some processing
        # output_array = cv2.GaussianBlur(input_array, (5, 5), blur_factor)
        return input_array
    except Exception as e:
        print(e)
        return input_array
    