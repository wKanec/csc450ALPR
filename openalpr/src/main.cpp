/*
 * Copyright (c) 2015 OpenALPR Technology, Inc.
 * Open source Automated License Plate Recognition [http://www.openalpr.com]
 *
 * This file is part of OpenALPR.
 *
 * OpenALPR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License
 * version 3 as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

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
#include "alpr_impl.h"



using namespace alpr;

const std::string MAIN_WINDOW_NAME = "ALPR main window";

const bool SAVE_LAST_VIDEO_STILL = false;
const std::string LAST_VIDEO_STILL_LOCATION = "/tmp/laststill.jpg";
const std::string WEBCAM_PREFIX = "/dev/video";
MotionDetector motiondetector;
bool do_motiondetection = true;

/** Function Headers */
//split headers
std::vector<AlprRegionOfInterest> getROI(Alpr* alpr, cv::Mat frame, std::string region, bool writeJson);
SplitReturn split1 ( Alpr* alpr, cv::Mat frame, std::string region, bool writeJson, AlprImpl* impl);
SplitReturn2 split2 (SplitReturn split1return, AlprImpl* impl);
SplitReturn3 split3 (SplitReturn2 split2return, AlprImpl* impl);
AlprFullDetails split4 (SplitReturn2 split2return, SplitReturn3 split3return, AlprImpl* impl);
AlprResults split6 (AlprFullDetails details, AlprImpl* impl, SplitReturn split1return);


bool detectandshow( AlprResults results);
bool is_supported_image(std::string image_file);

bool measureProcessingTime = false;
std::string templatePattern;

// This boolean is set to false when the user hits terminates (e.g., CTRL+C )
// so we can end infinite loops for things like video processing.
bool program_active = true;


