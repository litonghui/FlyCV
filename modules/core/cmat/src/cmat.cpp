// Copyright (c) 2022 FlyCV Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "modules/core/cmat/interface/cmat.h"

#include <map>

#include "modules/core/mat/interface/mat.h"
#include "modules/core/cmat/include/cmat_common.h"
#include "modules/core/base/include/type_info.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

typedef std::pair<CFCVImageType, FCVImageType> CTypePair;

static std::map<CFCVImageType, FCVImageType> c_img_type_map = {
    {CFCVImageType::GRAY_U8, FCVImageType::GRAY_U8},
    {CFCVImageType::GRAY_U16, FCVImageType::GRAY_U16},
    {CFCVImageType::GRAY_S32, FCVImageType::GRAY_S32},
    {CFCVImageType::GRAY_F32, FCVImageType::GRAY_F32},
    {CFCVImageType::GRAY_F64, FCVImageType::GRAY_F64},
    {CFCVImageType::PLA_BGR_U8, FCVImageType::PLA_BGR_U8},
    {CFCVImageType::PLA_RGB_U8, FCVImageType::PLA_RGB_U8},
    {CFCVImageType::PKG_BGR_U8, FCVImageType::PKG_BGR_U8},
    {CFCVImageType::PKG_RGB_U8, FCVImageType::PKG_RGB_U8},
    {CFCVImageType::PLA_BGRA_U8, FCVImageType::PLA_BGRA_U8},
    {CFCVImageType::PLA_RGBA_U8, FCVImageType::PLA_RGBA_U8},
    {CFCVImageType::PKG_BGRA_U8, FCVImageType::PKG_BGRA_U8},
    {CFCVImageType::PKG_RGBA_U8, FCVImageType::PKG_RGBA_U8},
    {CFCVImageType::PLA_BGR_F32, FCVImageType::PLA_BGR_F32},
    {CFCVImageType::PLA_RGB_F32, FCVImageType::PLA_RGB_F32},
    {CFCVImageType::PKG_BGR_F32, FCVImageType::PKG_BGR_F32},
    {CFCVImageType::PKG_RGB_F32, FCVImageType::PKG_RGB_F32},
    {CFCVImageType::PLA_BGRA_F32, FCVImageType::PLA_BGRA_F32},
    {CFCVImageType::PLA_RGBA_F32, FCVImageType::PLA_RGBA_F32},
    {CFCVImageType::PKG_BGRA_F32, FCVImageType::PKG_BGRA_F32},
    {CFCVImageType::PKG_RGBA_F32, FCVImageType::PKG_RGBA_F32},
    {CFCVImageType::PKG_BGR_F64, FCVImageType::PKG_BGR_F64},
    {CFCVImageType::PKG_RGB_F64, FCVImageType::PKG_RGB_F64},
    {CFCVImageType::PKG_BGRA_F64, FCVImageType::PKG_BGRA_F64},
    {CFCVImageType::PKG_RGBA_F64, FCVImageType::PKG_RGBA_F64},
    {CFCVImageType::PKG_BGR565_U8, FCVImageType::PKG_BGR565_U8},
    {CFCVImageType::PKG_RGB565_U8, FCVImageType::PKG_RGB565_U8},
    {CFCVImageType::NV12, FCVImageType::NV12},
    {CFCVImageType::NV21, FCVImageType::NV21},
    {CFCVImageType::I420, FCVImageType::I420},
};

int cmat_to_mat(CMat* src, Mat& dst) {
    if (src == nullptr) {
        LOG_ERR("The src is nullptr!");
        return -1;
    }

    auto iter = c_img_type_map.find(src->type);

    if (iter == c_img_type_map.end()) {
        LOG_ERR("The src type is not supported!");
        return -1;
    }

    dst = Mat(src->width, src->height, iter->second, src->data);

    return 0;
}

CMat* mat_to_cmat(Mat& src) {
    if (src.empty()) {
        LOG_ERR("The src is empty!");
        return nullptr;
    }

    CMat* dst = create_cmat(src.width(), src.height(), CFCVImageType((int)src.type()));
    memcpy(dst->data, src.data(), src.total_byte_size());

    return dst;
}

bool check_cmat(CMat* src) {
    if (src->data == nullptr
            || src->height <= 0
            || src->width <= 0
            || src->total_byte_size <= 0
            || src->stride <= 0) {
        return false;
    }

    return true;
}

CMat* create_cmat(int width, int height, CFCVImageType type) {
    auto iter = c_img_type_map.find(type);

    if (iter == c_img_type_map.end()) {
        LOG_ERR("There is no matching image type!");
        return nullptr;
    }

    TypeInfo type_info;

    if (get_type_info(iter->second, type_info)) {
        LOG_ERR("The type is not supported!");
        return nullptr;
    }

    CMat* mat = (CMat*)malloc(sizeof(CMat));
    int channel_offset = 0;
    int stride = 0;

    parse_type_info(type_info, width, height,
            channel_offset, stride, mat->total_byte_size);

    mat->data = malloc(sizeof(unsigned char) * mat->total_byte_size);
    mat->channels = type_info.channels;
    mat->width = width;
    mat->height = height;
    mat->stride = stride;
    mat->type = type;
    mat->type_byte_size = type_info.type_byte_size;

    return mat;
}

int release_cmat(CMat* mat) {
    if (mat == nullptr) return -1;

    if (mat->data != nullptr) {
        free(mat->data);
        mat->data = nullptr;
    }

    free(mat);
    mat = nullptr;
    return 0;
}

void csize_to_size(CSize& csize, Size& size) {
    size.set_width(csize.width);
    size.set_height(csize.height);
}

InterpolationType cinterpolation_to_interpolation(CInterpolationType type) {
    static std::map<CInterpolationType, InterpolationType> type_map = {
        {CInterpolationType::INTER_NEAREST, InterpolationType::INTER_NEAREST},
        {CInterpolationType::INTER_LINEAR, InterpolationType::INTER_LINEAR},
        {CInterpolationType::INTER_CUBIC, InterpolationType::INTER_CUBIC},
        {CInterpolationType::INTER_AREA, InterpolationType::INTER_AREA},
        {CInterpolationType::WARP_INVERSE_MAP, InterpolationType::WARP_INVERSE_MAP},
    };

    auto iter = type_map.find(type);

    if (iter == type_map.end()) {
        LOG_ERR("There is no matching interpolation type!");
        return InterpolationType::INTER_NEAREST;
    }

    return iter->second;
}

NormType cnormtype_to_normtype(CNormType type) {
    static std::map<CNormType, NormType> type_map = {
        {CNormType::NORM_INF, NormType::NORM_INF},
        {CNormType::NORM_L1, NormType::NORM_L1},
        {CNormType::NORM_L2, NormType::NORM_L2},
    };

    auto iter = type_map.find(type);

    if (iter == type_map.end()) {
        LOG_ERR("There is no matching interpolation type!");
        return NormType::NORM_INF;
    }

    return iter->second;
}

FCVImageType cimagetype_to_imagetype(CFCVImageType ctype) {
    auto iter = c_img_type_map.find(ctype);

    if (iter == c_img_type_map.end()) {
        LOG_ERR("There is no matching image type!");
        return FCVImageType::GRAY_U8;
    }

    return iter->second;
};

G_FCV_NAMESPACE1_END()
