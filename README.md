# poresize
Pipeline for collagen pore-size calculation

## Algorithm
Reference is the paper Mickel, et al., Robust Pore Size Analysis of Filamentous Networks from Three-Dimensional Confocal Microscopy".  Biophysical Journal. V95 Dec 2008. 

Minimum resolved fiber spacing is 2 voxels.

1. DENOISING: Gaussian smoothing of image, followed by anisotropic smoothing.

2. BINARIZATION: Threshold image to binary.  All voxels with intensity greater than I_c set to 1 and all others to 0.  The 1's are the collagen and the 0's are the fluid.

3. ARTIFACT REMOVAL: Morphological Closing -- dilation followed by erosion (width 1).

4. EUCLIDEAN DISTANCE MAP OF THE FLUID PHASE

5. COVERING RADIUS TRANSFORM (CRT)

6. PORE SIZE DISTRIBUTION is the histogram of values of the CRT

