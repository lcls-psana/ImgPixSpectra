#ifndef IMGPIXSPECTRA_CSPADPIXSPECTRA_H
#define IMGPIXSPECTRA_CSPADPIXSPECTRA_H

//--------------------------------------------------------------------------
// File and Version Information:
// 	$Id$
//
// Description:
//	Class CSPadPixSpectra.
//
//------------------------------------------------------------------------

//-----------------
// C/C++ Headers --
//-----------------

//----------------------
// Base Class Headers --
//----------------------
#include "psana/Module.h"

//-------------------------------
// Collaborating Class Headers --
//-------------------------------
#include "psddl_psana/cspad.ddl.h"
//#include "ndarray/ndarray.h"

//------------------------------------
// Collaborating Class Declarations --
//------------------------------------

//		---------------------
// 		-- Class Interface --
//		---------------------

namespace ImgPixSpectra {

/// @addtogroup ImgPixSpectra

/**
 *  @ingroup ImgPixSpectra
 *
 *  @brief Creates the spectal array for all pixels in the CSPad detector.
 *
 *  CSPadPixSpectra class is a psana module which creates and fills 
 *  the spectral array for all pixels in the CSPad array. The spectral
 *  array has two dimensions, the total number of pixels and
 *  the number of amplitude bins requested in the list of configuration parameters.
 *
 *  An example of the configuration file (psana.cfg) for this module:
 *
 *    @code
 *    [psana]
 *    files         = /reg/d/psdm/CXI/cxi35711/xtc/e86-r0009-s00-c00.xtc
 *    modules       = ImgPixSpectra.CSPadPixSpectra
 *
 *    [ImgPixSpectra.CSPadPixSpectra]
 *    source        = CxiDs1.0:Cspad.0
 *    amin          =    10.
 *    amax          =  2010.
 *    nbins         =   100
 *    arr_fname     = cspad-pix-spectra.txt
 *    #events       =   500                                                  
 *    @endcode
 *
 *  The output file "cspad-pix-spectra.txt" contains the spectral array 
 *  for CSPad pixels accumulated in job. 
 *  Axillary file with additional name extension ".sha" contains the shape parameters
 *  of the spectral array. The file(s) can be used for further analysis 
 *  or presentation, for example, using the python script:
 *
 *    @code
 *    ImgPixSpectra/data/PlotSpectralArrayFromFile.py cspad-pix-spectra.txt
 *    @endcode
 *
 *  This software was developed for the LCLS project. If you use all or 
 *  part of it, please give an appropriate acknowledgment.
 *
 *  @see AdditionalClass
 *
 *  @version \$Id: CSPadPixSpectra.h$
 *
 *  @author Mikhail S. Dubrovin
 */

class CSPadPixSpectra : public Module {
public:

  typedef Psana::CsPad::DataV2    CSPadDataType;
  typedef Psana::CsPad::ElementV2 CSPadElementType;
  typedef Psana::CsPad::ConfigV3  CSPadConfigType;

  // Default constructor
  CSPadPixSpectra (const std::string& name) ;

  // Destructor
  virtual ~CSPadPixSpectra () ;

  /// Method which is called once at the beginning of the job
  virtual void beginJob(Event& evt, Env& env);
  
  /// Method which is called at the beginning of the run
  virtual void beginRun(Event& evt, Env& env);
  
  /// Method which is called at the beginning of the calibration cycle
  virtual void beginCalibCycle(Event& evt, Env& env);
  
  /// Method which is called with event data, this is the only required 
  /// method, all other methods are optional
  virtual void event(Event& evt, Env& env);
  
  /// Method which is called at the end of the calibration cycle
  virtual void endCalibCycle(Event& evt, Env& env);

  /// Method which is called at the end of the run
  virtual void endRun(Event& evt, Env& env);

  /// Method which is called once at the end of the job
  virtual void endJob(Event& evt, Env& env);

protected:

  void getQuadConfigPars(Env& env);
  void printQuadConfigPars();
  void printInputPars();
  void arrayInit();
  void arrayDelete();
  void loopOverQuads(shared_ptr<CSPadDataType> data);
  void arrayFill(int quad, const int16_t* data, uint32_t roiMask);
  void saveArrayInFile();
  void saveShapeInFile();
  int  ampToIndex(double amp);

private:

  // Data members, this is for example purposes only
  
  Source   m_src;         // Data source set from config file
  Pds::Src m_actualSrc;
  std::string m_key;
  unsigned m_maxEvents;
  double   m_amin;
  double   m_amax;
  int      m_nbins;
  std::string   m_arr_fname;
  std::string   m_arr_shape_fname;
  bool     m_filter;
  long     m_count;
  
  int      m_nbins1;
  double   m_factor;

  uint32_t m_roiMask        [4];
  uint32_t m_numAsicsStored [4];

  uint32_t m_nquads;         // 4
  uint32_t m_n2x1;           // 8
  uint32_t m_ncols2x1;       // 185
  uint32_t m_nrows2x1;       // 388
  uint32_t m_sizeOf2x1Arr;   // 185*388;
  uint32_t m_sizeOfQuadArr;  // 185*388*8;
  uint32_t m_sizeOfCSPadArr; // 185*388*8*4;
  uint32_t m_pixel_ind;

  //std::vector<int> m_image_shape;

  int*           m_arr;
  //ndarray<int,2> m_arr2d;


//--------------------
  /**
   * @brief Gets m_numQuadsInConfig, m_roiMask[q] and m_numAsicsStored[q] from the Psana::CsPad::ConfigV# object.
   * 
   */

template <typename T>
bool getQuadConfigParsForType(Env& env)
{
  shared_ptr<T> config = env.configStore().get(m_src);
  if (config.get()) {
      for (uint32_t q = 0; q < 4; ++ q) {
        m_roiMask[q]         = config->roiMask(q);
        m_numAsicsStored[q]  = config->numAsicsStored(q);
      }

      m_nquads         = config->numQuads();               // this may be less than 4
      m_n2x1           = Psana::CsPad::SectorsPerQuad;     // v_image_shape[0]; // 8
      m_ncols2x1       = Psana::CsPad::ColumnsPerASIC;     // v_image_shape[1]; // 185
      m_nrows2x1       = Psana::CsPad::MaxRowsPerASIC * 2; // v_image_shape[2]; // 388
      m_sizeOf2x1Arr   = m_nrows2x1      * m_ncols2x1;                          // 185*388;
      m_sizeOfQuadArr  = m_sizeOf2x1Arr  * m_n2x1;                              // 185*388*8;
      m_sizeOfCSPadArr = m_sizeOfQuadArr * 4;                                   // 185*388*8*4;

      return true; 
  }
  return false;
}

//--------------------

};

} // namespace ImgPixSpectra

#endif // IMGPIXSPECTRA_CSPADPIXSPECTRA_H
