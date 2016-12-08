#include <iostream>
#include <iomanip>

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

#include <fstream>

#include <vector>

int main (int argc, char *argv[])
{
  if (argc < 4)
    {
      std::cout << "Usage: " << argv[0] 
		<< " covering_transform histogram_csv number_of_bins" << std::endl;

      std::cout << std::endl;
      std::cout << "Covering transform file is assumed to have the following format:" << std::endl;
      std::cout << "  * Voxel value >= 0:  This is the maximum radius of the ball that will cover this voxel in the pore / solid phase." << std::endl;
      std::cout << "  * Voxel value < 0:  This is the phase that is ignored." << std::endl;

      return 1;
    }

  //  typedef itk::Image<unsigned char, 3> phase_image_type;
  typedef itk::Image<float, 3> dt_image_type;

  try {

    std::cout << "##############################################" << std::endl;
    std::cout << "##       CRT HISTOGRAM                      ##" << std::endl;
    std::cout << "##############################################" << std::endl;

    std::cout << "#  Reading distance transform file " << argv[1] << " ... ";
    std::cout.flush();
    
    itk::ImageFileReader<dt_image_type>::Pointer dtReader 
      = itk::ImageFileReader<dt_image_type>::New();
    dtReader->SetFileName(argv[1]);
    dtReader->UpdateLargestPossibleRegion();
    std::cout << "done! " << std::endl;
    
    // Image iterators
    itk::ImageRegionIterator<dt_image_type>
      dt_it(dtReader->GetOutput(), dtReader->GetOutput()->GetLargestPossibleRegion());

    // Loop through the images and record covering radius values for the phase.
    // Also record the max dt value (min is assumed zero)
    std::vector<float> porePhase;

    float max = 0.0f;

    int numbins = atoi(argv[3]);
    std::cout << "#  Computing histogram with " << numbins << " bins ...";
    std::cout.flush();
    
    while (! dt_it.IsAtEnd())
      {
        float d = dt_it.Get();
        if (d >= 0)
          { 
            // Update max value
            if (d > max)
              {
                max = d;
              }
            
            porePhase.push_back(d);
          }
        ++dt_it;
      }
    
    // Build histograms

    // First compute bin ranges
  

    std::vector<float> binsMin(numbins);
    std::vector<float> binsMax(numbins);

    // Bin ranges are multiples of max / numbins

    float sz = max / (float)numbins;

    binsMin[0] = 0.0f;
    binsMax[0] = sz;
    
    for (unsigned int i = 1; i < numbins; i++)
      {
        binsMin[i] = binsMax[i-1];
        binsMax[i] = binsMin[i]+sz; 
      }
    
    // Catch rounding error and highest value in last bin
    binsMax[numbins] = max + 0.00001f;

    // Compute histograms

    // Initialize bins
    std::vector<unsigned long> poreHisto(numbins);
    for (unsigned int i = 0; i < numbins; i++)
      {
        poreHisto[i] = 0;
      }


    // Sort into bins and compute histogram
    unsigned long total = 0;
    for (unsigned long i = 0; i < porePhase.size(); i++)
      {
        unsigned int bin = static_cast<unsigned int>( floor( porePhase[i] / sz ));
        poreHisto[bin] = poreHisto[bin] + 1; 
      }

    // Compute as percentages
    std::vector<float> porePercent;
    for (unsigned int i = 0; i < poreHisto.size(); i++)
      {
        float p = static_cast<float>(poreHisto[i]) /  static_cast<float>(porePhase.size());
        porePercent.push_back(p);
        
      }

    
    std::cout << " done!" << std::endl << std::endl;

   
    
    
    
    // Print out histograms
    for (unsigned int i = 0; i < numbins; i++)
      {
        std::cout << std::fixed;
        std::cout << std::setprecision(3) << std::setw(8) << binsMin[i] << " - " << binsMax[i] << ":\t"
                  << poreHisto[i] << "\t" << porePercent[i] << std::endl;
      }
    std::cout << std::endl;
    
    
    // Write values of histogram to a csv file

    std::cout << "#  Writing histogram to file " << argv[2] << " ... ";
    std::cout.flush();
    std::ofstream ofs(argv[2], std::ofstream::out);

    if (! ofs.is_open())
      {
        throw (itk::ExceptionObject("Couldn't open csv file for output"));
      }
    
    ofs << "Bin Number,Bin Range,Count,Percent"<< std::endl;
    for (unsigned int i = 0; i < numbins; i++)
      {
        ofs << i << "," << binsMin[i] << " - " << binsMax[i] << ","
            << poreHisto[i] << "," << porePercent[i] << std::endl;
      }

    ofs.close();
    std::cout << "done!" << std::endl;

  }
  catch (itk::ExceptionObject &e)
    {
      std::cerr << e << std::endl;
      return 1;
    }
  catch( ...)
    {
      std::cerr << "Unknown error" << std::endl;
      return 2;
    }

  return 0;
}


