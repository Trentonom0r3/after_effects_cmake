#include "Skeleton.h"
#include <iostream>
#include <filesystem>

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
	out_data->out_flags = PF_OutFlag_DEEP_COLOR_AWARE;  // Just 16bpc, not 32bpc
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

    out_data->num_params = SKELETON_NUM_PARAMS;
    return err;
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
        return PF_Err_INTERNAL_STRUCT_DAMAGED;
    }

    // Redirect std::cerr to the log file
    std::streambuf* original_cerr_buffer = std::cerr.rdbuf();
    std::cerr.rdbuf(log_file.rdbuf());

    std::cerr << "Render function called." << std::endl;
    std::cerr << "Full path: " << full_path << std::endl;

    try {
        if (!Py_IsInitialized()) {
            py::initialize_interpreter();
        }

        py::dict paramsDict = convertParamsToDict(params, in_data);

        py::module_ sys = py::module_::import("sys");
        py::object path = sys.attr("path");
        // Append necessary directories to the Python path
        path.attr("append")(src_dir.string()); // Add directory to path
        std::cerr << "Python path appended successfully." << std::endl;

        py::module_ script = py::module_::import(ScriptName.c_str());
        std::cerr << "Script loaded successfully." << std::endl;

        py::array output_array = AEToNumpyConverter::ConvertLayerToNumpy(&params[0]->u.ld, in_data);
        std::cerr << "Output array created successfully." << std::endl;

        py::object result = script.attr("render")(output_array, paramsDict);
        std::cerr << "Render function called successfully." << std::endl;

        AEToNumpyConverter::ConvertNumpyToLayerDef(result.cast<py::array>(), output);
        std::cerr << "New image created successfully." << std::endl;

        err = PF_Err_NONE;
    }
    catch (const py::error_already_set& e) {
        std::cerr << "Python Error (py::error_already_set): " << std::endl;
        std::cerr << e.what() << std::endl;
        std::cerr << e.trace() << std::endl; // Capture the Python traceback
        err = PF_Err_INTERNAL_STRUCT_DAMAGED;
    }
    catch (const std::invalid_argument& e) {
        std::cerr << "Error (std::invalid_argument): " << e.what() << std::endl;
        err = PF_Err_INTERNAL_STRUCT_DAMAGED;
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Error (std::runtime_error): " << e.what() << std::endl;
        err = PF_Err_INTERNAL_STRUCT_DAMAGED;
    }
    catch (const std::exception& e) {
        std::cerr << "Error (std::exception): " << e.what() << std::endl;
        err = PF_Err_INTERNAL_STRUCT_DAMAGED;
    }
    catch (...) {
        std::cerr << "Unknown error occurred." << std::endl;
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

