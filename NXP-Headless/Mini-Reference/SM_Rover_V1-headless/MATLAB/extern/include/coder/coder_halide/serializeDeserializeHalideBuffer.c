/* Copyright 2024 The MathWorks, Inc. */
#include "serializeDeserializeHalideBuffer.h"


void initialize_halide_buffer(halide_buffer_t* hbuffer){
    //Initializing fields of halide_buffer_t;
    hbuffer->device=0;
    hbuffer->device_interface=NULL;
    hbuffer->padding=NULL;
    hbuffer->flags=0;
}

halide_buffer_t matlabArrayToHalideBuffer_size_numDim(const void* wrapee, const int* size, const int dim,
const uint8_t halide_type_t_code, const uint8_t halide_type_t_bits, const uint16_t halide_type_t_lanes) {
    halide_buffer_t hbuffer;
    initialize_halide_buffer(&hbuffer);
    hbuffer.dimensions = dim;
    hbuffer.dim = (halide_dimension_t*)malloc(sizeof(halide_dimension_t)*dim);
    int total_size = (dim > 0) ? 1 : 0;
    for (int i = 0; i < dim; i++) {
        hbuffer.dim[i].min = 0;

        if (i != 0) {
            total_size = total_size * size[i - 1];
        }

        hbuffer.dim[i].stride = (int32_t) total_size;
        hbuffer.dim[i].extent = (int32_t) size[i];
    }

    hbuffer.host = (uint8_t*) wrapee;
    struct halide_type_t hType;
    hType.code = halide_type_t_code;
    hType.bits = halide_type_t_bits;
    hType.lanes = halide_type_t_lanes;
    hbuffer.type = hType;
    hbuffer.flags |= halide_buffer_flag_host_dirty;
    hbuffer.flags &= ~(uint64_t) halide_buffer_flag_device_dirty;
    return hbuffer;
}

halide_buffer_t matlabArrayToHalideBuffer_min_size_stride_numDim(const void* wrapee, const int* min, const int* size, const int* stride, const int dim,
const uint8_t halide_type_t_code, const uint8_t halide_type_t_bits, const uint16_t halide_type_t_lanes) {
    halide_buffer_t hbuffer;
    initialize_halide_buffer(&hbuffer);
    hbuffer.dimensions = dim;
    hbuffer.dim = (halide_dimension_t*)malloc(sizeof(halide_dimension_t)*dim);
    for (int i = 0; i < dim; i++) {
        hbuffer.dim[i].min = (int32_t) min[i];
        hbuffer.dim[i].extent = (int32_t) size[i];
        hbuffer.dim[i].stride = (int32_t) stride[i];
    }

    hbuffer.host = (uint8_t*) wrapee;
    struct halide_type_t hType;
    hType.code = halide_type_t_code;
    hType.bits = halide_type_t_bits;
    hType.lanes = halide_type_t_lanes;
    hbuffer.type = hType;
    hbuffer.flags |= halide_buffer_flag_host_dirty;
    hbuffer.flags &= ~(uint64_t) halide_buffer_flag_device_dirty;
    return hbuffer;
}


void halideBufferToMatlabArray(halide_buffer_t* wrapee, void* out, const int size) {
    out = (void*)(wrapee->host);
}

void deallocateHalideBuffer(halide_buffer_t* wrapee) {
    if (wrapee->dim) {
        free(wrapee->dim); // sbcheck:ok:allocterms
    }
}
