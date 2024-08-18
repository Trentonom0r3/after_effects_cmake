#include "Skeleton.h"
#include <iostream>
#include <filesystem>
#include "pch.h"
#include <fstream>

namespace fs = std::filesystem;
namespace py = pybind11;
int SKELETON_NUM_PARAMS = 1;

static PF_Err 
About (	
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output )
{
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	

	return PF_Err_NONE;
}

static PF_Err GlobalSetup(PF_InData* in_data, PF_OutData* out_data, PF_ParamDef* params[], PF_LayerDef* output) {
	out_data->my_version = PF_VERSION(MAJOR_VERSION, MINOR_VERSION, BUG_VERSION, STAGE_VERSION, BUILD_VERSION);
	PF_Err err = PF_Err_NONE;
    out_data->out_flags = PF_OutFlag_DEEP_COLOR_AWARE | PF_OutFlag_DISPLAY_ERROR_MESSAGE | PF_OutFlag_I_EXPAND_BUFFER;  // Just 16bpc, not 32bpc
    if (!pyfx::running()) {
        pyfx::start();
    }
	return err;
}

static PF_Err
ParamsSetup(
    PF_InData* in_data,
    PF_OutData* out_data,
    PF_ParamDef* params[],
    PF_LayerDef* output)
{
    PF_Err        err = PF_Err_NONE;
    PF_ParamDef    def;

    AEFX_CLR_STRUCT(def);


    out_data->num_params = SKELETON_NUM_PARAMS;
    return err;
}

std::string getCurrentDirectory() {
    char buffer[MAX_PATH];
    if (GetModuleFileNameA(NULL, buffer, MAX_PATH)) {
        std::string path(buffer);
        size_t pos = path.find_last_of("\\/");
        return path.substr(0, pos);
    }
    return "";
}

py::dict convertParamsToDict(PF_ParamDef* params[], PF_InData* in_data) {
    try {
        py::dict d;
        PF_ParamDef def;
        //find params size
        for (int i = 1; i < SKELETON_NUM_PARAMS; ++i) {
            std::cerr << "Parameter " << i << std::endl;
            AEFX_CLR_STRUCT(def);
            PF_CHECKOUT_PARAM(in_data, i, in_data->current_time, in_data->time_step, in_data->time_scale, &def);
            switch (def.param_type) {
                std::cerr << "Parameter type: " << def.param_type << std::endl;
            case PF_Param_ANGLE:
                d[def.name] = def.u.ad.value;
                break;
            case PF_Param_POPUP:
                d[def.name] = def.u.pd.value;
                break;
            case PF_Param_CHECKBOX:
                d[def.name] = def.u.bd.value;
                break;
            case PF_Param_COLOR:
                d[def.name] = std::vector<uint8_t>{
                    def.u.cd.value.red,
                    def.u.cd.value.green,
                    def.u.cd.value.blue
                };
                break;
            case PF_Param_POINT:
                d[def.name] = std::vector<int>{
                    def.u.td.x_value,
                    def.u.td.y_value
                };
                break;
            case PF_Param_POINT_3D:
                d[def.name] = std::vector<double>{
                    def.u.point3d_d.x_value,
                    def.u.point3d_d.y_value,
                    def.u.point3d_d.z_value
                };
                break;
            case PF_Param_FLOAT_SLIDER:
                d[def.name] = def.u.fs_d.value;
                break;
            default:
                // Handle or log unknown parameter types if needed
                std::cerr << "Unknown parameter type: " << def.param_type << std::endl;
                break;
            }
            PF_CHECKIN_PARAM(in_data, &def);
        }
        return d;
    }
    catch (const std::exception& e) {
        std::cerr << "Error in convertParamsToDict: " << e.what() << std::endl;
        throw e;
    }
}

// Utility function to convert a string to lowercase
std::string toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

