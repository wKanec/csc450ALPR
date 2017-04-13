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

#include "alpr_impl.h"
#include "result_aggregator.h"


void plateAnalysisThread(void* arg);

using namespace std;
using namespace cv;


namespace alpr
{
  SplitReturn::SplitReturn(cv::Mat passedimg, std::vector<cv::Rect> passedwarpedregion, 
  AlprFullDetails passedresponse, std::vector<cv::Rect>passedregion)
  {
	img = passedimg;
	region = passedregion;
	warpedRegion = passedwarpedregion;
	response = passedresponse;
	//country_aggregator = passedaggregator
  }
  SplitReturn::SplitReturn(){
	  
  }
  
  //Add a function to call split 1
  /*SplitReturn SplitReturn::split1( unsigned char* pixelData, int bytesPerPixel, int imgWidth, int imgHeight, std::vector<AlprRegionOfInterest> regionsOfInterest )
  {
	  
	SplitReturn split1return = AlprImpl.recognize(pixelData, bytesPerPixel, imgWidth, imgHeight, regionsOfInterest);
	return split1return;
  }*/
  AlprFullDetails SplitReturn::getResponse(){
	  return response;
  }

  cv::Mat SplitReturn::getImage(){
	return img;
  }
  std::vector<cv::Rect> SplitReturn::getWarpedRegion(){
	return warpedRegion;
  }
  
  std::vector<cv::Rect> SplitReturn::getRegion(){
	  return region;
  }
  
  //void SplitReturn::testsplit(){
  //	std::cout<<"Test Split"<<std::endl;
  //}

  AlprImpl::AlprImpl(const std::string country, const std::string configFile, const std::string runtimeDir)
  {
	  
    std::cout << "ALPR_IMPL 1" << std::endl;
    timespec startTime;
    getTimeMonotonic(&startTime);
    
    config = new Config(country, configFile, runtimeDir);

    prewarp = ALPR_NULL_PTR;
    
    // Config file or runtime dir not found.  Don't process any further.
    if (config->loaded == false)
    {
      return;
    }

    prewarp = new PreWarp(config);
    
    loadRecognizers();
    setNumThreads(0);

    setDetectRegion(DEFAULT_DETECT_REGION);
    this->topN = DEFAULT_TOPN;
    setDefaultRegion("");
    
    timespec endTime;
    getTimeMonotonic(&endTime);
    if (config->debugTiming)
      cout << "OpenALPR Initialization Time: " << diffclock(startTime, endTime) << "ms." << endl;
    
  }

  AlprImpl::~AlprImpl()
  {
	std::cout << "ALPR_IMPL 2" << std::endl;
    delete config;

    typedef std::map<std::string, AlprRecognizers>::iterator it_type;
    for(it_type iterator = recognizers.begin(); iterator != recognizers.end(); iterator++) {

      delete iterator->second.plateDetector;
      delete iterator->second.stateDetector;
      delete iterator->second.ocr;
    }

    delete prewarp;
  }

  bool AlprImpl::isLoaded()
  {
    return config->loaded;
  }


  SplitReturn AlprImpl::recognizeFullDetails(cv::Mat img, std::vector<cv::Rect> regionsOfInterest)
  {

    timespec startTime;
    getTimeMonotonic(&startTime);


    AlprFullDetails response;

    int64_t start_time = getEpochTimeMs();

    // Fix regions of interest in case they extend beyond the bounds of the image
    for (unsigned int i = 0; i < regionsOfInterest.size(); i++)
      regionsOfInterest[i] = expandRect(regionsOfInterest[i], 0, 0, img.cols, img.rows);

    for (unsigned int i = 0; i < regionsOfInterest.size(); i++)
    {
      response.results.regionsOfInterest.push_back(AlprRegionOfInterest(regionsOfInterest[i].x, regionsOfInterest[i].y,
              regionsOfInterest[i].width, regionsOfInterest[i].height));
    }

    /*if (!img.data)
    {
      // Invalid image
      if (this->config->debugGeneral)
        std::cerr << "Invalid image" << std::endl;

      return response;
    }*/

    // Convert image to grayscale if required
    Mat grayImg = img;
    if (img.channels() > 2){
		cvtColor( img, grayImg, CV_BGR2GRAY );
	}
    
    // Prewarp the image and ROIs if configured]
    std::vector<cv::Rect> warpedRegionsOfInterest = regionsOfInterest;
    // Warp the image if prewarp is provided
    grayImg = prewarp->warpImage(grayImg);
    warpedRegionsOfInterest = prewarp->projectRects(regionsOfInterest, grayImg.cols, grayImg.rows, false);

    // Iterate through each country provided (typically just one)
    // and aggregate the results if necessary
	//std::cout<<"	create ResultAggregator country_agregator(MERGE_PICK_BEST, topN,config)"<<std::endl;

	SplitReturn split1return(grayImg, warpedRegionsOfInterest, response, regionsOfInterest);
	
	timespec endTime;
    getTimeMonotonic(&endTime);
    if (config->debugTiming)
    {
      cout << "Total Time to process split1: " << diffclock(startTime, endTime) << "ms." << endl;
    }


	std::cout<<"============================Split 1 END================================"<<std::endl;
	std::cout<<"===== RETURN IMG AND WARPED REGION OF INTEREST =====" <<std::endl;
	std::cout << "   " <<std::endl;
	std::cout << "   " <<std::endl;
	std::cout << "   " <<std::endl;
	
	return split1return;
  }
  