int main( int argc, const char** argv )
{
  std::vector<std::string> filenames;
  std::string configFile = "";
  bool outputJson = false;
  int seektoms = 0;
  bool detectRegion = false;
  std::string country;
  int topn;
  bool debug_mode = false;
  std::cout << "Main 1" <<std::endl;
  

  TCLAP::CmdLine cmd("OpenAlpr Command Line Utility", ' ', Alpr::getVersion());

  TCLAP::UnlabeledMultiArg<std::string>  fileArg( "image_file", "Image containing license plates", true, "", "image_file_path"  );

  
  TCLAP::ValueArg<std::string> countryCodeArg("c","country","Country code to identify (either us for USA or eu for Europe).  Default=us",false, "us" ,"country_code");
  TCLAP::ValueArg<int> seekToMsArg("","seek","Seek to the specified millisecond in a video file. Default=0",false, 0 ,"integer_ms");
  TCLAP::ValueArg<std::string> configFileArg("","config","Path to the openalpr.conf file",false, "" ,"config_file");
  TCLAP::ValueArg<std::string> templatePatternArg("p","pattern","Attempt to match the plate number against a plate pattern (e.g., md for Maryland, ca for California)",false, "" ,"pattern code");
  TCLAP::ValueArg<int> topNArg("n","topn","Max number of possible plate numbers to return.  Default=10",false, 10 ,"topN");

  TCLAP::SwitchArg jsonSwitch("j","json","Output recognition results in JSON format.  Default=off", cmd, false);
  TCLAP::SwitchArg debugSwitch("","debug","Enable debug output.  Default=off", cmd, false);
  TCLAP::SwitchArg detectRegionSwitch("d","detect_region","Attempt to detect the region of the plate image.  [Experimental]  Default=off", cmd, false);
  TCLAP::SwitchArg clockSwitch("","clock","Measure/print the total time to process image and all plates.  Default=off", cmd, false);
  TCLAP::SwitchArg motiondetect("", "motion", "Use motion detection on video file or stream.  Default=off", cmd, false);
  try
  {
    cmd.add( templatePatternArg );
    cmd.add( seekToMsArg );
    cmd.add( topNArg );
    cmd.add( configFileArg );
    cmd.add( fileArg );
    cmd.add( countryCodeArg );

    
    if (cmd.parse( argc, argv ) == false)
    {
      // Error occurred while parsing.  Exit now.
      return 1;
    }

    filenames = fileArg.getValue();

    country = countryCodeArg.getValue();
    seektoms = seekToMsArg.getValue();
    outputJson = jsonSwitch.getValue();
    debug_mode = debugSwitch.getValue();
    configFile = configFileArg.getValue();
    detectRegion = detectRegionSwitch.getValue();
    templatePattern = templatePatternArg.getValue();
    topn = topNArg.getValue();
    measureProcessingTime = clockSwitch.getValue();
	do_motiondetection = motiondetect.getValue();
  }
  catch (TCLAP::ArgException &e)    // catch any exceptions
  {
    std::cerr << "error: " << e.error() << " for arg " << e.argId() << std::endl;
    return 1;
  }

  std::cout << "Main 2" <<std::endl;
  cv::Mat frame;

  Alpr alpr(country, configFile);
  AlprImpl* impl = new AlprImpl(country, configFile);
  alpr.setTopN(topn);
  
  if (debug_mode)
  {
    alpr.getConfig()->setDebug(true);
  }

  if (detectRegion)
    alpr.setDetectRegion(detectRegion);

  if (templatePattern.empty() == false)
    alpr.setDefaultRegion(templatePattern);

  if (alpr.isLoaded() == false)
  {
    std::cerr << "Error loading OpenALPR" << std::endl;
    return 1;
  }

  for (unsigned int i = 0; i < filenames.size(); i++)
  {
    std::string filename = filenames[i];

    if (is_supported_image(filename))
    {
      if (fileExists(filename.c_str()))
      {
        frame = cv::imread(filename);

	//Add code for seperation of functions
	
		std::cout << "==========================FIRST SPLIT===============================" <<std::endl;
		std::cout << "Find regions of interest and edit image." <<std::endl;
		std::cout << "=====================================================================" <<std::endl;

		SplitReturn split1return = split1(&alpr, frame, "", outputJson,  impl);
		std::cout<<"==============================SPLIT 2================================"<<std::endl;
		std::cout<<"Locate possible plates in Regions of Interst and load country info"<<std::endl;
		std::cout<<"====================================================================="<<std::endl;
		SplitReturn2 split2return = split2(split1return,impl);
		
		std::cout << "==================================SPLIT 3===================================" <<std::endl;
		SplitReturn3 split3return = split3(split2return,impl);
		
		std::cout << "==================================SPLIT 4===================================" <<std::endl;
		AlprFullDetails details = split4(split2return, split3return,impl);
		
		std::cout << "==========================split 6 Start===========================================" << std::endl;
	    std::cout << "=======================Input response===================" << std::endl;
	
		AlprResults results = split6(details, impl, split1return);
		bool plate_found = detectandshow(results);
	    std::cout << "===============End of Splits===========" << std::endl;
	


		
		if (!plate_found && !outputJson)
          std::cout << "No license plates found." << std::endl;
      }
      else
      {
        std::cerr << "Image file not found: " << filename << std::endl;
      }
    }
	else
    {
      std::cerr << "Unknown file type" << std::endl;
      return 1;
    }
  }

  return 0;
}

SplitReturn split1 ( Alpr* alpr, cv::Mat frame, std::string region, bool writeJson,AlprImpl* impl){
	
	SplitReturn split1return;

	std::vector<AlprRegionOfInterest> regionsOfInterest = getROI(alpr, frame, region, writeJson);
	//first call to alpr_Impl
	if (regionsOfInterest.size()>0) split1return = impl->recognize(frame.data, frame.elemSize(), frame.cols, frame.rows, regionsOfInterest);

	return split1return;
}

