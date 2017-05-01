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


#ifndef OPENALPR_ALPRIMPL_H
#define OPENALPR_ALPRIMPL_H

#include <list>
#include <sstream>
#include <vector>
#include <queue>

#include "alpr.h"
#include "config.h"

#include "detection/detector.h"
#include "detection/detectorfactory.h"

#include "prewarp.h"

#include "licenseplatecandidate.h"
#include "../statedetection/state_detector.h"
#include "ocr/ocr.h"
#include "ocr/ocrfactory.h"

#include "constants.h"

#include "cjson.h"

#include "pipeline_data.h"

#include "prewarp.h"

#include <opencv2/core/core.hpp>
   
#include "support/platform.h"
#include "support/utf8.h"

#define DEFAULT_TOPN 25
#define DEFAULT_DETECT_REGION false

#define ALPR_NULL_PTR 0






namespace alpr
{

  struct AlprFullDetails
  {
    std::vector<PlateRegion> plateRegions;
    AlprResults results;
  };

  struct AlprRecognizers
  {
    Detector* plateDetector;
    StateDetector* stateDetector;
    OCR* ocr;
  };

  class SplitReturn{
	
    private:
	cv::Mat img;
	std::vector<cv::Rect> warpedRegion;
	AlprFullDetails response;
	std::vector<cv::Rect> region;

    public:
	SplitReturn(cv::Mat, std::vector<cv::Rect>, AlprFullDetails, std::vector<cv::Rect>);
	SplitReturn();

	AlprFullDetails get_response();
	cv::Mat get_image();
	std::vector<cv::Rect> get_warped_region();
	std::vector<cv::Rect> get_region();
  };

  class SplitReturn2{
    private:
	cv::Mat grayImg;
	std::queue<PlateRegion> plateQueue;
	//AlprRecognizers country_recognizers;
	std::vector<PlateRegion> warpedPlateRegions;

    public:
	//SplitReturn2(cv::Mat, std::queue<PlateRegion>, AlprRecognizers, std::vector<PlateRegion>);
	SplitReturn2(cv::Mat, std::queue<PlateRegion>, std::vector<PlateRegion>);
	SplitReturn2();
	
	cv::Mat get_image();
	std::queue<PlateRegion> get_queue();
	AlprRecognizers get_country_recognizers();
	std::vector<PlateRegion> get_warped_regions();
  };
  

  class AlprImpl
  {

    public:
      AlprImpl* impl;
      AlprImpl(const std::string country, const std::string configFile = "", const std::string runtimeDir = "");
      virtual ~AlprImpl();

      /*was AlprResults*/
      SplitReturn recognizeFullDetails(cv::Mat img, std::vector<cv::Rect> regionsOfInterest);

      AlprResults recognize( std::vector<char> imageBytes );

      AlprResults recognize( std::vector<char> imageBytes, std::vector<AlprRegionOfInterest> regionsOfInterest );
      SplitReturn recognize( unsigned char* pixelData, int bytesPerPixel, int imgWidth, int imgHeight, std::vector<AlprRegionOfInterest> regionsOfInterest );
      
      SplitReturn2 split2impl(SplitReturn split1return);
	  AlprFullDetails split3impl(SplitReturn2 split2return);
	  //SplitReturn4 split4impl(SplitReturn3 split3return, SplitReturn2 split2return);
	  //AlprFullDetails split5impl(SplitReturn4 split4return, SplitReturn2 split2return);

	  AlprFullDetails split6impl(AlprFullDetails iter_results, SplitReturn split1return);

      SplitReturn recognize( cv::Mat img, std::vector<cv::Rect> regionsOfInterest);

      AlprResults recognize( cv::Mat img);	
      
      SplitReturn2 analyzeSingleCountry(cv::Mat grayImg, std::vector<cv::Rect> regionsOfInterest);

      void setCountry(std::string country);
      void setPrewarp(std::string prewarp_config);
      void setMask(unsigned char* pixelData, int bytesPerPixel, int imgWidth, int imgHeight);
      
      void setDetectRegion(bool detectRegion);
      void setTopN(int topn);
      void setDefaultRegion(std::string region);

      static std::string toJson( const AlprResults results );
      static std::string toJson( const AlprPlateResult result );
      
      static AlprResults fromJson(std::string json);
      static std::string getVersion();

      static cJSON* createJsonObj(const AlprPlateResult* result);
      
      Config* config;

      bool isLoaded();

    private:

      std::map<std::string, AlprRecognizers> recognizers;

      PreWarp* prewarp;

      int topN;
      bool detectRegion;
      std::string defaultRegion;

      void loadRecognizers();
      
      cv::Mat getCharacterTransformMatrix(PipelineData* pipeline_data );
      std::vector<AlprCoordinate> getCharacterPoints(cv::Rect char_rect, cv::Mat transmtx);
      std::vector<cv::Rect> convertRects(std::vector<AlprRegionOfInterest> regionsOfInterest);

  };
}


#endif // OPENALPR_ALPRIMPL_H