  AlprFullDetails AlprImpl::split2impl(SplitReturn split1return)
  {
	std::cout<<"==============================SPLIT 2================================"<<std::endl;
	std::cout<<"Locate possible plates in Regions of Interst and load country info"<<std::endl;
	std::cout<<"====================================================================="<<std::endl;
	timespec startTime;
    getTimeMonotonic(&startTime);
	int64_t start_time = getEpochTimeMs();

	
	cv::Mat img = split1return.getImage();
	cv:: Mat grayImg = img;
	
	std::vector<cv::Rect> warpedRegionsOfInterest = split1return.getWarpedRegion();
	AlprFullDetails response = split1return.getResponse();
	std::vector<cv::Rect> regionsOfInterest = split1return.getRegion();

	
	ResultAggregator country_aggregator(MERGE_PICK_BEST, topN, config);
	for (unsigned int i = 0; i < config->loaded_countries.size(); i++)
    {
      if (config->debugGeneral)
        cout << "Analyzing: " << config->loaded_countries[i] << endl;

      config->setCountry(config->loaded_countries[i]);
      // Reapply analysis for each multiple analysis value set in the config,
      // make a minor imperceptible tweak to the input image each time
      ResultAggregator iter_aggregator(MERGE_COMBINE, topN, config);      
	  
	  for (unsigned int iteration = 0; iteration < config->analysis_count; iteration++)
      {
		std::cout << "config analysis_count" <<std::endl;
		std::cout << config->analysis_count <<std::endl;
        Mat iteration_image = iter_aggregator.applyImperceptibleChange(grayImg, iteration);
        //drawAndWait(iteration_image);
		//----------------------------------------------------------------
		//----------------------------------------------------------------
		//Everything is in the line below---------------------------------
		//----------------------------------------------------------------
		//---------------------------------------------------------------
		//---------------------------------------------------------------

        AlprFullDetails iter_results = analyzeSingleCountry(img, iteration_image, warpedRegionsOfInterest);
		//iter_results is the end of split 4 which returns a plate que
		//call 5-7 to receive results
		//Split 8 begins here.
		iter_aggregator.addResults(iter_results);
      }
		std::cout << "   " <<std::endl;
		std::cout << "   " <<std::endl;
		std::cout << "   " <<std::endl;
		std::cout << "==================receive response===========================================" << std::endl;
		std::cout << "==================iter_aggregator.addResults(response)===================" << std::endl;
	
      AlprFullDetails sub_results = iter_aggregator.getAggregateResults();
      sub_results.results.epoch_time = start_time;
      sub_results.results.img_width = img.cols;
      sub_results.results.img_height = img.rows;
      
      country_aggregator.addResults(sub_results);
    }
    response = country_aggregator.getAggregateResults();

    timespec endTime;
    getTimeMonotonic(&endTime);
    if (config->debugTiming)
    {
      cout << "Total Time to process image: " << diffclock(startTime, endTime) << "ms." << endl;
    }

    if (config->debugGeneral && config->debugShowImages)
    {
	  std::cout << "ALPR_IMPL 5 if debug and show image" << std::endl;
      for (unsigned int i = 0; i < regionsOfInterest.size(); i++)
      {
        rectangle(img, regionsOfInterest[i], Scalar(0,255,0), 2);
      }

      for (unsigned int i = 0; i < response.plateRegions.size(); i++)
      {
        rectangle(img, response.plateRegions[i].rect, Scalar(0, 0, 255), 2);
      }

      for (unsigned int i = 0; i < response.results.plates.size(); i++)
      {
		std::cout<<"		draw boxes around plate and charaters"<<std::endl;
        // Draw a box around the license plate 
        for (int z = 0; z < 4; z++)
        {
			std::cout << "		draw a box around the license plate" << std::endl;
          AlprCoordinate* coords = response.results.plates[i].plate_points;
          Point p1(coords[z].x, coords[z].y);
          Point p2(coords[(z + 1) % 4].x, coords[(z + 1) % 4].y);
          line(img, p1, p2, Scalar(255,0,255), 2);
        }
        
        // Draw the individual character boxes
        for (int q = 0; q < response.results.plates[i].bestPlate.character_details.size(); q++)
        {
          AlprChar details = response.results.plates[i].bestPlate.character_details[q];
          line(img, Point(details.corners[0].x, details.corners[0].y), Point(details.corners[1].x, details.corners[1].y), Scalar(0,255,0), 1);
          line(img, Point(details.corners[1].x, details.corners[1].y), Point(details.corners[2].x, details.corners[2].y), Scalar(0,255,0), 1);
          line(img, Point(details.corners[2].x, details.corners[2].y), Point(details.corners[3].x, details.corners[3].y), Scalar(0,255,0), 1);
          line(img, Point(details.corners[3].x, details.corners[3].y), Point(details.corners[0].x, details.corners[0].y), Scalar(0,255,0), 1);
        }
      }


      displayImage(config, "Main Image", img);

      // Sleep 1ms
      sleep_ms(1);

    }


    if (config->debugPauseOnFrame)
    {
      // Pause indefinitely until they press a key
      while ((char) cv::waitKey(50) == -1)
      {}
    }
		std::cout << "======================return edited response to =========================" << std::endl;

    return response;
  }
	//Everything is in the line below---------------------------------
	//----------------------------------------------------------------
	//---------------------------------------------------------------
	//---------------------------------------------------------------
  AlprFullDetails AlprImpl::analyzeSingleCountry(cv::Mat colorImg, cv::Mat grayImg, std::vector<cv::Rect> warpedRegionsOfInterest)
  {
	//std::cout << "ALPR_IMPL 6 analyzeSingleCountry (colorImg,grayImg,warpedRegionofInterest" << std::endl;
	//std::cout << "-----------Create a que of the plate regions----------------------" << std::endl;
	AlprFullDetails response;
    
    AlprRecognizers country_recognizers = recognizers[config->country];

    vector<PlateRegion> warpedPlateRegions;
    // Find all the candidate regions
    if (config->skipDetection == false)
    {
	  std::cout << "ALPR_IMPL 8 skipDetection == False" << std::endl;
      warpedPlateRegions = country_recognizers.plateDetector->detect(grayImg, warpedRegionsOfInterest);
    }
    else
    {
	  std::cout << "ALPR_IMPL 9 skipDetection == True" << std::endl;
      // They have elected to skip plate detection.  Instead, return a list of plate regions
      // based on their regions of interest
      for (unsigned int i = 0; i < warpedRegionsOfInterest.size(); i++)
      {
        PlateRegion pr;
        pr.rect = cv::Rect(warpedRegionsOfInterest[i]);
        warpedPlateRegions.push_back(pr);
      }
    }
    queue<PlateRegion> plateQueue;
    for (unsigned int i = 0; i < warpedPlateRegions.size(); i++)
      plateQueue.push(warpedPlateRegions[i]);
	  
	
	  
	std::cout << "==========================split 2 END==============================" << std::endl;
	std::cout << "===============Return PlateQueue of possible plates================" << std::endl;
	std::cout << "   " <<std::endl;
	std::cout << "   " <<std::endl;
	std::cout << "   " <<std::endl;
	AlprFullDetails results = AlprImpl::split3impl(colorImg, grayImg, plateQueue, country_recognizers, warpedPlateRegions);
	return results;
  }	
	
