/*
 *COPYRIGHT
 *All modification made by Intel Corporation: © 2017 Intel Corporation.
 *Copyright (c) 2015 Preferred Infrastructure, Inc.
 *Copyright (c) 2015 Preferred Networks, Inc.
 *
 *Permission is hereby granted, free of charge, to any person obtaining a copy
 *of this software and associated documentation files (the "Software"), to deal
 *in the Software without restriction, including without limitation the rights
 *to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *copies of the Software, and to permit persons to whom the Software is
 *furnished to do so, subject to the following conditions:
 *
 *The above copyright notice and this permission notice shall be included in
 *all copies or substantial portions of the Software.
 *
 *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *THE SOFTWARE.
 *
 *
 *######################################################################
 *# The CuPy is designed based on NumPy's API.
 *# CuPy's source code and documents contain the original NumPy ones.
 *######################################################################
 *Copyright (c) 2005-2016, NumPy Developers.
 *All rights reserved.
 *
 *Redistribution and use in source and binary forms, with or without
 *modification, are permitted provided that the following conditions are
 *met:
 *
 *    * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *    * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *
 *    * Neither the name of the NumPy Developers nor the names of any
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *######################################################################
 */


#ifndef _DECONVOLUTION_H_
#define _DECONVOLUTION_H_

#include <mkldnn.hpp>
#include <vector>
#include <memory>
#include "layer.h"
#include "layer_factory.h"

