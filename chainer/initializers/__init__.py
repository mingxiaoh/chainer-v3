import numpy

from chainer.initializers import constant  # NOQA
from chainer.initializers import normal  # NOQA
from chainer.initializers import orthogonal  # NOQA
from chainer.initializers import uniform  # NOQA


# import class and function
from chainer.initializers.constant import Constant
from chainer.initializers.constant import Identity  # NOQA
from chainer.initializers.constant import One  # NOQA
from chainer.initializers.constant import Zero  # NOQA
from chainer.initializers.normal import GlorotNormal  # NOQA
from chainer.initializers.normal import HeNormal
from chainer.initializers.normal import Normal  # NOQA
from chainer.initializers.orthogonal import Orthogonal  # NOQA
from chainer.initializers.uniform import GlorotUniform  # NOQA
from chainer.initializers.uniform import HeUniform  # NOQA
from chainer.initializers.uniform import LeCunUniform  # NOQA
from chainer.initializers.uniform import Uniform  # NOQA


def generate_array(initializer, shape, xp):
    """Return initialized array.

    The algorithms used to make the new values depend on the
    concrete derived classes. The dtype of a generated array depends on
    ``initializer.dtype``.

    Args:
        initializer: A callable object that takes :class:`numpy.ndarray`
             or :class:`cupy.ndarray` and edits its value.
        shape (tuple): Shape of a return array.
        xp (module): :mod:`cupy` or :mod:`numpy`.

    Returns:
        numpy.ndarray or cupy.ndarray: An initialized array.

    """
    dtype = numpy.float32
    if hasattr(initializer, 'dtype') and initializer.dtype is not None:
        dtype = initializer.dtype
    array = xp.empty(shape, dtype=dtype)
    initializer(array)
    return array


def init_weight(weights, initializer):
    """Helper function for initialization of the weight tensor.

    This function accepts several types of initializer, prepares
    the appropriate ``~chainer.Initializer`` if necessary,
    and does the initialization.

    Args:
         weights (numpy.ndarray or cupy.ndarray):
             Weight tensor to be initialized.
         initializer: The value used to initialize the data.
             May be ``None`` (in which case
             :class:`~chainer.initializers.HeNormal`
             is used as an initializer), a scalar to set all values to,
             an ``numpy.ndarray`` to be assigned,
             or a callable that takes :class:`numpy.ndarray`
             or :class:`cupy.ndarray` and edits its value.

    """

    if initializer is None:
        initializer = HeNormal(1 / numpy.sqrt(2))
    elif numpy.isscalar(initializer):
        initializer = Constant(initializer)
    elif isinstance(initializer, numpy.ndarray):
        initializer = Constant(initializer)

    assert callable(initializer)
    initializer(weights)


def _get_initializer(initializer):
    if initializer is None:
        return HeNormal(1 / numpy.sqrt(2))
    if numpy.isscalar(initializer):
        return Constant(initializer)
    if isinstance(initializer, numpy.ndarray):
        return Constant(initializer)

    assert callable(initializer)
    return initializer