 AlprFullDetails AlprImpl::split3impl(cv::Mat colorImg,cv::Mat grayImg, std::queue<PlateRegion> plateQueue,
	AlprRecognizers country_recognizers, std::vector<PlateRegion> warpedPlateRegions)
  {
	  
	std::cout << "==========================split 3 Start=============================" << std::endl;
	std::cout << "=========================Input PlateQueue===========================" << std::endl;
	std::cout << "check for possible characters in plates" << std::endl;
	std::cout << "====================================================================" << std::endl;
 	AlprFullDetails response;

	timespec startTime;
    getTimeMonotonic(&startTime);
	
	int platecount = 0;
    while(!plateQueue.empty())
    {
      PlateRegion plateRegion = plateQueue.front();
      plateQueue.pop();
      PipelineData pipeline_data(colorImg, grayImg, plateRegion.rect, config);
      pipeline_data.prewarp = prewarp;
      timespec platestarttime;
      getTimeMonotonic(&platestarttime);
      LicensePlateCandidate lp(&pipeline_data);
	  //std::cout << "-----------------------split?--------------------------------------" << std::endl;
      lp.recognize();
	  //std::cout << "-----------------------split end?--------------------------------------" << std::endl;
      bool plateDetected = false;
      if (pipeline_data.disqualified && config->debugGeneral)
      {
        cout << "Disqualify reason: " << pipeline_data.disqualify_reason << endl;
      }
	  
	  //check if pipeline data in disqualified if it is then stop 
	  //if pipeline data not disqualified then add data here to 
	  //send pipeline_data to the zookeepers and back to below code.
	  std::cout << "=======================Split 3 END================================"<< std::endl;
	  std::cout << "=================Return pipeline_data==================" << std::endl;
	  std::cout << "   " <<std::endl;
	  std::cout << "   " <<std::endl;
	  std::cout << "   " <<std::endl;
	
	
	std::cout << "=======================split 4 Start============================" << std::endl;
	std::cout << "====================Input pipeline_data=========================" << std::endl;
	std::cout << "Run character analysis" << std::endl;
	std::cout << "================================================================" << std::endl;

	  
	  if (!pipeline_data.disqualified)
      {
        AlprPlateResult plateResult;
        
        plateResult.country = config->country;
        
        // If there's only one pattern for a country, use it.  Otherwise use the default
        if (country_recognizers.ocr->postProcessor.getPatterns().size() == 1)
          plateResult.region = country_recognizers.ocr->postProcessor.getPatterns()[0];
        else
          plateResult.region = defaultRegion;
        
        plateResult.regionConfidence = 0;
        plateResult.plate_index = platecount++;
        plateResult.requested_topn = topN;

        // If using prewarp, remap the plate corners to the original image
        vector<Point2f> cornerPoints = pipeline_data.plate_corners;
        cornerPoints = prewarp->projectPoints(cornerPoints, true);

        for (int pointidx = 0; pointidx < 4; pointidx++)
        {
          plateResult.plate_points[pointidx].x = (int) cornerPoints[pointidx].x;
          plateResult.plate_points[pointidx].y = (int) cornerPoints[pointidx].y;
        }

        
        #ifndef SKIP_STATE_DETECTION
		std::cout << "ALPR_IMPL 10.3" << std::endl;
        if (detectRegion && country_recognizers.stateDetector->isLoaded())
        {
          std::vector<StateCandidate> state_candidates = country_recognizers.stateDetector->detect(pipeline_data.color_deskewed.data,
                                                                               pipeline_data.color_deskewed.elemSize(),
                                                                               pipeline_data.color_deskewed.cols,
                                                                               pipeline_data.color_deskewed.rows);

          if (state_candidates.size() > 0)
          {
            plateResult.region = state_candidates[0].state_code;
            plateResult.regionConfidence = (int) state_candidates[0].confidence;
          }
        }
        #endif
        if (plateResult.region.length() > 0 && country_recognizers.ocr->postProcessor.regionIsValid(plateResult.region) == false)
        {
		  std::cout << "ALPR_IMPL 10.4" << std::endl;
          std::cerr << "Invalid pattern provided: " << plateResult.region << std::endl;
          std::cerr << "Valid patterns are located in the " << config->country << ".patterns file" << std::endl;
        }
        std::cout << "------------perform ocr and postProcessor.analyze---------------" << std::endl;
		country_recognizers.ocr->performOCR(&pipeline_data);
		std::cout << "-----------------------test--------------------------------------" << std::endl;
        country_recognizers.ocr->postProcessor.analyze(plateResult.region, topN);
        timespec resultsStartTime;
        getTimeMonotonic(&resultsStartTime);
        const vector<PPResult> ppResults = country_recognizers.ocr->postProcessor.getResults();

        std::cout << "		char transform matrix" << std::endl;
		std::cout << "		is best plate selected" << std::endl;
		std::cout << "===========================Split 4 END========================================" << std::endl;
		std::cout << "======================Return pipeline_data and ppResults===============================" << std::endl;
		std::cout << "   " <<std::endl;
		std::cout << "   " <<std::endl;
		std::cout << "   " <<std::endl;
	
	
		std::cout << "==========================split 5 Start===========================================" << std::endl;
		std::cout << "==================Input pipeline_data and ppResults===================" << std::endl;
	

        int bestPlateIndex = 0;

        cv::Mat charTransformMatrix = getCharacterTransformMatrix(&pipeline_data);
        bool isBestPlateSelected = false;
        for (unsigned int pp = 0; pp < ppResults.size(); pp++)
        {
		  //std::cout << "ALPR_IMPL 10.6" << std::endl;

          // Set our "best plate" match to either the first entry, or the first entry with a postprocessor template match
			std::cout << "		Set best plate match." << std::endl;

		  if (isBestPlateSelected == false && ppResults[pp].matchesTemplate){
            bestPlateIndex = plateResult.topNPlates.size();
            isBestPlateSelected = true;
          }

          AlprPlate aplate;
          aplate.characters = ppResults[pp].letters;
          aplate.overall_confidence = ppResults[pp].totalscore;
          aplate.matches_template = ppResults[pp].matchesTemplate;

          // Grab detailed results for each character
		  std::cout << "		get detailed results for each character" << std::endl;
          for (unsigned int c_idx = 0; c_idx < ppResults[pp].letter_details.size(); c_idx++)
          {
			//std::cout << "ALPR_IMPL 10.7" << std::endl;
            AlprChar character_details;
            Letter l = ppResults[pp].letter_details[c_idx];
            
            character_details.character = l.letter;
            character_details.confidence = l.totalscore;
            cv::Rect char_rect = pipeline_data.charRegionsFlat[l.charposition];
            std::vector<AlprCoordinate> charpoints = getCharacterPoints(char_rect, charTransformMatrix );
            for (int cpt = 0; cpt < 4; cpt++)
              character_details.corners[cpt] = charpoints[cpt];
            aplate.character_details.push_back(character_details);
          }
          plateResult.topNPlates.push_back(aplate);
        }
		std::cout << "ALPR_IMPL 10.8 Create BestPlate" << std::endl;
        if (plateResult.topNPlates.size() > bestPlateIndex)
        {
          AlprPlate bestPlate;
          bestPlate.characters = plateResult.topNPlates[bestPlateIndex].characters;
          bestPlate.matches_template = plateResult.topNPlates[bestPlateIndex].matches_template;
          bestPlate.overall_confidence = plateResult.topNPlates[bestPlateIndex].overall_confidence;
          bestPlate.character_details = plateResult.topNPlates[bestPlateIndex].character_details;

          plateResult.bestPlate = bestPlate;
        }

        timespec plateEndTime;
        getTimeMonotonic(&plateEndTime);
        plateResult.processing_time_ms = diffclock(platestarttime, plateEndTime);
        if (config->debugTiming)
        {
          cout << "Result Generation Time: " << diffclock(resultsStartTime, plateEndTime) << "ms." << endl;
        }

        if (plateResult.topNPlates.size() > 0)
        {
          plateDetected = true;
          response.results.plates.push_back(plateResult);
        }
      }

      if (!plateDetected)
      {
		std::cout << "ALPR_IMPL 10.9 not a valid plate. check for children" << std::endl;
        // Not a valid plate
        // Check if this plate has any children, if so, send them back up for processing
        for (unsigned int childidx = 0; childidx < plateRegion.children.size(); childidx++)
        {
          plateQueue.push(plateRegion.children[childidx]);
        }
      }

    }
	
	std::cout << "ALPR_IMPL 11" << std::endl;

    // Unwarp plate regions if necessary
    prewarp->projectPlateRegions(warpedPlateRegions, grayImg.cols, grayImg.rows, true);
    response.plateRegions = warpedPlateRegions;

    timespec endTime;
    getTimeMonotonic(&endTime);
    response.results.total_processing_time_ms = diffclock(startTime, endTime);
	std::cout << "===========================Split 5 END========================================" << std::endl;
	std::cout << "=========================Return response===============================" << std::endl;
	std::cout << "   " <<std::endl;
	std::cout << "   " <<std::endl;
	std::cout << "   " <<std::endl;

	std::cout << "==========================split 6 Start===========================================" << std::endl;
	std::cout << "=======================Input response===================" << std::endl;
	
    return response;
  }

