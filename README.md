# Cpp-parallelism-image-filter
This is a university project in which we tested the impact of parallelism in C++ with the OpenMP library.

The input of the program is a bmp image, which is performed a Gaussian diffusion, the sovel operator or a copy from one file to another.

There is a sequential version and a parallel version to be able to compare the results.

To automate the running of the program we use the Makefile, so that it gets the results that would be an average of 100 executions, when introducing one of the two options: "medias_seq" or "medias_par"