// Helper function to get the path of the current module
std::string getCurrentModulePath() {
    char modulePath[MAX_PATH];
    HMODULE hModule = nullptr;

    if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCSTR>(&getCurrentModulePath), &hModule)) {
        GetModuleFileName(hModule, modulePath, sizeof(modulePath));
    }
    return std::string(modulePath);
}

static PF_Err
FrameSetup(
    PF_InData* in_data,
    PF_OutData* out_data,
    PF_ParamDef* params[],
    PF_LayerDef* output)
{

    return PF_Err_NONE;
}



static PF_Err Render(
    PF_InData* in_data,
    PF_OutData* out_data,
    PF_ParamDef* params[],
    PF_LayerDef* output)
{
    PF_Err err = PF_Err_NONE;
    AEGP_SuiteHandler suites(in_data->pica_basicP);
    std::string pluginName = "Skeleton";
    std::string ScriptName = toLowerCase(pluginName);

    // Get the actual path of the current module (plugin)
    std::string modulePath = getCurrentModulePath();
    fs::path full_path(modulePath);
    full_path = full_path.parent_path(); // Get directory of the module
    fs::path src_dir = full_path / "src";
    fs::path log_path = full_path / (ScriptName + ".log");

    // Ensure log directory exists
    fs::create_directories(log_path.parent_path());

    // Open log file in append mode
    std::ofstream log_file(log_path, std::ios_base::app);
    if (!log_file) {
        std::cerr << "Failed to open log file: " << log_path << std::endl;
    }

    // Redirect std::cerr to the log file
    std::streambuf* original_cerr_buffer = std::cerr.rdbuf();
    std::cerr.rdbuf(log_file.rdbuf());

    std::cerr << "Render function called." << std::endl;
    std::cerr << "Full path: " << full_path << std::endl;

    try {
        py::gil_scoped_acquire acquire;

        py::dict paramsDict = convertParamsToDict(params, in_data);

        py::module_ sys = py::module_::import("sys");
        py::object path = sys.attr("path");
        path.attr("append")(src_dir.string()); // Add directory to path
        std::cerr << "Python path appended successfully." << std::endl;

        py::module_ script;
        try {
            script = py::module_::import(ScriptName.c_str());
            std::cerr << "Script loaded successfully." << std::endl;
        }
        catch (const py::error_already_set& e) {
            std::cerr << "Error importing script: " << e.what() << std::endl;
            strncpy(out_data->return_msg, "Error importing Python script. Check the log file for more details.", PF_MAX_EFFECT_MSG_LEN);
            out_data->return_msg[PF_MAX_EFFECT_MSG_LEN] = '\0'; // Ensure null-termination
            err = PF_Err_INTERNAL_STRUCT_DAMAGED;
            throw; // Re-throw to the main catch block
        }

        py::object render_func;
        try {
            render_func = script.attr("render");
            if (!py::isinstance<py::function>(render_func)) {
                throw std::runtime_error("The 'render' attribute is not a callable function.");
            }
            std::cerr << "'render' function found successfully." << std::endl;
        }
        catch (const py::error_already_set& e) {
            std::cerr << "Error accessing 'render' attribute: " << e.what() << std::endl;
            strncpy(out_data->return_msg, "Error accessing 'render' function. Check the log file for more details.", PF_MAX_EFFECT_MSG_LEN);
            out_data->return_msg[PF_MAX_EFFECT_MSG_LEN] = '\0'; // Ensure null-termination
            err = PF_Err_INTERNAL_STRUCT_DAMAGED;
            throw; // Re-throw to the main catch block
        }
        catch (const std::runtime_error& e) {
            std::cerr << "Runtime error: " << e.what() << std::endl;
            strncpy(out_data->return_msg, e.what(), PF_MAX_EFFECT_MSG_LEN);
            out_data->return_msg[PF_MAX_EFFECT_MSG_LEN] = '\0'; // Ensure null-termination
            err = PF_Err_INTERNAL_STRUCT_DAMAGED;
            throw; // Re-throw to the main catch block
        }

        py::array output_array;
        try {
            output_array = render_func(AEToNumpyConverter::ConvertLayerToNumpy(&params[0]->u.ld, in_data), paramsDict).cast<py::array>();
            std::cerr << "Render function executed successfully." << std::endl;

            // Convert the result back to AE's PF_LayerDef format
            AEToNumpyConverter::ConvertNumpyToLayerDef(output_array, output, in_data);
            std::cerr << "New image created successfully." << std::endl;

            sys.attr("path").attr("remove")(src_dir.string()); // Clean up Python path
            err = PF_Err_NONE;
        }
        catch (const py::error_already_set& e) {
            std::cerr << "Python error in 'render' function: " << e.what() << std::endl;
            strncpy(out_data->return_msg, e.what(), PF_MAX_EFFECT_MSG_LEN);
            out_data->return_msg[PF_MAX_EFFECT_MSG_LEN] = '\0'; // Ensure null-termination
            err = PF_Err_INTERNAL_STRUCT_DAMAGED;
        }
        catch (const std::exception& e) {
            std::cerr << "Exception in 'render' function: " << e.what() << std::endl;
            strncpy(out_data->return_msg, e.what(), PF_MAX_EFFECT_MSG_LEN);
            out_data->return_msg[PF_MAX_EFFECT_MSG_LEN] = '\0'; // Ensure null-termination
            err = PF_Err_INTERNAL_STRUCT_DAMAGED;
        }
    }
    catch (...) {
        std::cerr << "An unknown error occurred." << std::endl;
        strncpy(out_data->return_msg, "An unknown error occurred.", PF_MAX_EFFECT_MSG_LEN);
        out_data->return_msg[PF_MAX_EFFECT_MSG_LEN] = '\0'; // Ensure null-termination
        err = PF_Err_INTERNAL_STRUCT_DAMAGED;
    }

    // Restore original std::cerr buffer
    std::cerr.rdbuf(original_cerr_buffer);
    log_file.close();

    return err;
}