template <typename T>
class DeConvolution2D : public Layer<T>
{
private:
static DeConvolution2D<T>* get_forward_object(
                    T* x, int x_d1, int x_d2, int x_d3, int x_d4,
                    T* W, int W_d1, int W_d2, int W_d3, int W_d4,
                    T* b, int b_d1,
                    T* y, int y_d1, int y_d2, int y_d3, int y_d4,
                    int ksize_h, int ksize_w,
                    int stride_y, int stride_x,
                    int pad_l_h, int pad_l_w,
                    int pad_r_h, int pad_r_w)
{
    DeConvolution2D<T>* deconv2d_forward = NULL;
    deconv2d_forward = dynamic_cast<DeConvolution2D<T>*> (
                        LayerFactory<T>::get_instance().get_deconv2d_layer(
                            x_d1, x_d2, x_d3, x_d4,
                            W_d1, W_d2, W_d3, W_d4,
                            b_d1,
                            ksize_h, ksize_w,
                            stride_y, stride_x,
                            pad_l_h, pad_l_w,
                            pad_r_h, pad_r_w));
    if (deconv2d_forward == NULL) {
        deconv2d_forward = new DeConvolution2D();
        LayerFactory<T>::get_instance().set_deconv2d_layer(
                            x_d1, x_d2, x_d3, x_d4,
                            W_d1, W_d2, W_d3, W_d4,
                            b_d1,
                            ksize_h, ksize_w,
                            stride_y, stride_x,
                            pad_l_h, pad_l_w,
                            pad_r_h, pad_r_w,
                            deconv2d_forward);
    }

    return deconv2d_forward;
}

static DeConvolution2D<T>* get_backward_object(
                    T* x,  int x_d1, int x_d2, int x_d3, int x_d4,
                    T* W,  int W_d1, int W_d2, int W_d3, int W_d4,
                    T* b,  int b_d1,
                    int ksize_h, int ksize_w,
                    int stride_y, int stride_x,
                    int pad_l_h, int pad_l_w,
                    int pad_r_h, int pad_r_w)
{
    DeConvolution2D<T>* deconv2d_backward;
    deconv2d_backward = dynamic_cast<DeConvolution2D<T>*>(
                        LayerFactory<T>::get_instance().get_deconv2d_layer(
                         x_d1, x_d2, x_d3, x_d4,
                         W_d1, W_d2, W_d3, W_d4,
                         b_d1,
                         ksize_h, ksize_w,
                         stride_y, stride_x,
                         pad_l_h, pad_l_w,
                         pad_r_h, pad_r_w));

    assert(deconv2d_backward != NULL); // we must have already done forward before

    return deconv2d_backward;

}

public:
static void do_forward(
                    T* x, int x_d1, int x_d2, int x_d3, int x_d4,
                    T* W, int W_d1, int W_d2, int W_d3, int W_d4,
                    T* b, int b_d1,
                    T* y, int y_d1, int y_d2, int y_d3, int y_d4,
                    int ksize_h, int ksize_w,
                    int stride_y, int stride_x,
                    int pad_l_h, int pad_l_w,
                    int pad_r_h, int pad_r_w)
{
    DeConvolution2D<T> *fwd_object = get_forward_object(
                                        x, x_d1, x_d2, x_d3, x_d4,
                                        W, W_d1, W_d2, W_d3, W_d4,
                                        b, b_d1,
                                        y, y_d1, y_d2, y_d3, y_d4,
                                        ksize_h, ksize_w,
                                        stride_y, stride_x,
                                        pad_l_h, pad_l_w,
                                        pad_r_h, pad_r_w);
    fwd_object->forward(
                    x, x_d1, x_d2, x_d3, x_d4,
                    W, W_d1, W_d2, W_d3, W_d4,
                    b, b_d1,
                    y, y_d1, y_d2, y_d3, y_d4,
                    stride_y, stride_x,
                    pad_l_h, pad_l_w,
                    pad_r_h, pad_r_w);
}

static void do_forward(
                    T* x, int x_d1, int x_d2, int x_d3, int x_d4,
                    T* W, int W_d1, int W_d2, int W_d3, int W_d4,
                    T* y, int y_d1, int y_d2, int y_d3, int y_d4,
                    int ksize_h, int ksize_w,
                    int stride_y, int stride_x,
                    int pad_l_h, int pad_l_w,
                    int pad_r_h, int pad_r_w)
{
    do_forward(
            x, x_d1, x_d2, x_d3, x_d4,
            W, W_d1, W_d2, W_d3, W_d4,
            NULL, -1,
            y, y_d1, y_d2, y_d3, y_d4,
            ksize_h, ksize_w,
            stride_y, stride_x,
            pad_l_h, pad_l_w,
            pad_r_h, pad_r_w);
}

static void do_backward(
                    T* x,  int x_d1, int x_d2, int x_d3, int x_d4,
                    T* W,  int W_d1, int W_d2, int W_d3, int W_d4,
                    T* b,  int b_d1,
                    T* gy, int gy_d1, int gy_d2, int gy_d3, int gy_d4,
                    T* gW, int gW_d1, int gW_d2, int gW_d3, int gW_d4,
                    T* gx, int gx_d1, int gx_d2, int gx_d3, int gx_d4,
                    T* gb, int gb_d1,
                    int ksize_h, int ksize_w,
                    int stride_y, int stride_x,
                    int pad_l_h, int pad_l_w,
                    int pad_r_h, int pad_r_w,
                    bool first_layer)
{
    DeConvolution2D<T> *bwd_object = get_backward_object(
                                    x, x_d1, x_d2, x_d3, x_d4,
                                    W, W_d1, W_d2, W_d3, W_d4,
                                    b, b_d1,
                                    ksize_h, ksize_w,
                                    stride_y, stride_x,
                                    pad_l_h, pad_l_w,
                                    pad_r_h, pad_r_w);
    bwd_object->backward(
                    x, x_d1, x_d2, x_d3, x_d4,
                    W, W_d1, W_d2, W_d3, W_d4,
                    b, b_d1,
                    gy, gy_d1, gy_d2, gy_d3, gy_d4,
                    gW, gW_d1, gW_d2, gW_d3, gW_d4,
                    gx, gx_d1, gx_d2, gx_d3, gx_d4,
                    gb, gb_d1,
                    first_layer);
}

static void do_backward(
                    T* x,  int x_d1, int x_d2, int x_d3, int x_d4,
                    T* W,  int W_d1, int W_d2, int W_d3, int W_d4,
                    T* gy, int gy_d1, int gy_d2, int gy_d3, int gy_d4,
                    T* gW, int gW_d1, int gW_d2, int gW_d3, int gW_d4,
                    T* gx, int gx_d1, int gx_d2, int gx_d3, int gx_d4,
                    int ksize_h, int ksize_w,
                    int stride_y, int stride_x,
                    int pad_l_h, int pad_l_w,
                    int pad_r_h, int pad_r_w,
                    bool first_layer)
{
    do_backward(
            x, x_d1, x_d2, x_d3, x_d4,
            W, W_d1, W_d2, W_d3, W_d4,
            NULL, -1,
            gy, gy_d1, gy_d2, gy_d3, gy_d4,
            gW, gW_d1, gW_d2, gW_d3, gW_d4,
            gx, gx_d1, gx_d2, gx_d3, gx_d4,
            NULL, -1,
            ksize_h, ksize_w,
            stride_y, stride_x,
            pad_l_h, pad_l_w,
            pad_r_h, pad_r_w,
            first_layer);
}

public:
    DeConvolution2D();
    ~DeConvolution2D();

