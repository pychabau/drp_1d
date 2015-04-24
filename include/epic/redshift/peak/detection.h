#ifndef _REDSHIFT_PEAK_DETECTION_
#define _REDSHIFT_PEAK_DETECTION_

#include <epic/core/common/managedobject.h>
#include <epic/core/common/datatypes.h>
#include <epic/core/common/range.h>

namespace NSEpic
{

class CSpectrum;
class CSpectrumAxis;
class CPeakDetection : public CManagedObject
{
    DEFINE_MANAGED_OBJECT( CPeakDetection )

public:

    CPeakDetection();
    virtual ~CPeakDetection();

    Bool Compute( const CSpectrum& spectrum, const TLambdaRange& lambdaRange, Float64 windowSize = 250.0, Float64 cut = 5.0, UInt32 medianSmoothHalfWidth = 1, UInt32 enlargeRate = 2.0 );

    const TInt32RangeList& GetResults() const;
    const TInt32RangeList& GetResultsEnlarged() const;

private:

    Void FindPossiblePeaks( const CSpectrumAxis& smoothedFluxAxis, const CSpectrumAxis& spectralAxis, UInt32 windowSampleCount, Float64 cut, TInt32RangeList& peakList );
    Void RedefineBorders( TInt32RangeList& peakList, const CSpectrumAxis& waves, const CSpectrumAxis& smoothFluxAxis, const CSpectrumAxis& fluxAxis );
    TInt32Range FindGaussianFitStartAndStop( Int32 i, const TInt32RangeList& peaksBorders, UInt32 enlargeRate, Int32 len );

    Float64 XMad( const Float64* x, Int32 n, Float64 median );

    TInt32RangeList            m_Results;
    TInt32RangeList            m_ResultsEnlargedForFit;

};

}

#endif
