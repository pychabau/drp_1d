#ifndef _REDSHIFT_SPECTRUM_IO_READER_
#define _REDSHIFT_SPECTRUM_IO_READER_

#include <RedshiftLibrary/common/datatypes.h>

#include <vector>

namespace NSEpic
{

class CSpectrum;

class CSpectrumIOReader
{

public:

    CSpectrumIOReader();
    virtual ~CSpectrumIOReader();

    virtual Bool Read( const char* fluxPath, std::shared_ptr<CSpectrum> s ) = 0;

private:

};


}

#endif
