#include <RedshiftLibrary/operator/linemodelresult.h>

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <fstream>
#include <iomanip>      // std::setprecision

#include <string>

using namespace NSEpic;


/**
 * \brief Empty constructor.
 **/
CLineModelResult::CLineModelResult()
{

}

/**
 * \brief Empty destructor.
 **/
CLineModelResult::~CLineModelResult()
{

}

/**
 * @brief CLineModelResult::Init
 * Initializes the linemodel results, by:
 *  - setting the redshift grid
 *  - setting the size of chisquare vectors
 * @param redshifts
 * @param restRays
 * @param nTplshapes
 * @return
 */
Int32 CLineModelResult::Init( std::vector<Float64> redshifts, CRayCatalog::TRayVector restRays, Int32 nTplshapes )
{
    Int32 err = 0;

    Int32 nResults = redshifts.size();
    Status.resize( nResults );
    ChiSquare.resize( nResults );
    Redshifts.resize( nResults );
    Redshifts = redshifts;
    restRayList = restRays;
    LineModelSolutions.resize( nResults );

    //init the tplshape chisquare results
    for(Int32 k=0; k<nTplshapes; k++)
    {
        TFloat64List _chi2tpl(nResults, DBL_MAX);
        ChiSquareTplshapes.push_back(_chi2tpl);
    }

    return err;
}

Int32 CLineModelResult::SetChisquareTplshapeResult( Int32 index, TFloat64List chisquareTplshape )
{
    if(index>=Redshifts.size())
    {
        return -1;
    }
    if(chisquareTplshape.size()!=ChiSquareTplshapes.size())
    {
        return -2;
    }
    if(chisquareTplshape.size()<1)
    {
        return -3;
    }

    for(Int32 k=0; k<chisquareTplshape.size(); k++)
    {
        ChiSquareTplshapes[k][index] = chisquareTplshape[k];
    }
    return 0;
}

TFloat64List CLineModelResult::GetChisquareTplshapeResult( Int32 index )
{
    TFloat64List chisquareTplshape;
    if(index>=Redshifts.size())
    {
        return chisquareTplshape;
    }
    if(ChiSquareTplshapes.size()<1)
    {
        return chisquareTplshape;
    }

    for(Int32 k=0; k<ChiSquareTplshapes.size(); k++)
    {
        chisquareTplshape.push_back(ChiSquareTplshapes[k][index]);
    }

    return chisquareTplshape;
}

/**
 * \brief Attempts to read the redshift and chisquare values stored in the argument stream.
 * Clear the current lines lists.
 * For each line in input stream:
 *   Tokenize each line.
 *   If the first character is not "#":
 *     Try to read the redshift. If this fails, return.
 *     Ignore the next token - it should be a name (string).
 *     Try to read the chisquare. If this fails, return.
 **/
Void CLineModelResult::Load( std::istream& stream )
{
    // Clear current lines list
    Redshifts.clear();
    ChiSquare.clear();
    Status.clear();

    std::string line;

    // Read file line by line
    while( std::getline( stream, line ) )
    {
        boost::char_separator<char> sep(" \t");

        // Tokenize each line
        typedef boost::tokenizer< boost::char_separator<char> > ttokenizer;
        ttokenizer tok( line, sep );

        // Check if it's not a comment
        ttokenizer::iterator it = tok.begin();
        if( it != tok.end() && *it != "#" )
        {
            // Parse position
            Float64 r = 0.0;
            try
            {
                r = boost::lexical_cast<Float64>(*it);
            }
            catch (boost::bad_lexical_cast)
            {
                return;
            }

            // Parse name
            ++it;

	    // Parse c
            Float64 c = 0.0;
            if( it != tok.end() )
            {
                try
                {
                    c = boost::lexical_cast<Float64>(*it);
                }
                catch (boost::bad_lexical_cast)
                {
                    return;
                }
            }
            else
            {
                return;
            }

            Redshifts.push_back( r );
            ChiSquare.push_back( c );
            Status.push_back( COperator::nStatus_OK );
        }
    }
}

/**
 * \brief Prints the results currently in the argument store, in the argument stream.
 * Using the argument stream to print values from the argument store:
 * Print a header as a comment.
 * Print each redshift and chisquare values.
 * Print each Extrema as a comment.
 * Print each BIC as a comment.
 * Print each POSTERIOR as a comment.
 * Print each SigmaZ as a comment.
 * Print each LogArea as a comment.
 **/
Void CLineModelResult::Save( const CDataStore& store, std::ostream& stream ) const
{
    stream <<  "#Redshifts\tChiSquare"<< std::endl;
    for ( int i=0; i<Redshifts.size(); i++)
    {
        stream <<  Redshifts[i] << std::setprecision(32) << "\t" << std::scientific << ChiSquare[i] << std::fixed << std::endl;
    }

    ExtremaResult.Save(store, stream);

    // save dTransposeDNocontinuum, on 1 line
    if(true){
        stream <<  "#dTransposeDNocontinuum = {";
        stream <<  dTransposeDNocontinuum << "\t";
        stream << "}" << std::endl;
    }

    // save dTransposeD, on 1 line
    if(true){
        stream <<  "#dTransposeD = {";
        stream <<  dTransposeD << "\t";
        stream << "}" << std::endl;
    }

}