  AlprResults AlprImpl::recognize( std::vector<char> imageBytes)
  {
	std::cout << "ALPR_IMPL 12" << std::endl;
    try
    {
      cv::Mat img = cv::imdecode(cv::Mat(imageBytes), 1);
      return this->recognize(img);
    }
    catch (cv::Exception& e)
    {
      std::cerr << "Caught exception in OpenALPR recognize: " << e.msg << std::endl;
      AlprResults emptyresults;
      return emptyresults;
    }
  }

  AlprResults AlprImpl::recognize(std::vector<char> imageBytes, std::vector<AlprRegionOfInterest> regionsOfInterest)
  {
	std::cout << "ALPR_IMPL 13" << std::endl;
    try
    {
      cv::Mat img = cv::imdecode(cv::Mat(imageBytes), 1);

      std::vector<cv::Rect> rois = convertRects(regionsOfInterest);
	  SplitReturn splitDetails = recognizeFullDetails(img,rois);
	  AlprFullDetails fullDetails = split2impl(splitDetails);
      //AlprFullDetails fullDetails = recognizeFullDetails(img, rois);
      return fullDetails.results;
    }
    catch (cv::Exception& e)
    {
      std::cerr << "Caught exception in OpenALPR recognize: " << e.msg << std::endl;
      AlprResults emptyresults;
      return emptyresults;
    }
  }

