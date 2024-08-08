/*******************************************************************/
/*                                                                 */
/*                      ADOBE CONFIDENTIAL                         */
/*                   _ _ _ _ _ _ _ _ _ _ _ _ _                     */
/*                                                                 */
/* Copyright 2007-2023 Adobe Inc.                                  */
/* All Rights Reserved.                                            */
/*                                                                 */
/* NOTICE:  All information contained herein is, and remains the   */
/* property of Adobe Inc. and its suppliers, if                    */
/* any.  The intellectual and technical concepts contained         */
/* herein are proprietary to Adobe Inc. and its                    */
/* suppliers and may be covered by U.S. and Foreign Patents,       */
/* patents in process, and are protected by trade secret or        */
/* copyright law.  Dissemination of this information or            */
/* reproduction of this material is strictly forbidden unless      */
/* prior written permission is obtained from Adobe Inc.            */
/* Incorporated.                                                   */
/*                                                                 */
/*******************************************************************/

/*
    Skeleton.h
*/

#pragma once

#ifndef SKELETON_H
#define SKELETON_H

typedef unsigned char        u_char;
typedef unsigned short       u_short;
typedef unsigned short       u_int16;
typedef unsigned long        u_long;
typedef short int            int16;
#define PF_TABLE_BITS    12
#define PF_TABLE_SZ_16   4096

#define PF_DEEP_COLOR_AWARE 1    // make sure we get 16bpc pixels; 
// AE_Effect.h checks for this.

#include "AEConfig.h"

#ifdef AE_OS_WIN
typedef unsigned short PixelType;
#include <Windows.h>
#endif

#include "entry.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "AE_EffectCBSuites.h"
#include "String_Utils.h"
#include "AE_GeneralPlug.h"
#include "AEFX_ChannelDepthTpl.h"
#include "AEGP_SuiteHandler.h"
#include <Python.h>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>  // for the embedded interpreter
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <pybind11/functional.h>
#include <pybind11/detail/common.h>
#include <cstddef> // Add this line to include the definition for ssize_t
#include <vector>
#include <string>

/* Versioning information */

#define MAJOR_VERSION   1
#define MINOR_VERSION   1
#define BUG_VERSION     0
#define STAGE_VERSION   PF_Stage_DEVELOP
#define BUILD_VERSION   1

namespace py = pybind11;
class AEToNumpyConverter {
public:
    static py::array ConvertLayerToNumpy(PF_LayerDef* layerDef, PF_InData* in_data) {
        // Get a pointer to the pixel data
        PF_Pixel8* pixelData = nullptr;
        PF_GET_PIXEL_DATA8(layerDef, NULL, &pixelData);

        if (!pixelData) {
            throw std::runtime_error("Failed to get pixel data.");
        }

        // Calculate the width and height of the full layer
        int width = layerDef->width;
        int height = layerDef->height;

        // Create a NumPy array for the output
        std::vector<py::ssize_t> shape = { height, width, 4 };
        std::vector<py::ssize_t> strides = { static_cast<py::ssize_t>(width * 4), 4, 1 };
        py::array np_array = py::array(py::buffer_info(
            nullptr,                    // Pointer to the data (nullptr for now)
            sizeof(uint8_t),            // Size of one element
            py::format_descriptor<uint8_t>::format(), // Format descriptor
            3,                          // Number of dimensions
            shape,                      // Dimensions
            strides                     // Strides
        ));

        // Allocate memory for the NumPy array and get a pointer to it
        uint8_t* np_data = static_cast<uint8_t*>(np_array.request().ptr);

        // Copy pixel data from AE and reorder channels from ARGB to RGBA
        for (int y = 0; y < height; ++y) {
            PF_Pixel8* srcRowPtr = pixelData + y * (layerDef->rowbytes / sizeof(PF_Pixel8));
            uint8_t* dstRowPtr = np_data + y * width * 4;
            for (int x = 0; x < width; ++x) {
                dstRowPtr[4 * x + 0] = srcRowPtr[x].red;   // R
                dstRowPtr[4 * x + 1] = srcRowPtr[x].green; // G
                dstRowPtr[4 * x + 2] = srcRowPtr[x].blue;  // B
                dstRowPtr[4 * x + 3] = srcRowPtr[x].alpha; // A
            }
        }

        return np_array;
    }

    static void ConvertNumpyToLayerDef(const py::array& np_array, PF_LayerDef* layerDef) {
        int width = layerDef->width;
        int height = layerDef->height;

        // Validate input array
        if (np_array.ndim() != 3 || np_array.shape(2) != 4) {
            throw std::invalid_argument("NumPy array must have 3 dimensions and 4 channels (RGBA).");
        }

        // Get the pointer to the data
        uint8_t* np_data = static_cast<uint8_t*>(np_array.request().ptr);

        // Copy pixel data from NumPy array and reorder channels from RGBA to ARGB
        for (int y = 0; y < height; ++y) {
            uint8_t* srcRowPtr = np_data + y * width * 4;
            PF_Pixel8* dstRowPtr = reinterpret_cast<PF_Pixel8*>(reinterpret_cast<uint8_t*>(layerDef->data) + y * layerDef->rowbytes);
            for (int x = 0; x < width; ++x) {
                dstRowPtr[x].red = srcRowPtr[4 * x + 0]; // R
                dstRowPtr[x].green = srcRowPtr[4 * x + 1]; // G
                dstRowPtr[x].blue = srcRowPtr[4 * x + 2]; // B
                dstRowPtr[x].alpha = srcRowPtr[4 * x + 3]; // A
            }
        }

        //std::cerr << "ConvertNumpyToLayerDef: Data copied to layerDef successfully." << std::endl;
    }
};


extern "C" {

    DllExport
        PF_Err
        EffectMain(
            PF_Cmd          cmd,
            PF_InData* in_data,
            PF_OutData* out_data,
            PF_ParamDef* params[],
            PF_LayerDef* output,
            void* extra);

}

#endif // SKELETON_H