    /*
     * Convolution forward primitive setup
     * Params:
     * X: input, (n,c,h,w)
     * W: weight, (n, out_c, h, w)
     * b: bias
     * y: output, (n, out_c, out_h, out_w)
     */
    void forward_setup(T* x, int x_d1, int x_d2, int x_d3, int x_d4,
            T* W, int W_d1, int W_d2, int W_d3, int W_d4,
            T* b, int b_d1,
            T* y, int y_d1, int y_d2, int y_d3, int y_d4,
            int s1, int s2,
            int pl1, int pl2,
            int pr1, int pr2);

    /*
     * Convolution forward with bias
     * Params:
     * X: input, (n,c,h,w)
     * W: weight, (n, out_c, h, w)
     * b: bias
     * y: output, (n, out_c, out_h, out_w)
     */
    int forward(T* x, int x_d1, int x_d2, int x_d3, int x_d4,
            T* W, int W_d1, int W_d2, int W_d3, int W_d4,
            T* b, int b_d1,
            T* y, int y_d1, int y_d2, int y_d3, int y_d4,
            int s1, int s2,
            int pl1, int pl2,
            int pr1, int pr2);

    /*
     * Convolution forward without bias
     * Params:
     * X: input, (n,c,h,w)
     * W: weight, (n, out_c, h, w)
     * y: output, (n, out_c, out_h, out_w)
     */
    int forward(T* x, int x_d1, int x_d2, int x_d3, int x_d4,
            T* W, int W_d1, int W_d2, int W_d3, int W_d4,
            T* y, int y_d1, int y_d2, int y_d3, int y_d4,
            int s1, int s2,
            int pl1, int pl2,
            int pr1, int pr2);

    /*
     * Covolution backward primitive setup
     * Params:
     *
     */
    void backward_setup(T* x, int x_d1, int x_d2, int x_d3, int x_d4,
            T* W, int W_d1, int W_d2, int W_d3, int W_d4,
            T* b, int b_d1,
            T* gy, int gy_d1, int gy_d2, int gy_d3, int gy_d4,
            T* gW, int gW_d1, int gW_d2, int gW_d3, int gW_d4,
            T* gx, int gx_d1, int gx_d2, int gx_d3, int gx_d4,
            T* gb, int gb_d1);

    int backward( T* x, int x_d1, int x_d2, int x_d3, int x_d4,
            T* W, int W_d1, int W_d2, int W_d3, int W_d4,
            T* b, int b_d1,
            T* gy, int gy_d1, int gy_d2, int gy_d3, int gy_d4,
            T* gW, int gW_d1, int gW_d2, int gW_d3, int gW_d4,
            T* gx, int gx_d1, int gx_d2, int gx_d3, int gx_d4,
            T* gb, int gb_d1,
            bool first_layer);

    int backward( T* x, int x_d1, int x_d2, int x_d3, int x_d4,
            T* W, int W_d1, int W_d2, int W_d3, int W_d4,
            T* gy, int gy_d1, int gy_d2, int gy_d3, int gy_d4,
            T* gW, int gW_d1, int gW_d2, int gW_d3, int gW_d4,
            T* gx, int gx_d1, int gx_d2, int gx_d3, int gx_d4,
            bool first_layer);
private:
    // convolution primitive
    std::shared_ptr<mkldnn::primitive> deconv_fwd_;
    std::shared_ptr<mkldnn::primitive> deconv_bwd_data_;
    std::shared_ptr<mkldnn::primitive> deconv_bwd_weights_;
    //memory reorder primitive
    //forward
    mkldnn::primitive deconv_reorder_src_; // reorder x
    mkldnn::primitive deconv_reorder_weights_; //reorder W
    mkldnn::primitive deconv_reorder_dst_; //reorder y
    //backward
    mkldnn::primitive deconv_bwd_reorder_src_; // reorder x
    mkldnn::primitive deconv_bwd_reorder_diff_weights_; //reorder gW
    mkldnn::primitive deconv_bwd_reorder_dst_weights_; //reorder gY for gW
    mkldnn::primitive deconv_bwd_reorder_dst_data_; //reorder gY for gX
    mkldnn::primitive deconv_bwd_reorder_weights_; //reorder W
    mkldnn::primitive deconv_bwd_reorder_diff_src_; //reorder gX

    bool fwd_reorder_deconv_src_ = false;
    bool fwd_reorder_deconv_weights_ = false;
    bool fwd_reorder_deconv_dst_ = false;