  SplitReturn AlprImpl::recognize( unsigned char* pixelData, int bytesPerPixel, int imgWidth, int imgHeight, std::vector<AlprRegionOfInterest> regionsOfInterest)
  {
	std::cout<<"reshape image"<<std::endl;
      int arraySize = imgWidth * imgHeight * bytesPerPixel;
      cv::Mat imgData = cv::Mat(arraySize, 1, CV_8U, pixelData);
      cv::Mat img = imgData.reshape(bytesPerPixel, imgHeight);

      if (regionsOfInterest.size() == 0)
      {
        AlprRegionOfInterest fullFrame(0,0, img.cols, img.rows);

        regionsOfInterest.push_back(fullFrame);
      }
	  SplitReturn splitresults = this->recognize(img, this->convertRects(regionsOfInterest));
		
	return splitresults;
  }

  AlprResults AlprImpl::recognize(cv::Mat img)
  {
	std::cout << "ALPR_IMPL 15 recognize(Mat img)" << std::endl;
    std::vector<cv::Rect> regionsOfInterest;
    regionsOfInterest.push_back(cv::Rect(0, 0, img.cols, img.rows));
	std::cout << "++++++++++++++++++++ALPR_IMPL 15 recognize END+++++++++++++++++++++" <<std::endl;
    SplitReturn splitresults = recognize(img, regionsOfInterest);
	AlprFullDetails fullDetails = split2impl(splitresults);
	return fullDetails.results;
  }
	
