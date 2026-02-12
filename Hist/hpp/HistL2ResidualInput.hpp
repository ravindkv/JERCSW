
#pragma once

struct HistL2ResidualInput {
    double ptAvgProj   = 0.0;
    double ptAverage   = 0.0;
    double ptProbe      = 0.0;
    double ptMet   = 0.0;
    double ptOther   = 0.0;
    double ptUnclustered   = 0.0;
    double etaTag     = 0.0;
    double etaProbe     = 0.0;
    double phiProbe     = 0.0;
    double ptTag    = 0.0;
    double massTag    = 0.0;
    double asymmA    = 0.0;
    double asymmB    = 0.0;
    double phiTag   = 0.0;
    double relDbResp    = 0.0;
    double relMpfResp    = 0.0;
    double relMpfxResp    = 0.0;
    double respMetOnBisector         = 0.0;
    double respSumTnPonBisector         = 0.0;
    double respProbeOnBisector         = 0.0;
    double respTagOnBisector         = 0.0;
    double respSumOtherOnBisector         = 0.0;
    double respUnclusteredOnBisector         = 0.0;

    double respMetOnMean         = 0.0;
    double respSumTnPonMean         = 0.0;
    double respProbeOnMean         = 0.0;
    double respTagOnMean         = 0.0;
    double respSumOtherOnMean         = 0.0;
    double respUnclusteredOnMean         = 0.0;

    double respMetOnProbe         = 0.0;
    double respSumTnPonProbe         = 0.0;
    double respProbeOnProbe         = 0.0;
    double respTagOnProbe         = 0.0;
    double respSumOtherOnProbe         = 0.0;
    double respUnclusteredOnProbe         = 0.0;

    double respMetOnTag         = 0.0;
    double respMetOnTagx        = 0.0;
    double respSumTnPonTag         = 0.0;
    double respProbeOnTag         = 0.0;
    double respTagOnTag         = 0.0;
    double respSumOtherOnTag         = 0.0;
    double respUnclusteredOnTag         = 0.0;
    double weight      = 1.0;
};
