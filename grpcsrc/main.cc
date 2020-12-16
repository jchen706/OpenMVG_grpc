#include "openMVG/features/akaze/image_describer_akaze_io.hpp"

#include "openMVG/features/sift/SIFT_Anatomy_Image_Describer_io.hpp"
#include "openMVG/image/image_io.hpp"
#include "openMVG/features/regions_factory_io.hpp"
#include "openMVG/sfm/sfm_data.hpp"
#include "openMVG/sfm/sfm_data_io.hpp"
#include "openMVG/system/timer.hpp"
#include "openMVG/exif/exif_IO_EasyExif.hpp"
#include "gps_distance.h"

using namespace std;
using namespace openMVG;
using namespace openMVG::image;
using namespace openMVG::features;
using namespace openMVG::sfm;
using namespace openMVG::exif;


#include <atomic>
#include <cstdlib>
#include <fstream>
#include <string>

int main(int argc, char** argv) {

    // extract exif information

    // calculate distance 

    std::string path = "/home/j/openMVG/src/grpcsrc/DJI_0035.png";

    std::string path1 = "/home/j/openMVG/src/grpcsrc/DJI_0034.png";

    std::unique_ptr<Exif_IO> exif_io ( new Exif_IO_EasyExif( path ) );
    double val;
   
    exif_io->GPSLatitude(&val);
    std::cout << val << std::endl;
    double lat1 = val; 

    exif_io->GPSLongitude(&val);
    std::cout << val << std::endl;
    double long1 = val; 

    exif_io->GPSAltitude(&val);
    std::cout << val << std::endl;

    std::unique_ptr<Exif_IO> exif_io1 ( new Exif_IO_EasyExif( path1 ) );
    //double val;
    exif_io1->GPSLatitude(&val);
    std::cout << val << std::endl;
    double lat2 = val; 

	
    exif_io1->GPSLongitude(&val);
    std::cout << val << std::endl;
    double long2 = val; 
    

    exif_io1->GPSAltitude(&val);
    std::cout << val << std::endl;

	
	// call the distance function 
	cout << setprecision(15) << fixed; 
	cout << distance(lat1, long1, 
					lat2, long2)  << " meters" <<endl; 

 


	return 1;
}