SplitReturn2 split2 (SplitReturn split1return, AlprImpl* impl){
	SplitReturn2 split2return;
	
	split2return = impl->split2impl(split1return);
	return split2return;
}
SplitReturn3 split3 (SplitReturn2 split2return, AlprImpl* impl){
	SplitReturn3 split3return;
	//add code to call split3-5 a second time for a second plate
	split3return = impl->split3impl(split2return);
	return split3return;
}
AlprFullDetails split4 (SplitReturn2 split2return, SplitReturn3 split3return, AlprImpl* impl){
	AlprResults results;
	AlprFullDetails details;
	
	details = impl->split4impl(split3return, split2return);
	
	//results = details.results;
	return details;
}
//AlprFullDetails split4


AlprResults split6 (AlprFullDetails details, AlprImpl* impl, SplitReturn split1return){
	
	AlprResults results;
	
	details = impl->split6impl(details, split1return);
	results = details.results;
	return results;
}

bool is_supported_image(std::string image_file)
{
  return (hasEndingInsensitive(image_file, ".png") || hasEndingInsensitive(image_file, ".jpg") || 
	  hasEndingInsensitive(image_file, ".tif") || hasEndingInsensitive(image_file, ".bmp") ||  
	  hasEndingInsensitive(image_file, ".jpeg") || hasEndingInsensitive(image_file, ".gif"));
}


std::vector<AlprRegionOfInterest> getROI( Alpr* alpr, cv::Mat frame, std::string region, bool writeJson)
{

  std::vector<AlprRegionOfInterest> regionsOfInterest;
  if (do_motiondetection)
  {
	  std::cout << "Motion detected" <<std::endl;
	  cv::Rect rectan = motiondetector.MotionDetect(&frame);
	  if (rectan.width>0) regionsOfInterest.push_back(AlprRegionOfInterest(rectan.x, rectan.y, rectan.width, rectan.height));
  }
  else 
	  {
		std::cout<<"Get Region of Interest"<<std::endl;
		regionsOfInterest.push_back(AlprRegionOfInterest(0, 0, frame.cols, frame.rows));
	}
  
    return (regionsOfInterest);
}


//Second split jumps here 
bool detectandshow(AlprResults results)
{
  //AlprResults results;


  timespec startTime;
  getTimeMonotonic(&startTime);
  //split 2 in
  ///move this to split 2.....
  //if (regionsOfInterest.size()>0) results = alpr->recognize(frame.data, frame.elemSize(), frame.cols, frame.rows, regionsOfInterest);
  
  
  //split 8 out
  std::cout<<"Receive recognize response from 8"<<std::endl;
  timespec endTime;
  getTimeMonotonic(&endTime);
  double totalProcessingTime = diffclock(startTime, endTime);
  if (measureProcessingTime)
    std::cout << "Total Time to process image: " << totalProcessingTime << "ms." << std::endl;  
  
  //if (writeJson)
  //{
  //  std::cout << alpr->toJson( results ) << std::endl;
  //}
  //else
  //{
for (int i = 0; i < results.plates.size(); i++)
{
	std::cout << "plate" << i << ": " << results.plates[i].topNPlates.size() << " results";
	if (measureProcessingTime)
	std::cout << " -- Processing Time = " << results.plates[i].processing_time_ms << "ms.";
	std::cout << std::endl;

	if (results.plates[i].regionConfidence > 0)
	std::cout << "State ID: " << results.plates[i].region << " (" << results.plates[i].regionConfidence << "% confidence)" << std::endl;
      
	for (int k = 0; k < results.plates[i].topNPlates.size(); k++)
	{
	// Replace the multiline newline character with a dash
	std::string no_newline = results.plates[i].topNPlates[k].characters;
	std::replace(no_newline.begin(), no_newline.end(), '\n','-');
        
	std::cout << "    - " << no_newline << "\t confidence: " << results.plates[i].topNPlates[k].overall_confidence;
	if (templatePattern.size() > 0 || results.plates[i].regionConfidence > 0)
		std::cout << "\t pattern_match: " << results.plates[i].topNPlates[k].matches_template;
        
	std::cout << std::endl;
	}
}
  //}


  return results.plates.size() > 0;
}

