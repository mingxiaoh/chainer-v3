from chainer import function
from chainer.utils import conv
from chainer.utils import type_check

import numpy
from mkldnn.chainer.runtime import Engine
from mkldnn.compute_complex import *

# Most important thing
from mkldnn.api.support import *

import mkldnn.api.memory as m

import mkldnn.api.convolution_forward as conv_forward
import mkldnn.api.convolution_backward_data as conv_backdata
import mkldnn.api.convolution_backward_weights as conv_backweights

conv_f_op = conv_forward.conv_f_op
conv_bd_op = conv_backdata.conv_bd_op

conv_bw_op = conv_backweights.conv_bw_op
conv_bwb_op = conv_backweights.conv_bwb_op

from mkldnn.mdarray import *

class conv_geometry(object):
    def __init__(self, x_shape, W_shape, stride, pad, cover_all):
        assert isinstance(x_shape, tuple), 'X shape must be tuple'
        assert isinstance(W_shape, tuple), 'W shape must be tuple'

        sy, sx = _pair(stride)
        p_upper, p_left = _pair(pad)

        out_c, _, kh, kw = W_shape
        n, c, h, w = x_shape

        out_h = conv.get_conv_outsize(h, kh, sy, p_upper, cover_all = cover_all)
        assert out_h > 0, 'Height in the output should be positive.'
        out_w = conv.get_conv_outsize(w, kw, sx, p_left, cover_all = cover_all)
        assert out_w > 0, 'Width in the output should be positive.'

        p_down = sy * (out_h -1) + kh - h - p_upper
        p_right = sx * (out_w -1) + kw - w - p_left

        self.p_upper = p_upper
        self.p_left = p_left
        self.p_down = p_down
        self.p_right = p_right
        self.out_h = out_h
        self.out_w = out_w

        self._out_shape = (n, out_c, out_h, out_w)
        self._geometry = (_pair(stride), (p_upper, p_left), (p_down, p_right))

    @property
    def out_shape(self):
        return self._out_shape

    @property
    def geometry(self):
        return self._geometry

def create_forward_desc(d_creator, o_expect, inputs, geometry):
    inputs_d = [m.desc(v.shape, m.memory.f32, m.memory.any)
            for v in inputs if v is not None]

    return d_creator(forward, convolution_direct,
            *inputs_d, o_expect, *geometry, zero)

def create_backward_desc(d_creator, inputs, geometry):
    inputs_d = [m.desc(v.shape, m.memory.f32, m.memory.any)
            for v in inputs if v is not None]

    return d_creator(convolution_direct, *inputs_d, *geometry, zero)

def _pair(x):
    if hasattr(x, '__getitem__'):
        return x
    return x, x

class ConvolutionForward(ComputeComplex):
    cc_type = 'f'

    def __init__(self, inputs, stride=1, pad=0, cover_all=False,
            pos = None, e=Engine()):

        x = inputs[0]
        W = inputs[1]
        b = inputs[2] if len(inputs) == 3 else None

        if self.new:
            self._create_cc(x, W, b, stride, pad, cover_all, e)
            self.num_inputs = len(inputs)
        else:
            self._reuse(x, W, b)

    def _create_cc(self, x, W, b, stride, pad, cover_all, e):
        super(ConvolutionForward, self).__init__()
        g = conv_geometry(x.shape, W.shape, stride, pad, cover_all)

        y_d = m.desc(g.out_shape, m.memory.f32, m.memory.any)

        self.geometry = g.geometry

        # Create primitive_desc from any
        cc_d = create_forward_desc(conv_forward.desc, y_d, (x, W, b), g.geometry)
        cc_pd = conv_forward.primitive_desc(cc_d, e)

        # Transform inputs, nothing will be done if mdarray
        self.x = array(x, m.memory.nchw, e)
        self.W = array(W, m.memory.oihw, e)
        if b is not None:
            self.b = array(b, m.memory.x, e)

        if b is None:
            y = conv_f_op(cc_pd, self.x, self.W, self.dag_)
        else:
            y = conv_f_op(cc_pd, self.x, self.W, self.b, self.dag_)

        self._hint = cc_pd
        self.outputs = y,

    def _reuse(self, x, W, b):
        reuse_buffer(self.x, x)
        reuse_buffer(self.W, W)
        if b is not None:
            reuse_buffer(self.b, b)

    def match(self, inputs, stride = 1, pad = 0, cover_all = False, **kwargs):
        x = inputs[0]
        W = inputs[1]
        g = conv_geometry(x.shape, W.shape, stride, pad, cover_all)
        return (self.geometry == g.geometry) and (self.num_inputs == len(inputs))

