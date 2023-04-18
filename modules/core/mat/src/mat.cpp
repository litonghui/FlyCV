// Copyright (c) 2021 FlyCV Authors. All Rights Reserved.
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

#include "modules/core/mat/interface/mat.h"
#include "modules/core/base/include/type_info.h"

G_FCV_NAMESPACE1_BEGIN(g_fcv_ns)

Mat::Mat() :
        _width(0),
        _height(0),
        _stride(0),
        _type(FCVImageType::GRAY_U8),
        _platform(PlatformType::CPU),
        _data(nullptr),
        _allocator(nullptr) {
    _initialize();
}

Mat::Mat(
        int width,
        int height,
        FCVImageType type,
        void* data,
        int stride) :
        _width(width),
        _height(height),
        _stride(stride),
        _type(type),
        _platform(PlatformType::CPU),
        _data(data),
        _allocator(nullptr) {
    _initialize();
}

Mat::Mat(
        int width,
        int height,
        FCVImageType type,
        std::vector<uint64_t*>& phy_addrs,
        std::vector<uint64_t*>& vir_addrs,
        int stride) :
        _width(width),
        _height(height),
        _stride(stride),
        _type(type),
        _platform(PlatformType::CPU),
        _data(nullptr),
        _phy_addrs(phy_addrs),
        _vir_addrs(vir_addrs),
        _allocator(nullptr) {
    _initialize();
}

Mat::Mat(
        int width,
        int height,
        FCVImageType type,
        int stride,
        PlatformType platform) :
        _width(width),
        _height(height),
        _stride(stride),
        _type(type),
        _platform(platform),
        _data(nullptr),
        _allocator(nullptr) {
    _initialize();
    _allocator = get_allocator_from_platform(_total_byte_size, _platform);
    if (!_allocator) {
        LOG_ERR("Failed to init Mat!");
        return;
    }

    bool res = _allocator->get_data(&_data);
    if (!res) {
        LOG_ERR("Failed to get Mat data address!");
        return;
    }
    // TODO: get both virtual/physic data address for specific platform
    // eg:hisi amba. chenlong22@baidu.com
}

Mat::Mat(
        Size size,
        FCVImageType type,
        int stride,
        PlatformType platform) :
        _width(size.width()),
        _height(size.height()),
        _stride(stride),
        _type(type),
        _platform(platform),
        _data(nullptr),
        _allocator(nullptr) {
    _initialize();
    _allocator = get_allocator_from_platform(_total_byte_size, _platform);
    if (!_allocator) {
        LOG_ERR("Failed to init Mat!");
        return;
    }

    bool res = _allocator->get_data(&_data);

    if (!res) {
        LOG_ERR("Failed to get Mat data address!");
        return;
    }
}

Mat::~Mat() { _allocator = nullptr; }

int Mat::width() const { return _width; }

int Mat::height() const { return _height; }

Size2i Mat::size() const {
    return Size2i(_width, _height);
}

int Mat::channels() const { return _channels; }

int Mat::stride() const { return _stride; }

FCVImageType Mat::type() const { return _type; }

int Mat::type_byte_size() const {
    return _type_byte_size;
}

uint64_t Mat::total_byte_size() const {
    return _total_byte_size;
}

bool Mat::empty() const {
    return (!_data) || (_width == 0) || (_height == 0);
}

void* Mat::data() const {
    return _data;
}

Mat Mat::clone() const {
    Mat tmp(_width, _height, _type, _stride, _platform);
    memcpy(tmp.data(), _data, _total_byte_size);
    return tmp;
}

int Mat::_initialize() {
    TypeInfo type_info;
    int status = get_type_info(_type, type_info);

    if (status != 0) {
        LOG_ERR("Unsupport image type!");
        return -1;
    }

    _type_byte_size = type_info.type_byte_size;
    _channels = type_info.channels;
    _pixel_offset = type_info.pixel_offset;

    parse_type_info(type_info, _width, _height,
            _channel_offset, _stride, _total_byte_size);

    return 0;
}

void* Mat::_get_pixel_address(int x, int y, int c) const {
    if (x < 0 || y < 0 || c < 0 || x >= _width
            || y >= _height || c >= _channels) {
        LOG_ERR("The pixel coordinate (%d, %d, %d) is out of range", x, y, c);
        return nullptr;
    }

    char* ptr = nullptr;
    char* data = reinterpret_cast<char*>(_data);
    if (_channel_offset >= 0) { // RGB
        ptr = data + (y * _stride) + (x * _pixel_offset) + (c * _channel_offset);
    } else { // YUV
        if (c == 0) { // Y : the same calculation formula
            ptr = data + y * _stride + x;
        } else if (c == 1) { // UV planar
            switch (_type) {
            case FCVImageType::NV12:
            case FCVImageType::NV21:
                ptr = data + _height * _stride + (y >> 1) * _stride + ((x >> 1) << 1);
                break;
            case FCVImageType::I420:
                ptr = data + _height * _stride + ((y * _stride) >> 2) + (x >> 1);
                break;
            default:
                break;
            }
        } else if (c == 2) {
            switch (_type) {
            case FCVImageType::NV12:
            case FCVImageType::NV21:
                ptr = data + _height * _stride + (y >> 1) * _stride + (x | int(1));
                break;
            case FCVImageType::I420:
                ptr = data + _height * _stride + (_height >> 1) * (_stride >> 1) + (y >> 1) * (_stride >> 1) + (x >> 1);
                break;
            default:
                break;
            }
        }
    }

    return ptr;
}

G_FCV_NAMESPACE1_END()