    bool bwd_reorder_src_ = false;
    bool bwd_reorder_diff_dst_weights_ = false; // reorder gy for gW
    bool bwd_reorder_diff_dst_data_ = false; // reorder gy for gX
    bool bwd_reorder_diff_weights_ = false;
    bool bwd_reorder_weights_ = false;
    bool bwd_reorder_diff_src_ = false;

    bool fwd_first_run_ = true;
    bool bwd_first_run_ = true;

    //desc & prmitive desc
    //forward
    std::shared_ptr<mkldnn::convolution_forward::desc> fwd_desc_;
    std::shared_ptr<mkldnn::convolution_forward::primitive_desc> fwd_pd_;
    //backward
    std::shared_ptr<mkldnn::convolution_backward_weights::desc> bwd_weights_desc_;
    std::shared_ptr<mkldnn::convolution_backward_weights::primitive_desc> bwd_weights_pd_;
    std::shared_ptr<mkldnn::convolution_backward_data::desc> bwd_data_desc_;
    std::shared_ptr<mkldnn::convolution_backward_data::primitive_desc> bwd_data_pd_;

    //stream
    std::shared_ptr<mkldnn::stream> fwd_stream_;
    std::vector<mkldnn::primitive> fwd_primitives_;
    std::shared_ptr<mkldnn::stream> bwd_weights_stream_;
    std::vector<mkldnn::primitive> bwd_weights_primitives_;
    std::shared_ptr<mkldnn::stream> bwd_data_stream_;
    std::vector<mkldnn::primitive> bwd_data_primitives_;

    //memory dims
    mkldnn::memory::dims src_tz_;
    mkldnn::memory::dims weights_tz_;
    mkldnn::memory::dims dst_tz_;
    mkldnn::memory::dims bias_tz_;
    mkldnn::memory::dims strides_;
    mkldnn::memory::dims padding_l_;
    mkldnn::memory::dims padding_r_;

    //user memory
    //forward
    std::shared_ptr<mkldnn::memory> user_src_mem_; //x
    std::shared_ptr<mkldnn::memory> user_weights_mem_; //W
    std::shared_ptr<mkldnn::memory> user_bias_mem_; //b
    std::shared_ptr<mkldnn::memory> user_dst_mem_; //y
    //backward
    std::shared_ptr<mkldnn::memory> user_bwd_diff_src_mem_; //gX
    std::shared_ptr<mkldnn::memory> user_bwd_diff_weights_mem_; //gW
    std::shared_ptr<mkldnn::memory> user_bwd_diff_bias_mem_; //gb
    std::shared_ptr<mkldnn::memory> user_bwd_diff_dst_mem_; //gy
    std::shared_ptr<mkldnn::memory> user_bwd_src_mem_; //x
    std::shared_ptr<mkldnn::memory> user_bwd_weights_mem_; //W
//    std::shared_ptr<mkldnn::memory> user_bwd_dst_mem_; //y

    //MKLDNN memory
    //forward
    std::shared_ptr<mkldnn::memory> src_mem_; // x
    std::shared_ptr<mkldnn::memory> weights_mem_;// W
    std::shared_ptr<mkldnn::memory> bias_mem_;// b
    std::shared_ptr<mkldnn::memory> dst_mem_; //y
    //backward
    std::shared_ptr<mkldnn::memory> bwd_src_mem_; // x
    std::shared_ptr<mkldnn::memory> bwd_weights_mem_; //W
    std::shared_ptr<mkldnn::memory> bwd_diff_weights_mem_; //gW
    std::shared_ptr<mkldnn::memory> bwd_diff_src_mem_; //gX
    std::shared_ptr<mkldnn::memory> bwd_diff_bias_mem_; //gb
    std::shared_ptr<mkldnn::memory> bwd_diff_dst_weights_mem_; // gy for gW
    std::shared_ptr<mkldnn::memory> bwd_diff_dst_data_mem_; // gy for gx

    //memory desc
    //forward & backward can share same mem desc
    std::shared_ptr<mkldnn::memory::desc> src_md_; //x & gx
    std::shared_ptr<mkldnn::memory::desc> weights_md_;// W & gW
    std::shared_ptr<mkldnn::memory::desc> bias_md_; // b & gb
    std::shared_ptr<mkldnn::memory::desc> dst_md_; // y & gy
};

#endif // _DECONVOLUTION_H_


// vim: et ts=4 sw=4 cindent cino^=l0,\:0,N-s