class ConvolutionBackwardData(ComputeComplex):
    cc_type = 'bd'

    def __init__(self, inputs, grad_outputs, hint,
            stride=1, pad=0, cover_all=False, pos = None, e=Engine()):
        x = inputs[0]
        W = inputs[1]
        gy = grad_outputs[0]

        if self.new:
            self._create_cc(x, W, gy, hint, stride, pad, cover_all, e)
        else:
            self._reuse(x, gy)

    def _create_cc(self, x, W, gy, hint, stride, pad, cover_all, e):
        super(ConvolutionBackwardData, self).__init__()

        g = conv_geometry(x.shape, W.shape, stride, pad, cover_all)

        # Create primitive descriptor
        cc_d = create_backward_desc(conv_backdata.desc, (x, W, gy), g.geometry)
        cc_pd = conv_backdata.primitive_desc(cc_d, e, hint)

        # Transform inputs
        self.gy = array(gy, m.memory.nchw, e)
        self.W = array(W, m.memory.oihw, e)

        gx = conv_bd_op(cc_pd, self.gy, self.W, self.dag_)

        self._hint = hint
        self.outputs = gx,

    def _reuse(self, W, gy):
        reuse_buffer(self.W, W)
        reuse_buffer(self.gy, gy)

    def match(self, inputs, grad_ouputs, hint, *args, **kwargs):
        return self.hint is hint

class ConvolutionBackwardWeighs(ComputeComplex):
    cc_type = 'bw'

    def __init__(self, inputs, grad_ouputs, hint,
            stride=1, pad=0, cover_all=False, pos = None, e=Engine()):
        x = inputs[0]
        W = inputs[1]
        b = inputs[2] if len(inputs) == 3 else None
        gy = grad_ouputs[0]

        if self.new:
            self._create_cc(x, W, b, gy, hint, stride, pad, cover_all, e)
        else:
            self._reuse_cc(x, gy)

    def _create_cc(self, x, W, b, gy, hint, stride, pad, cover_all, e):
        super(ConvolutionBackwardWeighs, self).__init__()

        g = conv_geometry(x.shape, W.shape, stride, pad, cover_all)

        cc_d = create_backward_desc(conv_backweights.desc, (x, W, b, gy), g.geometry)
        cc_pd = conv_backweights.primitive_desc(cc_d, e, hint)

        self.gy = array(gy, m.memory.nchw, e)
        self.x = array(x, m.memory.nchw, e)
        self._hint = hint

        # Prepare outputs mdarray
        # gW = mdarray(cc_pd.diff_weights_primitive_desc())
        # if b is not None:
        #     gb = mdarray(cc_pd.diff_bias_primitive_desc())

        if b is not None:
            # XXX: This is ugly, will use swig to do something about it
            # ideal: gW, gb = conv_bwb_op(cc_pd, self.x, self.gy, self.dag_)

            gW = conv_bwb_op(cc_pd, self.x, self.gy, self.dag_)
            gb = gW.extra
        else:
            gW = conv_bw_op(cc_pd, self.x, self.gy, self.dag_)

        if b is not None:
            self.outputs = gW, gb
        else:
            self.outputs = gW,

    def _reuse_cc(self, x, gy):
        reuse_buffer(self.x, x)
        reuse_buffer(self.gy, gy)

    def match(self, inputs, grad_ouputs, hint, *args, **kwargs):
        return self.hint is hint

class Convolution2DFunctionMKLDNN(function.Function):

    def __init__(self, stride=1, pad=0, cover_all=False, deterministic=False):
        self.sy, self.sx = _pair(stride)
        self.ph, self.pw = _pair(pad)
        self.cover_all = cover_all
        self.deterministic = deterministic

    def check_type_forward(self, in_types):
        n_in = in_types.size()
        type_check.expect(2 <= n_in, n_in <= 3)

        x_type = in_types[0]
        w_type = in_types[1]
        type_check.expect(
            x_type.dtype.kind == 'f',
            w_type.dtype.kind == 'f',
            x_type.ndim == 4,
            w_type.ndim == 4,
            x_type.shape[1] == w_type.shape[1],
        )

        if type_check.eval(n_in) == 3:
            b_type = in_types[2]
            type_check.expect(
                b_type.dtype == x_type.dtype,
                b_type.ndim == 1,
                b_type.shape[0] == w_type.shape[0],
            )

    def forward_cpu(self, inputs):

        cc = ConvolutionForward(inputs, stride = (self.sy, self.sx),
                pad = (self.ph, self.pw), cover_all = self.cover_all,
                pos=(self.rank, self.fanout))

        self.hint = cc.hint

        y, = cc.execute_on()

        return y,

    def backward_cpu(self, inputs, grad_outputs):

        cc_data = ConvolutionBackwardData(inputs, grad_outputs, self.hint,
                stride = (self.sy, self.sx), pad = (self.ph, self.pw),
                cover_all = self.cover_all, pos=(self.rank, self.fanout))

        cc_weight = ConvolutionBackwardWeighs(inputs, grad_outputs, self.hint,
                stride = (self.sy, self.sx), pad = (self.ph, self.pw),
                cover_all = self.cover_all, pos=(self.rank, self.fanout))

        gx = cc_data.execute_on()
        gW_b = cc_weight.execute_on()

        return gx + gW_b
