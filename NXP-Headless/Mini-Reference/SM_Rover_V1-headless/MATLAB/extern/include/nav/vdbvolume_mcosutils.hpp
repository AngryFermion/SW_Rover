// Copyright 2023-2024 The MathWorks, Inc.
#include "matrix.h"
#include "mcos_factory/mi.hpp"
#include "sl_services_mi/slsv_mcos.hpp"

#ifndef VDBVOLUME_MCOSUTILS
#define VDBVOLUME_MCOSUTILS

namespace nav {
using mcos::factory::mxArrayProxy;

template <typename Data_T, typename M_T = size_t>
void mxArrayProxyToRaw(const mxArrayProxy& mx2D, Data_T*& data, M_T* numRow = nullptr) {
    data = static_cast<Data_T*>(mxGetData(mx2D.fMember));
    if (numRow != nullptr) {
        *numRow = static_cast<M_T>(mxGetM(mx2D.fMember));
    }
}
} // namespace nav

#endif /* VDBVOLUME_MCOSUTILS */
