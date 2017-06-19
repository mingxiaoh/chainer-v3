/*******************************************************************************
* Copyright 2016-2017 Intel Corporation
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*     http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*******************************************************************************/

%module (package="mkldnn.api") relu_backward
%{
  #define SWIG_FILE_WITH_INIT
  #include <cstddef>
  #include <mkldnn.hpp>
  using mkldnn::handle_traits;
%}

%include stl.i
%include exception.i

%feature("flatnested");
%feature("nodefaultctor");

%import support.i
%import memory.i
%import relu_forward.i

namespace mkldnn {

%rename (desc) relu_backward::desc;
%rename (primitive_desc) relu_backward::primitive_desc;

%exception relu_backward::desc::desc {
  try {
    $action
  }
  catch (mkldnn::error &e){
    SWIG_exception(SWIG_ValueError, e.message.c_str());
  }
}

struct relu_backward : public primitive {
    struct desc {
        c_api::mkldnn_relu_desc_t data;
        desc(const memory::desc &diff_data_desc, const memory::desc &data_desc,
            double negative_slope);
    };

    struct primitive_desc : public handle<c_api::mkldnn_primitive_desc_t> {
        primitive_desc(const desc &adesc, const engine &aengine,
        const relu_forward::primitive_desc &hint_fwd_primitive_desc);
        
        memory::primitive_desc diff_src_primitive_desc() const;
    };

    relu_backward(const primitive_desc &aprimitive_desc,
            const primitive::at &src, const primitive::at &diff_dst,
            const memory &diff_src);
};

}