package com.openalpr.jni;

import com.openalpr.jni.json.JSONException;
import com.openalpr.jni.json.JSONObject;
import com.openalpr.jni.json.JSONArray;


import java.util.ArrayList;
import java.util.List;



public class AlprSplitReturn {
	//image
	
	//warpedRegion as vector
	AlprRegionOfInterest javaWarpedRegion;
	//Region as vector
	AlprRegionOfInterest javaRegion;
	//Response as vector and results
	AlprRegionOfInterest javaResponseVector;
	AlprResults javaResponseResults;
	
	    AlprSplitReturn(JSONObject roiObjWarpedRegion,JSONObject roiObjRegion, JSONObject roiObjResponseVector, 
			String json ) throws JSONException
    {
		
        javaWarpedRegion = AlprRegionOfInterest(roiObjWarpedRegion);
		javaRegion = AlprRegionOfInterest(roiObjRegion);
		javaResponseVector = AlprRegionOfInterest(roiObjResponseVector);
		javaResponseResults = AlprResults(json);
		
    }
	
	    public int GetJavaWarpedRegion() {
        return javaWarpedRegion;
    }
	
	    public int GetJavaRegion() {
        return javaRegion;
    }
	    public int GetJavaResponseVector() {
        return javaResponseVector;
    }
	    public int GetJavaResponseResults() {
        return javaResponseResults;
    }
    
}
