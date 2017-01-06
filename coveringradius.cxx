#include <iostream>
#include <iomanip>

#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkImageDuplicator.h>

#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionConstIteratorWithIndex.h>

#include <fstream>

#include <vector>

int main (int argc, char *argv[])
{

  // Constants for coding output image
  const float WRONGPHASE = -1.0;


  if (argc < 4)
    {
      std::cout << "Usage:" << argv[0] 
      	<< " distance_transform phase_model phase output_file" << std::endl;

      std::cout << "REQUIRED PARAMETERS" << std::endl;
      std::cout << "distance_transform = The image file containing the distance transform" << std::endl;
      std::cout << "phase_model = The image file containing the phase model, where 0 represents fluid phase and 1 represents the collagen phase.  Image dimensions must match those of the distance transform." << std::endl;
      std::cout << "phase = 0 or 1.  This is the phase for which you want to compute the covering radius transform.  Voxels not in this phase are set to -1 on the output (all other values will be positive)." << std::endl;
      std::cout << "output_file = This is the name of the file to produce as output (the covering radius transform file." << std::endl;

      return 1;
    }

  typedef itk::Image<unsigned char, 3> phase_image_type;
  typedef itk::Image<float, 3> dt_image_type;

  try {

    std::cout << "##############################################" << std::endl;
    std::cout << "##             COVERING RADIUS              ##" << std::endl;
    std::cout << "##############################################" << std::endl;

    std::cout << "#  Reading input distance transform \"" << argv[1] << "\" ..." ;
    std::cout.flush();    
    itk::ImageFileReader<dt_image_type>::Pointer dtReader 
      = itk::ImageFileReader<dt_image_type>::New();
    dtReader->SetFileName(argv[1]);
    dtReader->UpdateLargestPossibleRegion();

    // Pointer to distance transform image
    dt_image_type::Pointer dtImg = dtReader->GetOutput();

    std::cout << " done!" << std::endl;

    float spX = dtImg->GetSpacing()[0];
    float spY = dtImg->GetSpacing()[1];
    float spZ = dtImg->GetSpacing()[2];

    std::cout << "#  Using pixel spacing " << spX << "mm X " << spY << "mm X "
              << spZ << "mm" << std::endl;

    if ((spX != spY) || (spY != spZ))
      {
        throw itk::ExceptionObject("This code requires isotropic voxels (spacing in each dimension should be equal");
      }

    std::cout << "#  Reading phase model file \"" << argv[2] << "\" ..." ;
    std::cout.flush();    
    itk::ImageFileReader<phase_image_type>::Pointer phaseReader 
      = itk::ImageFileReader<phase_image_type>::New();
    phaseReader->SetFileName(argv[2]);
    phaseReader->UpdateLargestPossibleRegion();
    phase_image_type::Pointer phaseImg = phaseReader->GetOutput();
    std::cout << " done!" << std::endl;
    
    // Check image sizes
    for (unsigned int i = 0; i < 3; i++)
      {
        if (dtReader->GetOutput()->GetLargestPossibleRegion().GetSize()[i]
            != phaseReader->GetOutput()->GetLargestPossibleRegion().GetSize()[i])
          {
            throw itk::ExceptionObject("Input images are not the same size");
          }
      }


    unsigned int phase = static_cast<unsigned int>(atoi(argv[3]));
    std::cout << "#  Computing transform for phase " << phase << std::endl;
    
    // Output Image (the Covering Radius Transform image)
    dt_image_type::Pointer outImg = dt_image_type::New();
    // outImg->SetRegions(dtImg->GetLargestPossibleRegion());
    // outImg->Allocate();    
    
    std::cout << "#  Copying input ... " ;
    std::cout.flush();

    itk::ImageDuplicator<dt_image_type>::Pointer dupFilter
      = itk::ImageDuplicator<dt_image_type>::New();
    dupFilter->SetInputImage(dtImg);
    dupFilter->Update();
    outImg = dupFilter->GetOutput();
    
    // // copy input to output
    // dt_it.GoToBegin();
    // out_it.GoToBegin();
    // while (!dt_it.IsAtEnd())
    //   {
      
    //   out_it.Set(dt_it.Get());
     
    //   ++dt_it;
    //   ++out_it;        
    //   }   
    std::cout << "done! " << std::endl;
    
    std::cout << "#  Computing covering radius transform ... " ;
    std::cout.flush();


    // Image iterators
    itk::ImageRegionConstIteratorWithIndex<dt_image_type>
      dt_it(dtImg, dtImg->GetLargestPossibleRegion());
    itk::ImageRegionIterator<dt_image_type>
      out_it(outImg, outImg->GetLargestPossibleRegion());
    itk::ImageRegionConstIterator<phase_image_type>
      phase_it(phaseImg, phaseImg->GetLargestPossibleRegion());


    
    // ALGORITHM
    // For every pixel (x,y,z) in dt
    //   d = dt->GetPixel(x,y,z)
    //     For every pixel (x1,y1,z1) in neighborhood of radius d
    //        if sqrt( (x-x1)^2 + (y-y1)^2 + (z-z1)^2 ) < d
    //           if dt->GetPixel(x1,y1,z1) < d
    //                dt->SetPixel(x1,y1,z1,d)                       


    itk::Index<3> idx;
    dt_it.GoToBegin();
    out_it.GoToBegin();
    phase_it.GoToBegin();

    itk::ImageRegion<3> region = dtReader->GetOutput()->GetRequestedRegion();

    // Set up progress reporting
    unsigned long total = region.GetSize()[0] * region.GetSize()[1] * region.GetSize()[2];
    unsigned long inc = 1000;
    unsigned long counter = total / inc;
    
    unsigned long c = 0;
    float perc = 0.0;
    float incr = 100.0 / static_cast<float>(inc);
    
    while (! dt_it.IsAtEnd())
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

        // Covering radius transform
        
        // Loop through the neighborhood, but exclude voxels not within the radius d from the center
        // (only look in the sphere of radius d). The center voxel of the neighborhood is value d.
        // If a neighborhood pixel has a value less d, then set that pixel's value in the output image
        // to d. Otherwise, do nothing.

        if (phase_it.Get() != phase)
          {
            out_it.Set(WRONGPHASE);
          }
        else
          {
            idx = dt_it.GetIndex();

            float d = fabs( dt_it.Get() ); // The center voxel value

            //            if (fabs(out_it.Set(d); gets the distance value
            
            // Neighborhood size s is distance d divided by pixel spacing.  Since we require
            // isotropic voxels, we just use spacing in X dimension as our spacing.
            unsigned long s = static_cast<unsigned long>(ceil(d / spX));
            //        std::cout << d << "->" << s << std::endl;
            
            // Start and ending offsets from current center.
            itk::Index<3> startOffset;
            startOffset[0] = idx[0] - s;
            startOffset[1] = idx[1] - s;
            startOffset[2] = idx[2] - s;
            
            itk::Index<3> endOffset;
            endOffset[0] = idx[0] + s;
            endOffset[1] = idx[1] + s;
            endOffset[2] = idx[2] + s;
            
            itk::Index<3> off;
            
            for (off[0] = startOffset[0]; off[0] < endOffset[0]; off[0]++)
              {
                float x = static_cast<float>(off[0] - idx[0]);
                float x2 = x*x;
                
                for (off[1] = startOffset[1]; off[1] < endOffset[1]; off[1]++)
                  {
                    float y = static_cast<float>(off[1] - idx[1]);
                    float y2 = y*y;
                    
                    for (off[2] = startOffset[2]; off[2] < endOffset[2]; off[2]++)
                      {
                        if (region.IsInside(off))
                          {
                            float z = static_cast<float>(off[2] - idx[2]);
                            
                            float dist = sqrt(x2+y2+z*z);
                            
                            // Ignore neighbors not in the sphere of radius d or with
                            // voxel values greater than V
                            if ( (dist <= d) && (fabs(dtImg->GetPixel(off)) <= d))
                              {
                                outImg->SetPixel(off,d);
                                // std::cout << "  " << d << "," << dist << ": " << x << ","
                                //           << y << "," << z << std::endl;
                              }
                          } // end if regions
                      }
                  }
              } // end for loops
          } // end else
        
        ++dt_it;
        ++out_it;
        ++phase_it;
      }
    std::cout << "done! " << std::endl;
    
    
    // Write transform file
    std::cout << "#  Writing output to \"" << argv[4] << "\" ...";
    std::cout.flush();
    itk::ImageFileWriter<dt_image_type>::Pointer dtWriter
      = itk::ImageFileWriter<dt_image_type>::New();
    dtWriter->SetFileName(argv[4]);
    dtWriter->SetInput(outImg);
    dtWriter->UpdateLargestPossibleRegion();
    std::cout << " done!" << std::endl;        
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


