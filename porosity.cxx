#include <iostream>
#include <iomanip>

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>

#include <itkConstNeighborhoodIterator.h>
#include <itkZeroFluxNeumannBoundaryCondition.h>

#include <fstream>

#include <vector>

int main (int argc, char *argv[])
{

  // Constants for coding output image
  const float WRONGPHASE = -1.0;


  if (argc < 4)
    {
      std::cout << "Usage:" << argv[0] 
      	<< " phase_model phase output_file neighborhood_radius" << std::endl;

      std::cout << "REQUIRED PARAMETERS" << std::endl;
      std::cout << "phase_model = The image file containing the phase model, where 0 represents fluid phase and 1 represents the collagen phase." << std::endl;
      std::cout << "phase = 0 or 1.  This is the label of the pore phase." << std::endl;
      std::cout << "output_file = This is the name of the output local porosity file." << std::endl;
      std::cout << "neighborhood_size = This is size of the neighborhood for which to compute local porosity.  Note that large neighborhood sizes will be very slow to compute.  Consider downsampling the image and using a correspondingly smaller neighborhood." << std::endl;

      return 1;
    }

  typedef itk::Image<unsigned char, 3> phase_image_type;
  typedef itk::Image<float, 3> output_image_type;

  try {

    std::cout << "############################################" << std::endl;
    std::cout << "##             Porosity                   ##" << std::endl;
    std::cout << "############################################" << std::endl;


    std::cout << "#  Reading phase model file \"" << argv[1] << "\" ..." ;
    std::cout.flush();    
    itk::ImageFileReader<phase_image_type>::Pointer phaseReader 
      = itk::ImageFileReader<phase_image_type>::New();
    phaseReader->SetFileName(argv[1]);
    phaseReader->UpdateLargestPossibleRegion();
    phase_image_type::Pointer phaseImg = phaseReader->GetOutput();
    std::cout << " done!" << std::endl;
    

    itk::ImageRegion<3> region = phaseReader->GetOutput()->GetRequestedRegion();

    
    unsigned int phase = static_cast<unsigned int>(atoi(argv[2]));
    std::cout << "#  Computing porosity for phase label " << phase << std::endl;

    unsigned int nsize = static_cast<unsigned int>(atoi(argv[4]));
    std::cout << "#  Using neighborhood radius of " << nsize << std::endl;

    if (nsize < 1)
      {
        throw itk::ExceptionObject("Neighborhood size must be greater than zero");
      }
    
    // Output Image (the local porosity image)
    output_image_type::Pointer outImg = output_image_type::New();
    outImg->SetRegions(region);
    outImg->SetOrigin(phaseReader->GetOutput()->GetOrigin());
    outImg->SetSpacing(phaseReader->GetOutput()->GetSpacing());
    outImg->Allocate();
    
    // Image iterators
    itk::ConstNeighborhoodIterator<phase_image_type>::RadiusType r;
    r.Fill(nsize);
    itk::ConstNeighborhoodIterator<phase_image_type>
      phase_it(r,phaseImg,region);
    itk::ImageRegionIterator<output_image_type>
      out_it(outImg, region);
    
 // Set up progress reporting
    unsigned long total = region.GetSize()[0] * region.GetSize()[1] * region.GetSize()[2];
    unsigned long inc = 1000;
    unsigned long counter = total / inc;
    
    unsigned long c = 0;
    float perc = 0.0;
    float incr = 100.0 / static_cast<float>(inc);

    unsigned long globalCount = 0;

    std::cout << "#  Computing porosity ... " ;
    std::cout.flush();
    
    while (! phase_it.IsAtEnd())
      {
        // Progress reporting
        if ((++c) > counter)
          {
            if (perc > 0.0)
              {
                std::cout << "\b\b\b\b\b\b";
              }
            std::cout << std::fixed;
            perc += incr;
            std::cout << std::setprecision(1) << std::setw(4) << perc << "% ";
            std::cout.flush();
            c = 0;
          }

                
        if (phase_it.GetCenterPixel() == phase) globalCount++;

        unsigned long localCount = 0;

        for (unsigned i = 0; i < phase_it.Size(); i++)
          {
            if (phase_it.GetPixel(i) == phase)
              {
                localCount++;
              }
          }

        // Local porosity in neighborhood around this pixel
        out_it.Set( static_cast<float>(localCount) / static_cast<float>(phase_it.Size()) );


        ++out_it;
        ++phase_it;
      }
    std::cout << "done! " << std::endl;
    
    
    // Write porosity file
    std::cout << "#  Writing output to \"" << argv[3] << "\" ...";
    std::cout.flush();
    itk::ImageFileWriter<output_image_type>::Pointer dtWriter
      = itk::ImageFileWriter<output_image_type>::New();
    dtWriter->SetFileName(argv[3]);
    dtWriter->SetInput(outImg);
    dtWriter->UpdateLargestPossibleRegion();
    std::cout << " done!" << std::endl;
    
    // Global porosity
    
    float p = static_cast<float>(globalCount) / static_cast<float>(total);
    std::cout << "Global porosity value is " << std::setprecision(4) << p << std::endl;
    
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


