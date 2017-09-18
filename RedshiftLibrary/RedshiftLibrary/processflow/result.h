#ifndef _REDSHIFT_OPERATOR_RESULT_
#define _REDSHIFT_OPERATOR_RESULT_

#include <RedshiftLibrary/common/datatypes.h>

#include <vector>
#include <ostream>
#include <map>

namespace NSEpic
{

class CDataStore;

/**
 * \ingroup Redshift
 */
class COperatorResult
{

public:

    COperatorResult();
    virtual ~COperatorResult();

    virtual Void Save( const CDataStore& store, std::ostream& stream ) const = 0;
    virtual Void SaveLine( const CDataStore& store, std::ostream& stream ) const = 0;
    //virtual Void Load( std::istream& stream ) = 0;

    Void SetReliabilityLabel( std::string lbl );

protected:

    std::string m_ReliabilityLabel;
};

typedef std::vector< std::shared_ptr<COperatorResult> >           TOperatorResultList;
typedef std::map< std::string, std::shared_ptr< const COperatorResult> > TOperatorResultMap;

}

#endif
