import shutil
import os
import re
#create decorator for parameter supervision. Requires paramID (+1) to be passed as an argument, as well as the name for double checing.
# apply the decorator to the python code, which will be used in further cpp generation (not written yet)
#basically used to provide a param_id (which ID is being used in the c++ code) as well as a callback function be used when that parameter is changed.
def supervise_param(param_id, name):
    def decorator(func):
        def wrapper(*args, **kwargs):
            return func(*args, **kwargs)
        return wrapper
    return decorator

#usage
#@supervise_param(1, "SliderParam")
#def slider_param_changed(value):
    #print(f"SliderParam changed to {value}")

#cpp code:

#static PF_Err
#UserChangedParam(
#	PF_InData* in_data,
#	PF_OutData* out_data,
#	PF_ParamDef* params[],
#	PF_LayerDef* outputP,
#	const PF_UserChangedParamExtra* which_hitP)
#{
#	PF_Err				err = PF_Err_NONE;
#
#	if (which_hitP->param_index == 8) //if is the button.
#	{
#		PF_ParamDef def;
	#	//1->50
#		std::vector<cv::Mat> frames;
#		for (int i = 1; i <= 50; i++) {
#			AEFX_CLR_STRUCT(def);
#			int time = i * in_data->time_step;
#			PF_CHECKOUT_PARAM(in_data, 0, time, in_data->time_step, in_data->time_scale, &def);
#			auto frame = AEToCVConverter::ConvertLayerToMat(&def.u.ld, in_data);
#			frames.push_back(frame);
#			PF_CHECKIN_PARAM(in_data, &def);
#		}
#		for (int i = 0; i < frames.size() - 1; i++) {
#			cv::imshow("Frame", frames[i]);
		#	cv::waitKey(0);
#				
#		}
#	}
#
#	return err;
#}

def generate_cpp_user_changed_param(parameters):
    # to do this, will have to create the callback 
    pass

"""
    PF_ParamDef scale_param;
    PF_CHECKOUT_PARAM(in_data, INDEX, in_data->current_time, in_data->time_step, in_data->time_scale, &scale_param);
  
static
PF_Err
FrameSetup(
    PF_InData* in_data,
    PF_OutData* out_data,
    PF_ParamDef* params[],
    PF_LayerDef* output)
{
    PF_ParamDef scale_param;
    PF_CHECKOUT_PARAM(in_data, INDEX, in_data->current_time, in_data->time_step, in_data->time_scale, &scale_param);
    // Assume you have a parameter that affects the size, such as a blur or scale factor
    PF_FpLong blur_factor = params[1]->u.fs_d.value;
    PF_FpLong scale_factor = 1.0 + (blur_factor / 10.0);  // Adjust scaling based on blur

    // Calculate the estimated size of the output buffer
    A_long new_width = static_cast<A_long>(params[0]->u.ld.width * scale_factor);
    A_long new_height = static_cast<A_long>(params[0]->u.ld.height * scale_factor);

    // Set the output size
    out_data->width = new_width;
    out_data->height = new_height;

    // Adjust the origin if necessary
    out_data->origin.h = static_cast<A_short>((out_data->width - params[0]->u.ld.width) / 2);
    out_data->origin.v = static_cast<A_short>((out_data->height - params[0]->u.ld.height) / 2);

    return PF_Err_NONE;
}
"""
def generate_cpp_frame_setup(parameters):
    cpp_code = """
static PF_Err
FrameSetup(
    PF_InData* in_data,
    PF_OutData* out_data,
    PF_ParamDef* params[],
    PF_LayerDef* output)
{
    PF_Err err = PF_Err_NONE;
"""
    
    for param in parameters:
        if param.type == "slider" and param.kwargs.get("i_resize_buffer", False):
            cpp_code += f"""
    PF_ParamDef scale_param;
    PF_CHECKOUT_PARAM(in_data, {param.id}, in_data->current_time, in_data->time_step, in_data->time_scale, &scale_param);
    PF_FpLong scale_value = scale_param.u.fs_d.value;

    // Calculate the estimated size of the output buffer
    A_long new_width = static_cast<A_long>(params[0]->u.ld.width * scale_value);
    A_long new_height = static_cast<A_long>(params[0]->u.ld.height * scale_value);
    
    // Set the output size
    out_data->width = new_width;
    out_data->height = new_height;
    
    // Adjust the origin if necessary
    out_data->origin.h = static_cast<A_short>((out_data->width - params[0]->u.ld.width) / 2);
    out_data->origin.v = static_cast<A_short>((out_data->height - params[0]->u.ld.height) / 2);
    
    PF_CHECKIN_PARAM(in_data, &scale_param);
"""
    
    cpp_code += """
    return err;
}
"""
    return cpp_code



def replace_frame_setup(cpp_file, cpp_code):
    """
    Replace the FrameSetup function in the specified C++ file with new code.

    Args:
        cpp_file (str): Path to the C++ file.
        cpp_code (str): New code to replace the FrameSetup function.
    """

    try:
        # Read the content of the C++ file
        with open(cpp_file, "r") as f:
            file_contents = f.read()
        #input("Press Enter to continue...")
        # Check if the cpp_code ends with a closing brace
        if not cpp_code.strip().endswith('}'):
            print("Error: cpp_code does not end with a closing brace.")
            #input("Press Enter to continue...")
            return

        # Regex pattern to match the FrameSetup function
        pattern = re.compile(r"static PF_Err\s+FrameSetup\s*\([^)]*\)\s*{[^}]*}", re.DOTALL)

        # Replace the FrameSetup function with new code
        new_contents = pattern.sub(cpp_code, file_contents)

        # Write the new content to the file
        with open(cpp_file, "w") as f:
            f.write(new_contents)

    except FileNotFoundError:
        print(f"Error: The file {cpp_file} does not exist.")
        #input("Press Enter to continue...")
    except IOError as e:
        print(f"Error: Unable to read/write the file {cpp_file}. {e}")
       # input("Press Enter to continue...")

    
  
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
