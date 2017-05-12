package com.openalpr.jni;

import com.openalpr.jni.json.JSONException;

public class Alpr {
    static {
        // Load the OpenALPR library at runtime
        // openalprjni.dll (Windows) or libopenalprjni.so (Linux/Mac)
        System.loadLibrary("openalprjni");
    }

    private native void initialize(String country, String configFile, String runtimeDir);
    private native void dispose();

    private native boolean is_loaded();
    private native String native_recognize(String imageFile);
    //private native byte[] native_recognize(byte[] imageBytes);
    private native byte[] native_firstSplit(byte[] imageBytes, SplitRect rect);
    private native String native_nextSplit(byte[] imageBytes, SplitRect rect);
    private native String native_recognize(long imageData, int bytesPerPixel, int imgWidth, int imgHeight);

    private native void set_default_region(String region);
    private native void detect_region(boolean detectRegion);
    private native void set_top_n(int topN);
    private native String get_version();



    public Alpr(String country, String configFile, String runtimeDir)
    {
        initialize(country, configFile, runtimeDir);
    }

    public void unload()
    {
        dispose();
    }

    public boolean isLoaded()
    {
        return is_loaded();
    }
	//When does this get called? image is converted to bytes
	// in main.java.
    public AlprResults recognize(String imageFile) throws AlprException
    {
        try {
            String json = native_recognize(imageFile);
            return new AlprResults(json);
        } catch (JSONException e)
        {
            throw new AlprException("Unable to parse ALPR results");
        }
    }

	//main.java calls here
    public SplitReturn firstSplit(byte[] imageBytes) throws AlprException
    {
        SplitRect rect = new SplitRect();

        byte[] img = native_firstSplit(imageBytes, rect);

        return new SplitReturn(img, rect);
    }

    public AlprResults nextSplit(byte[] imageBytes, SplitRect rect) throws AlprException {
        try {
            String json = native_nextSplit(imageBytes, rect);
            return new AlprResults(json);
        } catch (JSONException e) {
            e.printStackTrace();
            throw new AlprException("Unable to parse ALPR results");
        }
    }


	//where is this used
    public AlprResults recognize(long imageData, int bytesPerPixel, int imgWidth, int imgHeight) throws AlprException
    {
        try {
            String json = native_recognize(imageData, bytesPerPixel, imgWidth, imgHeight);
            return new AlprResults(json);
        } catch (JSONException e)
        {
            throw new AlprException("Unable to parse ALPR results");
        }
    }


    public void setTopN(int topN)
    {
        set_top_n(topN);
    }

    public void setDefaultRegion(String region)
    {
        set_default_region(region);
    }

    public void setDetectRegion(boolean detectRegion)
    {
        detect_region(detectRegion);
    }

    public String getVersion()
    {
        return get_version();
    }
}