/**
 * \brief Prints the argument store's number of redshift results in the argument stream.
 **/
Void CLineModelResult::SaveLine( const CDataStore& store, std::ostream& stream ) const
{
    stream << "LineModelResult" << "\t" << Redshifts.size() << std::endl;
}

Int32 CLineModelResult::GetNLinesOverCutThreshold(Int32 extremaIdx, Float64 snrThres, Float64 fitThres) const
{
    if( ExtremaResult.Extrema.size()<=extremaIdx )
    {
        return 0;
    }
    Int32 nSol=0;
    Int32 solutionIdx=0;
    for ( UInt32 i2=0; i2<LineModelSolutions.size(); i2++)
    {
        if( Redshifts[i2]==ExtremaResult.Extrema[extremaIdx] )
        {
            solutionIdx = i2;
            break;
        }
    }
    std::vector<Int32> indexesSols;
    for ( UInt32 j=0; j<LineModelSolutions[solutionIdx].Amplitudes.size(); j++)
    {
        //skip if already sol
        bool alreadysol = false;
        for( Int32 i=0; i<indexesSols.size(); i++ )
        {
            if( LineModelSolutions[solutionIdx].ElementId[j]==indexesSols[i] )
            {
                alreadysol=true;
                break;
            }
        }
        if( alreadysol )
        {
            continue;
        }
        if( !LineModelSolutions[solutionIdx].Rays[j].GetIsStrong() )
        {
            continue;
        }
        if( !LineModelSolutions[solutionIdx].Rays[j].GetIsEmission() )
        {
            continue;
        }

        Float64 noise = LineModelSolutions[solutionIdx].Errors[j];
        if( noise>0 )
        {
            Float64 snr = LineModelSolutions[solutionIdx].Amplitudes[j]/noise;
            Float64 Fittingsnr = LineModelSolutions[solutionIdx].Amplitudes[j]/LineModelSolutions[solutionIdx].FittingError[j];
            if( snr>=snrThres && Fittingsnr>=fitThres )
            {
                nSol++;
                indexesSols.push_back(LineModelSolutions[solutionIdx].ElementId[j]);
            }
        }

    }
    return nSol;
}

/**
 * @brief CLineModelResult::GetStrongLinesPresence
 * @param filterType: 1: emission only, 2 abs only, else: no filter
 * @return: a list of boolean values indicating if a strong is present (not outsidelambdarange for that z) for each redshift
 */
std::vector<bool> CLineModelResult::GetStrongLinesPresence( UInt32 filterType ) const
{

    std::vector<bool> strongIsPresent(LineModelSolutions.size(), false);
    for ( UInt32 solutionIdx=0; solutionIdx<LineModelSolutions.size(); solutionIdx++)
    {
        strongIsPresent[solutionIdx] = false;

        for ( UInt32 j=0; j<LineModelSolutions[solutionIdx].Amplitudes.size(); j++)
        {
            if( !LineModelSolutions[solutionIdx].Rays[j].GetIsStrong() )
            {
                continue;
            }

            if(filterType==1)
            {
                if( !LineModelSolutions[solutionIdx].Rays[j].GetIsEmission() )
                {
                    continue;
                }
            }else if(filterType==2)
            {
                if( LineModelSolutions[solutionIdx].Rays[j].GetIsEmission() )
                {
                    continue;
                }
            }

            if(!LineModelSolutions[solutionIdx].OutsideLambdaRange[j])
            {
                strongIsPresent[solutionIdx] = true;
                break;
            }
        }
    }

    return strongIsPresent;
}




/**
 * \brief Returns the value of the ChiSquare of the Extrema indexed by the argument extremaIdx - if it is a valid index.
 * Let result be -1.
 * Return the result.
 **/
Float64 CLineModelResult::GetExtremaMerit( Int32 extremaIdx ) const
{
    Float64 outVal=-1.0;
    if( ExtremaResult.Extrema.size()>extremaIdx && ExtremaResult.ExtremaMerit.size()>extremaIdx )
    {
        outVal = ExtremaResult.ExtremaMerit[extremaIdx];
    }
    return outVal;
}

UInt32 CLineModelResult::GetExtremaIndex(UInt32 extremaIdx) const
{
    UInt32 solutionIdx=-1;
    if( ExtremaResult.Extrema.size()>extremaIdx && extremaIdx>=0 )
    {
        for ( UInt32 i2=0; i2<LineModelSolutions.size(); i2++)
        {
            if( Redshifts[i2]==ExtremaResult.Extrema[extremaIdx] )
            {
                solutionIdx = i2;
                break;
            }
        }
    }
    return solutionIdx;
}

Float64 CLineModelResult::GetMinChiSquare() const
{
    Float64 min=DBL_MAX;
    for ( int i=0; i<Redshifts.size(); i++)
    {
        if(min>ChiSquare[i])
        {
            min= ChiSquare[i];
        }
    }
    return min;
}

Float64 CLineModelResult::GetMaxChiSquare() const
{
    Float64 max=-DBL_MAX;
    for ( int i=0; i<Redshifts.size(); i++)
    {
        if(max<ChiSquare[i])
        {
            max= ChiSquare[i];
        }
    }
    return max;
}
