#include <epic/redshift/method/blindsolve.h>

#include <epic/core/debug/assert.h>
#include <epic/redshift/spectrum/template/catalog.h>
#include <epic/redshift/operator/correlation.h>
#include <epic/redshift/operator/chisquare.h>
#include <epic/redshift/extremum/extremum.h>
#include <epic/redshift/processflow/datastore.h>

using namespace NSEpic;
using namespace std;


COperatorBlindSolve::COperatorBlindSolve()
{

}

COperatorBlindSolve::~COperatorBlindSolve()
{

}

std::shared_ptr<const CBlindSolveResult> COperatorBlindSolve::Compute(  CDataStore& resultStore, const CSpectrum& spc, const CSpectrum& spcWithoutCont,
                                                        const CTemplateCatalog& tplCatalog, const TStringList& tplCategoryList,
                                                        const TFloat64Range& lambdaRange, const TFloat64Range& redshiftsRange, Float64 redshiftStep,
                                                        Int32 correlationExtremumCount, Float64 overlapThreshold )
{
    Bool storeResult = false;

    CDataStore::CAutoScope resultScope( resultStore, "blindsolve" );

    for( UInt32 i=0; i<tplCategoryList.size(); i++ )
    {
        std::string category = tplCategoryList[i];

        for( UInt32 j=0; j<tplCatalog.GetTemplateCount( category ); j++ )
        {
            const CTemplate& tpl = tplCatalog.GetTemplate( category, j );
            const CTemplate& tplWithoutCont = tplCatalog.GetTemplateWithoutContinuum( category, j );

            BlindSolve( resultStore, spc, spcWithoutCont, tpl, tplWithoutCont, lambdaRange, redshiftsRange, redshiftStep, correlationExtremumCount, overlapThreshold );

            storeResult = true;
        }
    }


    if( storeResult )
    {
        return  std::shared_ptr<const CBlindSolveResult>( new CBlindSolveResult() );
    }

    return NULL;
}

Bool COperatorBlindSolve::BlindSolve( CDataStore& resultStore, const CSpectrum& spc, const CSpectrum& spcWithoutCont, const CTemplate& tpl, const CTemplate& tplWithoutCont,
                               const TFloat64Range& lambdaRange, const TFloat64Range& redshiftsRange, Float64 redshiftStep, Int32 correlationExtremumCount,
                               Float64 overlapThreshold )
{
    CSpectrum s = spc;
    s.GetSpectralAxis().ConvertToLogScale();

    CTemplate t = tpl;
    t.GetSpectralAxis().ConvertToLogScale();


    // Create redshift initial list by spanning redshift acdross the given range, with the given delta
    TFloat64List redshifts = redshiftsRange.SpreadOver( redshiftStep );
    DebugAssert( redshifts.size() > 0 );

    // Compute correlation factor at each of those redshifts
    COperatorCorrelation correlation;
    std::shared_ptr<const CCorrelationResult> correlationResult = dynamic_pointer_cast<const CCorrelationResult> ( correlation.Compute( spcWithoutCont, tplWithoutCont, lambdaRange, redshifts, overlapThreshold ) );

    if( !correlationResult )
    {
        return false;
    }

    resultStore.StoreScopedPerTemplateResult( tpl, "correlation", correlationResult );

    // Find redshifts extremum
    TPointList extremumList;
    CExtremum extremum( redshiftsRange, correlationExtremumCount);
    extremum.Find( correlationResult->Redshifts, correlationResult->Correlation, extremumList );

    // Refine Extremum with a second maximum search around the z candidates:
    // This corresponds to the finer xcorrelation in EZ Pandora (in standard_DP fctn in SolveKernel.py)
    Float64 radius = 0.001;
    for( Int32 i=0; i<extremumList.size(); i++ )
    {
        Float64 x = extremumList[i].X;
        Float64 left_border = max(redshiftsRange.GetBegin(), x-radius);
        Float64 right_border=min(redshiftsRange.GetEnd(), x+radius);

        TPointList extremumListFine;
        TFloat64Range rangeFine = TFloat64Range( left_border, right_border );
        CExtremum extremumFine( rangeFine , 1);
        extremumFine.Find( correlationResult->Redshifts, correlationResult->Correlation, extremumListFine );
        if( extremumListFine.size() ) {
            extremumList[i] = extremumListFine[0];
        }
    }

    if( extremumList.size() == 0 )
    {
        return false;
    }

    // Compute merit function
    TFloat64List extremumRedshifts( extremumList.size() );
    TFloat64List extremumCorrelation( extremumList.size() );
    for( Int32 i=0; i<extremumList.size(); i++ )
    {
        extremumRedshifts[i] = extremumList[i].X;
        extremumCorrelation[i] = extremumList[i].Y;
    }

    COperatorChiSquare meritChiSquare;
    auto chisquareResult = dynamic_pointer_cast<const CChisquareResult>( meritChiSquare.Compute( spc, tpl, lambdaRange, extremumRedshifts, overlapThreshold ) );
    if( !chisquareResult )
    {
        return false;
    }

    // Store results
    resultStore.StoreScopedPerTemplateResult( tpl, "merit", chisquareResult );

    return true;
}
