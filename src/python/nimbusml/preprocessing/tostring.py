# --------------------------------------------------------------------------------------------
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
# --------------------------------------------------------------------------------------------
# - Generated by tools/entrypoint_compiler.py: do not edit by hand
"""
ToString
"""

__all__ = ["ToString"]


from sklearn.base import TransformerMixin

from ..base_transform import BaseTransform
from ..internal.core.preprocessing.tostring import ToString as core
from ..internal.utils.utils import trace


class ToString(core, BaseTransform, TransformerMixin):
    """
    **Description**
        Turns the given column into a column of its string representation

    :param columns: see `Columns </nimbusml/concepts/columns>`_.

    :param params: Additional arguments sent to compute engine.

    """

    @trace
    def __init__(
            self,
            columns=None,
            **params):

        if columns:
            params['columns'] = columns
        BaseTransform.__init__(self, **params)
        core.__init__(
            self,
            **params)
        self._columns = columns

    def get_params(self, deep=False):
        """
        Get the parameters for this operator.
        """
        return core.get_params(self)
