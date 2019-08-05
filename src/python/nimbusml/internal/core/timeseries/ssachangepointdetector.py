# --------------------------------------------------------------------------------------------
# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.
# --------------------------------------------------------------------------------------------
# - Generated by tools/entrypoint_compiler.py: do not edit by hand
"""
SsaChangePointDetector
"""

__all__ = ["SsaChangePointDetector"]


from ...entrypoints.timeseriesprocessingentrypoints_ssachangepointdetector import \
    timeseriesprocessingentrypoints_ssachangepointdetector
from ...utils.utils import trace
from ..base_pipeline_item import BasePipelineItem, DefaultSignature


class SsaChangePointDetector(BasePipelineItem, DefaultSignature):
    """

    This transform detects the change-points in a seasonal time-series
    using Singular Spectrum Analysis (SSA).

    .. remarks::
        `Singular Spectrum Analysis (SSA)
        <https://en.wikipedia.org/wiki/Singular_spectrum_analysis>`_ is a
        powerful framework for decomposing the time-series into trend,
        seasonality and noise components as well as forecasting the future
        values of the time-series. In order to remove the
        effect of such components on anomaly detection, this transform add
        SSA as a time-series modeler component in the detection pipeline.

        The SSA component will be trained and it predicts the next expected
        value on the time-series under normal condition; this expected value
        is
        further used to calculate the amount of deviation from the normal
        behavior at that timestamp.
        The distribution of this deviation is then modeled using `Adaptive
        kernel density estimation
        <https://en.wikipedia.org/wiki/Variable_kernel_density_estimation>`_.

        This transform detects
        change points by calculating the martingale score for the sliding
        window based on the estimated distribution of deviations.
        The idea is based on the `Exchangeability
        Martingales <https://icml.cc/Conferences/2012/papers/808.pdf>`_ that
        detects a change of distribution over a stream of i.i.d. values. In
        short, the value of the
        martingale score starts increasing significantly when a sequence of
        small p-values detected in a row; this
        indicates the change of the distribution of the underlying data
        generation process.

    :param training_window_size: The number of points, N, from the beginning
        of the sequence used to train the SSA model.

    :param confidence: The confidence for change point detection in the range
        [0, 100].

    :param seasonal_window_size: An upper bound, L,  on the largest relevant
        seasonality in the input time-series, which also
        determines the order of the autoregression of SSA. It must satisfy 2
        < L < N/2.

    :param change_history_length: The length of the sliding window on p-value
        for computing the martingale score.

    :param error_function: The function used to compute the error between the
        expected and the observed value. Possible values are:
        {``SignedDifference``, ``AbsoluteDifference``, ``SignedProportion``,
        ``AbsoluteProportion``, ``SquaredDifference``}.

    :param martingale: The type of martingale betting function used for
        computing the martingale score. Available options are {``Power``,
        ``Mixture``}.

    :param power_martingale_epsilon: The epsilon parameter for the Power
        martingale if martingale is set to ``Power``.

    :param params: Additional arguments sent to compute engine.

    .. seealso::
        :py:func:`IIDChangePointDetector
        <nimbusml.preprocessing.timeseries.IIDChangePointDetector>`,
        :py:func:`IIDSpikeDetector
        <nimbusml.preprocessing.timeseries.IIDSpikeDetector>`,
        :py:func:`SsaSpikeDetector
        <nimbusml.preprocessing.timeseries.SsaSpikeDetector>`.

    .. index:: models, timeseries, transform

    Example:
       .. literalinclude:: /../nimbusml/examples/SsaChangePointDetector.py
              :language: python
    """

    @trace
    def __init__(
            self,
            training_window_size=100,
            confidence=95.0,
            seasonal_window_size=10,
            change_history_length=20,
            error_function='SignedDifference',
            martingale='Power',
            power_martingale_epsilon=0.1,
            **params):
        BasePipelineItem.__init__(
            self, type='transform', **params)

        self.training_window_size = training_window_size
        self.confidence = confidence
        self.seasonal_window_size = seasonal_window_size
        self.change_history_length = change_history_length
        self.error_function = error_function
        self.martingale = martingale
        self.power_martingale_epsilon = power_martingale_epsilon

    @property
    def _entrypoint(self):
        return timeseriesprocessingentrypoints_ssachangepointdetector

    @trace
    def _get_node(self, **all_args):
        algo_args = dict(
            source=self.source,
            name=self._name_or_source,
            training_window_size=self.training_window_size,
            confidence=self.confidence,
            seasonal_window_size=self.seasonal_window_size,
            change_history_length=self.change_history_length,
            error_function=self.error_function,
            martingale=self.martingale,
            power_martingale_epsilon=self.power_martingale_epsilon)

        all_args.update(algo_args)
        return self._entrypoint(**all_args)