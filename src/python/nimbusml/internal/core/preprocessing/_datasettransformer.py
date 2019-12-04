# --------------------------------------------------------------------------------------------
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
# --------------------------------------------------------------------------------------------
# - Generated by tools/entrypoint_compiler.py: do not edit by hand
"""
DatasetTransformer
"""

__all__ = ["DatasetTransformer"]


from ...entrypoints.models_datasettransformer import models_datasettransformer
from ...utils.utils import trace
from ..base_pipeline_item import BasePipelineItem, DefaultSignature


class DatasetTransformer(BasePipelineItem, DefaultSignature):
    """
    **Description**
        Applies a TransformModel to a dataset.

    :param transform_model: Transform model.

    :param params: Additional arguments sent to compute engine.

    """

    @trace
    def __init__(
            self,
            transform_model,
            **params):
        BasePipelineItem.__init__(
            self, type='transform', **params)

        self.transform_model = transform_model

    @property
    def _entrypoint(self):
        return models_datasettransformer

    @trace
    def _get_node(self, **all_args):
        algo_args = dict(
            transform_model=self.transform_model)

        all_args.update(algo_args)
        return self._entrypoint(**all_args)