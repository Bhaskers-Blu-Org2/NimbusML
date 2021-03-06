# --------------------------------------------------------------------------------------------
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
# --------------------------------------------------------------------------------------------
# - Generated by tools/entrypoint_compiler.py: do not edit by hand
"""
ClassifierDisagreement
"""

__all__ = ["ClassifierDisagreement"]


from .....utils.entrypoints import Component
from .....utils.utils import trace


class ClassifierDisagreement(Component):
    """
    **Description**
        A measure of disagreement in predictions between a pair of classifiers, averaged over all pairs

    :param params: Additional arguments sent to compute engine.

    """

    @trace
    def __init__(
            self,
            **params):

        self.kind = 'EnsembleMulticlassDiversityMeasure'
        self.name = 'MultiDisagreementDiversityMeasure'
        self.settings = {}

        super(
            ClassifierDisagreement,
            self).__init__(
            name=self.name,
            settings=self.settings,
            kind=self.kind)
