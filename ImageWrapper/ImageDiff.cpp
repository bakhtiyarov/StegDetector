#include "ImageDiff.hpp"


double BMPdiff(uint8_t* planes1[3], uint32_t width, uint32_t height, uint8_t* planes2[3])
{
	double maxP = 0;

#pragma omp parallel for schedule(static)
	for (int64_t i = 0; i < height; i++)
	{
		for (int64_t j = 0; j < width; j++)
		{
			double pij = std::sqrt(std::pow(planes1[0][i*width + j] - planes2[0][i*width + j], 2) +
				std::pow(planes1[1][i*width + j] - planes2[1][i*width + j], 2) +
				std::pow(planes1[2][i*width + j] - planes2[2][i*width + j], 2));
#pragma omp critical
			{
				if (pij > maxP)
					maxP = pij;
			}
		}
	}

	return maxP;
}