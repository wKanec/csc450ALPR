

#include <cstdio>
#include <sstream>
#include <iostream>
#include <iterator>
#include <algorithm>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "tclap/CmdLine.h"
#include "support/filesystem.h"
#include "support/timing.h"
#include "support/platform.h"
#include "video/videobuffer.h"
#include "motiondetector.h"
#include "alpr.h"
#include "NodeSplit.h"

using namespace alpr;

namespace alpr
{
	int Split1( filename)
	{
	std::cout<<filename<<std::endl;
	}
}
	/*start here
	//888888888888888888888888888888888888888888888888888888888888888888888888888888888
    if (is_supported_image(filename))
    {
	  std::cout << "___________________Main 8___________________________" <<std::endl;
	  std::cout << "Main 8 Start here...is supported image(frame)" <<std::endl;
	  std::cout << "==================================FIRST SPLIT===================================" <<std::endl;
	  std::cout << "==================================filename===================================" <<std::endl;
	  std::cout << filename <<std::endl;

      if (fileExists(filename.c_str()))
      {
        frame = cv::imread(filename);

	//Add code for seperation of functions	

        bool plate_found = detectandshow(&alpr, frame, "", outputJson);
	std::cout << "==================Input ===================" << std::endl;
	
        if (!plate_found && !outputJson)
          std::cout << "No license plates found." << std::endl;
      }
      else
      {
        std::cerr << "Image file not found: " << filename << std::endl;
      }
	std::cout << "________________Main 8 End_______________________" <<std::endl;

    }
	else
    {
      std::cerr << "Unknown file type" << std::endl;
	  std::cout << "Main 10" <<std::endl;
      return 1;
    }
  }

  return 0;
}

bool is_supported_image(std::string image_file)
{
	std::cout << "Main 11" <<std::endl;
  return (hasEndingInsensitive(image_file, ".png") || hasEndingInsensitive(image_file, ".jpg") || 
	  hasEndingInsensitive(image_file, ".tif") || hasEndingInsensitive(image_file, ".bmp") ||  
	  hasEndingInsensitive(image_file, ".jpeg") || hasEndingInsensitive(image_file, ".gif"));
}


bool detectandshow( Alpr* alpr, cv::Mat frame, std::string region, bool writeJson)
{
  std::cout << "______----________----________---_______----_____---" <<std::endl;
  std::cout << "Main 12...detect and show(frame, no region)" <<std::endl;

  timespec startTime;
  getTimeMonotonic(&startTime);

  std::vector<AlprRegionOfInterest> regionsOfInterest;
  if (do_motiondetection)
  {
	  std::cout << "Main 12.1" <<std::endl;
	  cv::Rect rectan = motiondetector.MotionDetect(&frame);
	  if (rectan.width>0) regionsOfInterest.push_back(AlprRegionOfInterest(rectan.x, rectan.y, rectan.width, rectan.height));
  }
  else 
	  {
		std::cout << "------------------------------------------------------" <<std::endl;
		std::cout<<"Main 12.2 region of interest.push_back(0,0,frame.cols,frame.rows)"<<std::endl;
		regionsOfInterest.push_back(AlprRegionOfInterest(0, 0, frame.cols, frame.rows));
		std::cout << "------------------main12.2------------------------------------" <<std::endl;
	}
  AlprResults results;
  
  std::cout << "==================================FIRST SPLIT Return=================================================" <<std::endl;
  std::cout << "regionsOfInterest" <<std::endl;  
  std::cout << "frame" <<std::endl;
  std::cout << "   " <<std::endl;
  std::cout << "   " <<std::endl;
  std::cout << "   " <<std::endl;

  
  std::cout << "==================================Second SPLIT===================================" <<std::endl;
  std::cout << "=====??CALL alpr->recognize(frame.data,frame.elemSize(),frame.cols,frame.rows,regionsOfInterest)=========" <<std::endl;

  if (regionsOfInterest.size()>0) results = alpr->recognize(frame.data, frame.elemSize(), frame.cols, frame.rows, regionsOfInterest);
  
  std::cout<<"Receive recognize response from 8"<<std::endl;
  timespec endTime;
  getTimeMonotonic(&endTime);
  double totalProcessingTime = diffclock(startTime, endTime);
  if (measureProcessingTime)
    std::cout << "Total Time to process image: " << totalProcessingTime << "ms." << std::endl;
	std::cout << "Main 15" <<std::endl;
  
  
  if (writeJson)
  {
    std::cout << alpr->toJson( results ) << std::endl;
	std::cout << "Main 16" <<std::endl;
  }
  else
  {
	std::cout << "Main 17" <<std::endl;
    for (int i = 0; i < results.plates.size(); i++)
    {
      std::cout << "plate" << i << ": " << results.plates[i].topNPlates.size() << " results";
      if (measureProcessingTime)
        std::cout << " -- Processing Time = " << results.plates[i].processing_time_ms << "ms.";
      std::cout << std::endl;
	  std::cout << "Main 18" <<std::endl;

      if (results.plates[i].regionConfidence > 0)
        std::cout << "State ID: " << results.plates[i].region << " (" << results.plates[i].regionConfidence << "% confidence)" << std::endl;
		std::cout << "Main 19" <<std::endl;
      
      for (int k = 0; k < results.plates[i].topNPlates.size(); k++)
      {
		std::cout << "Main 20" <<std::endl;
        // Replace the multiline newline character with a dash
        std::string no_newline = results.plates[i].topNPlates[k].characters;
        std::replace(no_newline.begin(), no_newline.end(), '\n','-');
        
        std::cout << "    - " << no_newline << "\t confidence: " << results.plates[i].topNPlates[k].overall_confidence;
        if (templatePattern.size() > 0 || results.plates[i].regionConfidence > 0)
          std::cout << "\t pattern_match: " << results.plates[i].topNPlates[k].matches_template;
        
        std::cout << std::endl;
      }
    }
  }


  std::cout << "______----________----________---_______----_____---" <<std::endl;
  return results.plates.size() > 0;
}
*/
