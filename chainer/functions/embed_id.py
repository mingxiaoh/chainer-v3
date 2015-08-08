import numpy

from chainer import cuda
from chainer import function
from chainer.utils import type_check


class EmbedID(function.Function):

    """Efficient linear function for one-hot input.

    This is a parameterized function to embed the given discrete identifier
    (e.g. word) into a continuous vector space. This function just holds
    embedding vectors for all identifiers as one large matrix ``W``, which is
    learnable. The identifiers are directly used as indexes of the matrix
    ``W``.

    Args:
        in_size (int): Number of different identifiers (a.k.a. vocabulary
            size).
        out_size (int): Size of embedding vector.

    .. note::

       This function is non-differentiable with respect to the input
       identifiers.

    """
    parameter_names = ('W',)
    gradient_names = ('gW',)

    def __init__(self, in_size, out_size):
        self.W = numpy.random.randn(in_size, out_size).astype(numpy.float32)
        self.gW = numpy.empty_like(self.W)

    def check_type_forward(self, in_types):
        type_check.expect(in_types.size() == 1)
        x_type, = in_types

        type_check.expect(
            x_type.dtype == numpy.int32,
            x_type.ndim == 1,
        )

    def check_type_backward(self, in_types, out_types):
        type_check.expect(
            in_types.size() == 1,
            out_types.size() == 1,
        )
        x_type, = in_types
        y_type, = out_types

        type_check.expect(
            y_type.dtype == numpy.float32,
            y_type.ndim == 2,
            y_type.shape[0] == x_type.shape[0],
            y_type.shape[1] == type_check.Variable(self.W.shape[1],
                                                   'W.shape[1]'),
        )

    def forward(self, x):
        return self.W.take(x[0], axis=0),

    def backward_cpu(self, x, gy):
        numpy.add.at(self.gW, x[0], gy[0])
        return None,

    def backward_gpu(self, x, gy):
        cuda.elementwise(
            'T gy, raw int32 x, int32 n_out', 'raw T gW',
            'atomicAdd(&gW[x[i / n_out] * n_out + i % n_out], gy)',
            'embed_id_bwd')(gy[0], x[0], self.gW.shape[1], self.gW)
        return None,