  SplitReturn AlprImpl::recognize(cv::Mat img, std::vector<cv::Rect> regionsOfInterest)
  {

    SplitReturn splitreturn = recognizeFullDetails(img, regionsOfInterest);
	return splitreturn;
	//AlprFullDetails fullDetails = split2impl(splitreturn);
	//return fullDetails.results;
  }


   std::vector<cv::Rect> AlprImpl::convertRects(std::vector<AlprRegionOfInterest> regionsOfInterest)
   {
	 std::cout<< "ALPR_IMPL 17 convertRect(RegionofInterest)" << std::endl;
     std::vector<cv::Rect> rectRegions;
     for (unsigned int i = 0; i < regionsOfInterest.size(); i++)
     {
       rectRegions.push_back(cv::Rect(regionsOfInterest[i].x, regionsOfInterest[i].y, regionsOfInterest[i].width, regionsOfInterest[i].height));
     }
	 std::cout << "ALPR_IMPL 17 convert Rect END" <<std::endl;
     return rectRegions;
   }

  string AlprImpl::toJson( const AlprResults results )
  {
	std::cout << "ALPR_IMPL 18" << std::endl;
    cJSON *root, *jsonResults;
    root = cJSON_CreateObject();


    cJSON_AddNumberToObject(root,"version",	2	  );
    cJSON_AddStringToObject(root,"data_type",	"alpr_results"	  );

    cJSON_AddNumberToObject(root,"epoch_time",	results.epoch_time	  );
    cJSON_AddNumberToObject(root,"img_width",	results.img_width	  );
    cJSON_AddNumberToObject(root,"img_height",	results.img_height	  );
    cJSON_AddNumberToObject(root,"processing_time_ms", results.total_processing_time_ms );

    // Add the regions of interest to the JSON
    cJSON *rois;
    cJSON_AddItemToObject(root, "regions_of_interest", 		rois=cJSON_CreateArray());
    for (unsigned int i=0;i<results.regionsOfInterest.size();i++)
    {
	  std::cout << "ALPR_IMPL 18.1" << std::endl;
      cJSON *roi_object;
      roi_object = cJSON_CreateObject();
      cJSON_AddNumberToObject(roi_object, "x",  results.regionsOfInterest[i].x);
      cJSON_AddNumberToObject(roi_object, "y",  results.regionsOfInterest[i].y);
      cJSON_AddNumberToObject(roi_object, "width",  results.regionsOfInterest[i].width);
      cJSON_AddNumberToObject(roi_object, "height",  results.regionsOfInterest[i].height);

      cJSON_AddItemToArray(rois, roi_object);
    }


    cJSON_AddItemToObject(root, "results", 		jsonResults=cJSON_CreateArray());
    for (unsigned int i = 0; i < results.plates.size(); i++)
    {
	  std::cout << "ALPR_IMPL 18.2" << std::endl;
      cJSON *resultObj = createJsonObj( &results.plates[i] );
      cJSON_AddItemToArray(jsonResults, resultObj);
    }

    // Print the JSON object to a string and return
    char *out;
    out=cJSON_PrintUnformatted(root);

    cJSON_Delete(root);

    string response(out);

    free(out);
    return response;
  }



