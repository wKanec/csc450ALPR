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

#include "alpr.h"
#include "alpr_impl.h"

namespace alpr
{
	
//void SplitReturn::splittest(){
	//void impl->splittest();
	
//}
  // ALPR code

  Alpr::Alpr(const std::string country, const std::string configFile, const std::string runtimeDir)
  {
    impl = new AlprImpl(country, configFile, runtimeDir);
	std::cout << "ALPR 1" <<std::endl;

  }

  Alpr::~Alpr()
  {
    delete impl;
  }

  AlprResults Alpr::recognize(std::string filepath)
  {
	std::cout << "ALPR 2" <<std::endl;
    
    std::ifstream ifs(filepath.c_str(), std::ios::binary|std::ios::ate);
    
    if (ifs)
      {
	  std::cout << "ALPR 3" <<std::endl;

      std::ifstream::pos_type pos = ifs.tellg();

      std::vector<char>  buffer(pos);

      ifs.seekg(0, std::ios::beg);
      ifs.read(&buffer[0], pos);

      return this->recognize( buffer );
    }
    else
    {
	  std::cout << "ALPR 4" <<std::endl;

      std::cerr << "file does not exist: " << filepath << std::endl;
      AlprResults emptyResults;
      emptyResults.epoch_time = getEpochTimeMs();
      emptyResults.img_width = 0;
      emptyResults.img_height = 0;
      emptyResults.total_processing_time_ms = 0;
      return emptyResults;
    }
  }

  AlprResults Alpr::recognize(std::vector<char> imageBytes)
  {
	std::cout << "ALPR 5" <<std::endl;
    return impl->recognize(imageBytes);
  }

  AlprResults Alpr::recognize(std::vector<char> imageBytes, std::vector<AlprRegionOfInterest> regionsOfInterest)
  {
	  std::cout << "ALPR 6" <<std::endl;
	  AlprResults results = impl->recognize(imageBytes, regionsOfInterest);
	  //SplitReturn splitResults = impl->recognize(imageBytes, regionsOfInterest);
	  //AlprFullDetails details = impl->split2impl(splitreturn);
	  //AlprResults results = details.results;
	  return results;
  }
  //AlprResults or SplitReturn
  AlprResults Alpr::recognize(unsigned char* pixelData, int bytesPerPixel, int imgWidth, int imgHeight, std::vector<AlprRegionOfInterest> regionsOfInterest)
  {
	std::cout << "Split 1 ALPR" <<std::endl;
	SplitReturn splitreturn = impl->recognize(pixelData, bytesPerPixel, imgWidth, imgHeight, regionsOfInterest);
	AlprFullDetails details = impl->split2impl(splitreturn);
	AlprResults results = details.results;
	//sr.testsplit();
	return results;
  }
  
  //AlprResults Alpr::recognize(SplitReturn split1return);
  //{
	//std::cout<<"Split 2 ALPR"<<std::endl;
	//return impl->split2alpr(split1return);
  //}

  std::string Alpr::toJson( AlprResults results )
  {
	std::cout << "ALPR 8" <<std::endl;
    return AlprImpl::toJson(results);
  }
  std::string Alpr::toJson( AlprPlateResult result )
  {
	std::cout << "ALPR 9" <<std::endl;
    return AlprImpl::toJson(result);
  }

  AlprResults Alpr::fromJson(std::string json) {
	std::cout << "ALPR 10" <<std::endl;
    return AlprImpl::fromJson(json);
  }

  void Alpr::setCountry(std::string country) {
	std::cout << "ALPR 11" <<std::endl;
    impl->setCountry(country);
  }

  void Alpr::setPrewarp(std::string prewarp_config) {
	std::cout << "ALPR 12" <<std::endl;
    impl->setPrewarp(prewarp_config);
  }

  void Alpr::setMask(unsigned char* pixelData, int bytesPerPixel, int imgWidth, int imgHeight)
  {
	std::cout << "ALPR 13" <<std::endl;
	
    impl->setMask(pixelData, bytesPerPixel, imgWidth, imgHeight);
  }

  void Alpr::setDetectRegion(bool detectRegion)
  {
	std::cout << "ALPR 14" <<std::endl;
    impl->setDetectRegion(detectRegion);
  }

  void Alpr::setTopN(int topN)
  {
	std::cout << "ALPR 15" <<std::endl;
    impl->setTopN(topN);
  }

  void Alpr::setDefaultRegion(std::string region)
  {
	std::cout << "ALPR 16" <<std::endl;
    impl->setDefaultRegion(region);
  }

  bool Alpr::isLoaded()
  {
	std::cout << "ALPR 17" <<std::endl;
    return impl->isLoaded();
  }

  std::string Alpr::getVersion()
  {
	std::cout << "ALPR 1" <<std::endl;
    return AlprImpl::getVersion();
  }

  Config* Alpr::getConfig()
  {
	std::cout << "ALPR 1" <<std::endl;
    return impl->config;
  }
}