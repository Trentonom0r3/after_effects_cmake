import shutil
import os

def generate_cpp_params_setup(parameters):
    cpp_code = """
static PF_Err
ParamsSetup (    
    PF_InData        *in_data,
    PF_OutData        *out_data,
    PF_ParamDef        *params[],
    PF_LayerDef        *output )
{
    PF_Err        err        = PF_Err_NONE;
    PF_ParamDef    def;    

"""
    
    for param in parameters:
        name = param.name
        param_id = param.id
        if param.type == "slider":
            min_val = param.kwargs['min']
            max_val = param.kwargs['max']
            default_val = param.default
            cpp_code += f"""
    AEFX_CLR_STRUCT(def);
    PF_ADD_FLOAT_SLIDERX("{name}", {min_val}, {max_val}, {min_val}, {max_val}, {default_val}, PF_Precision_HUNDREDTHS, 0, PF_ParamFlag_SUPERVISE, {param_id});
    SKELETON_NUM_PARAMS++;
    """
        elif param.type == "checkbox":
            default_val = param.default
            cpp_code += f"""
    AEFX_CLR_STRUCT(def);
    PF_ADD_CHECKBOXX("{name}", true, 0, {param_id});
    SKELETON_NUM_PARAMS++;
    """
        elif param.type == "color":
            red, green, blue = param.default
            cpp_code += f"""
    AEFX_CLR_STRUCT(def);
    PF_ADD_COLOR("{name}", {int(red * 255)}, {int(green * 255)}, {int(blue * 255)}, {param_id});
    SKELETON_NUM_PARAMS++;
    """
        elif param.type == "point":
            x, y = param.default
            cpp_code += f"""
    AEFX_CLR_STRUCT(def);
    PF_ADD_POINT("{name}", {int(x * 100)}, {int(y * 100)}, false, {param_id});
    SKELETON_NUM_PARAMS++;
    """
        elif param.type == "point3d":
            x, y, z = param.default
            cpp_code += f"""
    AEFX_CLR_STRUCT(def);
    PF_ADD_POINT_3D("{name}", {x}, {y}, {z}, {param_id});
    SKELETON_NUM_PARAMS++;
    """
        elif param.type == "popup":
            choices = param.kwargs['choices']
            choices_str = '|'.join(choices)
            default_val = param.default
            num_choices = len(choices)
            cpp_code += f"""
    AEFX_CLR_STRUCT(def);
    PF_ADD_POPUPX("{name}", {num_choices}, {default_val}, "{choices_str}", 0, {param_id});
    SKELETON_NUM_PARAMS++;
    """
    cpp_code += """
    out_data->num_params = SKELETON_NUM_PARAMS;
    return err;
}
"""
    return cpp_code

import re

def replace_params_setup(cpp_file, cpp_code):
    """
    Replace the ParamsSetup function in the specified C++ file with new code.

    Args:
        cpp_file (str): Path to the C++ file.
        cpp_code (str): New code to replace the ParamsSetup function.
    """

    try:
        # Read the content of the C++ file
        with open(cpp_file, "r") as f:
            file_contents = f.read()

        # Check if the cpp_code ends with a closing brace
        if not cpp_code.strip().endswith('}'):
            print("Error: cpp_code does not end with a closing brace.")
            return

        # Regex pattern to match the ParamsSetup function
        pattern = re.compile(r"static PF_Err\s+ParamsSetup\s*\([^)]*\)\s*{[^}]*}", re.DOTALL)

        # Replace the ParamsSetup function with new code
        new_contents = pattern.sub(cpp_code, file_contents)

        # Write the new content to the file
        with open(cpp_file, "w") as f:
            f.write(new_contents)

    
    except FileNotFoundError:
        print(f"Error: The file {cpp_file} does not exist.")
    except IOError as e:
        print(f"Error: Unable to read/write the file {cpp_file}. {e}")

def copy_and_replace_folder(src_folder, dest_folder, cpp_code):
    if os.path.exists(dest_folder):
        shutil.rmtree(dest_folder)
    shutil.copytree(src_folder, dest_folder)
    print(f"Copied {src_folder} to {dest_folder}")
    cpp_file = os.path.join(os.path.abspath(dest_folder), "Template/Skeleton/Skeleton.cpp")
    replace_params_setup(cpp_file, cpp_code)