extern "C" DllExport
PF_Err PluginDataEntryFunction2(
	PF_PluginDataPtr inPtr,
	PF_PluginDataCB2 inPluginDataCallBackPtr,
	SPBasicSuite* inSPBasicSuitePtr,
	const char* inHostName,
	const char* inHostVersion)
{
	PF_Err result = PF_Err_INVALID_CALLBACK;
	result = PF_REGISTER_EFFECT_EXT2(
		inPtr,
		inPluginDataCallBackPtr,
		"Skeleton", // Name
		"ADBE Skeleton", // Match Name
		"Python Plugins", // Category
		AE_RESERVED_INFO, // Reserved Info
		"EffectMain",	// Entry point
		"https://www.adobe.com");	// support URL

	return result;
}


PF_Err
EffectMain(
	PF_Cmd			cmd,
	PF_InData		*in_data,
	PF_OutData		*out_data,
	PF_ParamDef		*params[],
	PF_LayerDef		*output,
	void			*extra)
{
	PF_Err		err = PF_Err_NONE;
	try {
		switch (cmd) {
			case PF_Cmd_ABOUT:

				err = About(in_data,
							out_data,
							params,
							output);
				break;
				
			case PF_Cmd_GLOBAL_SETUP:

				err = GlobalSetup(	in_data,
									out_data,
									params,
									output);
				break;

            case PF_Cmd_FRAME_SETUP:
                err = FrameSetup(in_data, out_data, params, output);
                break;
				
			case PF_Cmd_PARAMS_SETUP:

				err = ParamsSetup(	in_data,
									out_data,
									params,
									output);
				break;
				
			case PF_Cmd_RENDER:

				err = Render(	in_data,
								out_data,
								params,
								output);
				break;

			
		}
	}
	catch(PF_Err &thrown_err){
		err = thrown_err;
	}
	return err;
}