  std::string AlprImpl::toJson( const AlprPlateResult result )
  {
	std::cout << "ALPR_IMPL 19" << std::endl;
    cJSON *resultObj = createJsonObj( &result );
    
    char *out;
    out=cJSON_PrintUnformatted(resultObj);

    cJSON_Delete(resultObj);

    string response(out);

    free(out);
    
    return response;
  }
  cJSON* AlprImpl::createJsonObj(const AlprPlateResult* result)
  {
	std::cout << "ALPR_IMPL 20" << std::endl;
    cJSON *root, *coords, *candidates;

    root=cJSON_CreateObject();

    cJSON_AddStringToObject(root,"plate",		result->bestPlate.characters.c_str());
    cJSON_AddNumberToObject(root,"confidence",		result->bestPlate.overall_confidence);
    cJSON_AddNumberToObject(root,"matches_template",	result->bestPlate.matches_template);

    cJSON_AddNumberToObject(root,"plate_index",               result->plate_index);

    cJSON_AddStringToObject(root,"region",		result->region.c_str());
    cJSON_AddNumberToObject(root,"region_confidence",	result->regionConfidence);

    cJSON_AddNumberToObject(root,"processing_time_ms",	result->processing_time_ms);
    cJSON_AddNumberToObject(root,"requested_topn",	result->requested_topn);

    cJSON_AddItemToObject(root, "coordinates", 		coords=cJSON_CreateArray());
    for (int i=0;i<4;i++)
    {
      cJSON *coords_object;
      coords_object = cJSON_CreateObject();
      cJSON_AddNumberToObject(coords_object, "x",  result->plate_points[i].x);
      cJSON_AddNumberToObject(coords_object, "y",  result->plate_points[i].y);

      cJSON_AddItemToArray(coords, coords_object);
    }


    cJSON_AddItemToObject(root, "candidates", 		candidates=cJSON_CreateArray());
    for (unsigned int i = 0; i < result->topNPlates.size(); i++)
    {
	  std::cout << "ALPR_IMPL 21" << std::endl;
      cJSON *candidate_object;
      candidate_object = cJSON_CreateObject();
      cJSON_AddStringToObject(candidate_object, "plate",  result->topNPlates[i].characters.c_str());
      cJSON_AddNumberToObject(candidate_object, "confidence",  result->topNPlates[i].overall_confidence);
      cJSON_AddNumberToObject(candidate_object, "matches_template",  result->topNPlates[i].matches_template);

      cJSON_AddItemToArray(candidates, candidate_object);
    }

    return root;
  }

  AlprResults AlprImpl::fromJson(std::string json) {
	std::cout << "ALPR_IMPL 22" << std::endl;
    AlprResults allResults;

    cJSON* root = cJSON_Parse(json.c_str());

    int version = cJSON_GetObjectItem(root, "version")->valueint;
    allResults.epoch_time = (int64_t) cJSON_GetObjectItem(root, "epoch_time")->valuedouble;
    allResults.img_width = cJSON_GetObjectItem(root, "img_width")->valueint;
    allResults.img_height = cJSON_GetObjectItem(root, "img_height")->valueint;
    allResults.total_processing_time_ms = cJSON_GetObjectItem(root, "processing_time_ms")->valueint;


    cJSON* rois = cJSON_GetObjectItem(root,"regions_of_interest");
    int numRois = cJSON_GetArraySize(rois);
    for (int c = 0; c < numRois; c++)
    {
	  std::cout << "ALPR_IMPL 23" << std::endl;
      cJSON* roi = cJSON_GetArrayItem(rois, c);
      int x = cJSON_GetObjectItem(roi, "x")->valueint;
      int y = cJSON_GetObjectItem(roi, "y")->valueint;
      int width = cJSON_GetObjectItem(roi, "width")->valueint;
      int height = cJSON_GetObjectItem(roi, "height")->valueint;

      AlprRegionOfInterest alprRegion(x,y,width,height);
      allResults.regionsOfInterest.push_back(alprRegion);
    }

    cJSON* resultsArray = cJSON_GetObjectItem(root,"results");
    int resultsSize = cJSON_GetArraySize(resultsArray);

    for (int i = 0; i < resultsSize; i++)
    {
	  std::cout << "ALPR_IMPL 24" << std::endl;
      cJSON* item = cJSON_GetArrayItem(resultsArray, i);
      AlprPlateResult plate;

      //plate.bestPlate = cJSON_GetObjectItem(item, "plate")->valuestring;
      plate.processing_time_ms = cJSON_GetObjectItem(item, "processing_time_ms")->valuedouble;
      plate.plate_index = cJSON_GetObjectItem(item, "plate_index")->valueint;
      plate.region = std::string(cJSON_GetObjectItem(item, "region")->valuestring);
      plate.regionConfidence = cJSON_GetObjectItem(item, "region_confidence")->valueint;
      plate.requested_topn = cJSON_GetObjectItem(item, "requested_topn")->valueint;


      cJSON* coordinates = cJSON_GetObjectItem(item,"coordinates");
      for (int c = 0; c < 4; c++)
      {
        cJSON* coordinate = cJSON_GetArrayItem(coordinates, c);
        AlprCoordinate alprcoord;
        alprcoord.x = cJSON_GetObjectItem(coordinate, "x")->valueint;
        alprcoord.y = cJSON_GetObjectItem(coordinate, "y")->valueint;

        plate.plate_points[c] = alprcoord;
      }

      cJSON* candidates = cJSON_GetObjectItem(item,"candidates");
      int numCandidates = cJSON_GetArraySize(candidates);
      for (int c = 0; c < numCandidates; c++)
      {
        cJSON* candidate = cJSON_GetArrayItem(candidates, c);
        AlprPlate plateCandidate;
        plateCandidate.characters = std::string(cJSON_GetObjectItem(candidate, "plate")->valuestring);
        plateCandidate.overall_confidence = cJSON_GetObjectItem(candidate, "confidence")->valuedouble;
        plateCandidate.matches_template = (cJSON_GetObjectItem(candidate, "matches_template")->valueint) != 0;

        plate.topNPlates.push_back(plateCandidate);

        if (c == 0)
        {
          plate.bestPlate = plateCandidate;
        }
      }

      allResults.plates.push_back(plate);
    }


    cJSON_Delete(root);


    return allResults;
  }

  void AlprImpl::setCountry(std::string country) {
    config->load_countries(country);
    loadRecognizers();
  }

  void AlprImpl::setPrewarp(std::string prewarp_config)
  {
    if (prewarp_config.length() == 0)
      prewarp ->clear();
    else
      prewarp->initialize(prewarp_config);
  }
  
  void AlprImpl::setMask(unsigned char* pixelData, int bytesPerPixel, int imgWidth, int imgHeight) {
	std::cout << "ALPR_IMPL 25" << std::endl;

    try
    {
      int arraySize = imgWidth * imgHeight * bytesPerPixel;
      cv::Mat imgData = cv::Mat(arraySize, 1, CV_8U, pixelData);
      cv::Mat mask = imgData.reshape(bytesPerPixel, imgHeight);

      typedef std::map<std::string, AlprRecognizers>::iterator it_type;
      for (it_type iterator = recognizers.begin(); iterator != recognizers.end(); iterator++)
        iterator->second.plateDetector->setMask(mask);
    }
    catch (cv::Exception& e)
    {
      std::cerr << "Caught (and ignoring) error in setMask: " << e.msg << std::endl;
    }
  }



  void AlprImpl::setDetectRegion(bool detectRegion)
  {
    
    this->detectRegion = detectRegion;


  }
  void AlprImpl::setTopN(int topn)
  {
    this->topN = topn;
  }
  void AlprImpl::setDefaultRegion(string region)
  {
    this->defaultRegion = region;
  }

  std::string AlprImpl::getVersion()
  {
    std::stringstream ss;

    ss << OPENALPR_MAJOR_VERSION << "." << OPENALPR_MINOR_VERSION << "." << OPENALPR_PATCH_VERSION;
    return ss.str();
  }
  
  
  void AlprImpl::loadRecognizers() {
	std::cout << "ALPR_IMPL 26" << std::endl;
    for (unsigned int i = 0; i < config->loaded_countries.size(); i++)
    {
      config->setCountry(config->loaded_countries[i]);

      if (recognizers.find(config->country) == recognizers.end())
      {
        // Country training data has not already been loaded.  Load it.
        AlprRecognizers recognizer;
        recognizer.plateDetector = createDetector(config, prewarp);
        recognizer.ocr = createOcr(config);

        #ifndef SKIP_STATE_DETECTION
        recognizer.stateDetector = new StateDetector(this->config->country, this->config->config_file_path, this->config->runtimeBaseDir);
        #else
        recognizer.stateDetector = NULL;
        #endif

        recognizers[config->country] = recognizer;
      }

    }
  }

  
  cv::Mat AlprImpl::getCharacterTransformMatrix(PipelineData* pipeline_data ) {
	std::cout << "ALPR_IMPL 26" << std::endl;
    std::vector<Point2f> crop_corners;
    crop_corners.push_back(Point2f(0,0));
    crop_corners.push_back(Point2f(pipeline_data->crop_gray.cols,0));
    crop_corners.push_back(Point2f(pipeline_data->crop_gray.cols,pipeline_data->crop_gray.rows));
    crop_corners.push_back(Point2f(0,pipeline_data->crop_gray.rows));

    // Transform the points from the cropped region (skew corrected license plate region) back to the original image
    cv::Mat transmtx = cv::getPerspectiveTransform(crop_corners, pipeline_data->plate_corners);
    
    return transmtx;
  }
  
  std::vector<AlprCoordinate> AlprImpl::getCharacterPoints(cv::Rect char_rect, cv::Mat transmtx ) {
    
    //std::cout << "ALPR_IMPL 27" << std::endl;
    std::vector<Point2f> points;
    points.push_back(Point2f(char_rect.x, char_rect.y));
    points.push_back(Point2f(char_rect.x + char_rect.width, char_rect.y));
    points.push_back(Point2f(char_rect.x + char_rect.width, char_rect.y + char_rect.height));
    points.push_back(Point2f(char_rect.x, char_rect.y + char_rect.height));
    
    cv::perspectiveTransform(points, points, transmtx);
    
    // If using prewarp, remap the points to the original image
    points = prewarp->projectPoints(points, true);
        
    
    std::vector<AlprCoordinate> cornersvector;
    for (int i = 0; i < 4; i++)
    {
      AlprCoordinate coord;
      coord.x = round(points[i].x);
      coord.y = round(points[i].y);
      cornersvector.push_back(coord);
    }
    
    return cornersvector;
  }


}
