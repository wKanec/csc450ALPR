#include <alpr.h>
#include <alpr_impl.h>
 
#include "com_openalpr_jni_Alpr.h"

using namespace alpr;

bool initialized = false;
//static Alpr* nativeAlpr;
AlprImpl* nativeImpl;
typedef unsigned char byte;


JNIEXPORT void JNICALL Java_com_openalpr_jni_Alpr_initialize
  (JNIEnv *env, jobject thisObj, jstring jcountry, jstring jconfigFile, jstring jruntimeDir)
  {
    //printf("Initialize");

    // Convert strings from java to C++ and release resources
    const char *ccountry = env->GetStringUTFChars(jcountry, NULL);
    std::string country(ccountry);
    env->ReleaseStringUTFChars(jcountry, ccountry);

    const char *cconfigFile = env->GetStringUTFChars(jconfigFile, NULL);
    std::string configFile(cconfigFile);
    env->ReleaseStringUTFChars(jconfigFile, cconfigFile);

    const char *cruntimeDir = env->GetStringUTFChars(jruntimeDir, NULL);
    std::string runtimeDir(cruntimeDir);
    env->ReleaseStringUTFChars(jruntimeDir, cruntimeDir);


    //nativeAlpr = new alpr::Alpr(country, configFile, runtimeDir);
	nativeImpl = new AlprImpl(country, configFile, runtimeDir);

    initialized = true;
    return;
  }

JNIEXPORT void JNICALL Java_com_openalpr_jni_Alpr_dispose
  (JNIEnv *env, jobject thisObj)
  {
    //printf("Dispose");
    initialized = false;
    delete nativeImpl;
	//delete nativeImpl;
  }


JNIEXPORT jboolean JNICALL Java_com_openalpr_jni_Alpr_is_1loaded
  (JNIEnv *env, jobject thisObj)
  {
    //printf("IS LOADED");

    if (!initialized)
      return false;

    return (jboolean) nativeImpl->isLoaded();
	//return (jboolean) nativeImpl->isLoaded();
        
  }

JNIEXPORT jstring JNICALL Java_com_openalpr_jni_Alpr_native_1recognize__Ljava_lang_String_2
  (JNIEnv *env, jobject thisObj, jstring jimageFile)
  {
    //printf("Recognize file");

    // Convert strings from java to C++ and release resources
    const char *cimageFile = env->GetStringUTFChars(jimageFile, NULL);
    std::string imageFile(cimageFile);
    env->ReleaseStringUTFChars(jimageFile, cimageFile);

    AlprResults results = nativeImpl->recognize(imageFile);

    std::string json = Alpr::toJson(results);

    return env->NewStringUTF(json.c_str());
  }

//This is the method called by Matt will be split 1??
JNIEXPORT jbyteArray JNICALL Java_com_openalpr_jni_Alpr_native_1firstSplit
  (JNIEnv *env, jobject thisObj, jbyteArray jimageBytes, jobject rect)
  {

  //original
  int len = env->GetArrayLength (jimageBytes);
  unsigned char* buf = new unsigned char[len];
  env->GetByteArrayRegion (jimageBytes, 0, len, reinterpret_cast<jbyte*>(buf));

  std::vector<char> cvec(buf, buf+len);

  SplitReturn split1results = nativeImpl->recognize(cvec);
  //std::string json = Alpr::toJson(results);

//  int width = 1280;
//  int height = 720;
//  int bytesPerPixel = 3;
//jbyte * pNV21FrameData = env->GetByteArrayElements(jimageBytes, 0);
//  std::vector<AlprRegionOfInterest> regionsOfInterest;
//  regionsOfInterest.push_back(AlprRegionOfInterest(0, 0, width, height));
//
//  SplitReturn split1results = nativeImpl ->recognize((unsigned char *)pNV21FrameData, bytesPerPixel, width, height, regionsOfInterest);
    SplitReturn2 split2results = nativeImpl->split2impl(split1results);

    // Set rect data
    // Get the class of the input object
    jclass clazz = env->GetObjectClass(rect);

    // Get Field references
    jfieldID xfid = env->GetFieldID(clazz, "x", "I");
    jfieldID yfid = env->GetFieldID(clazz, "y", "I");
    jfieldID heightfid = env->GetFieldID(clazz, "height", "I");
    jfieldID widthfid = env->GetFieldID(clazz, "width", "I");

    //Get roi rect
    cv::Rect r = split2results.get_warped_regions()[0].rect;

    // Set fields for object
    env->SetIntField(rect, xfid, r.x);
    env->SetIntField(rect, yfid, r.y);
    env->SetIntField(rect, heightfid, r.height);
    env->SetIntField(rect, widthfid, r.width);

    // Convert image into byteArray
    cv::Mat returnImage = split2results.get_image();
    int size = returnImage.total() * returnImage.elemSize();
    byte * imageBytes = new byte[size];  // you will have to delete[] that later
    std::memcpy(imageBytes,returnImage.data,size * sizeof(byte));
    delete buf;


    // Convert byteArray into jbyteArray and return
    jbyteArray array = env->NewByteArray (size);
    env->SetByteArrayRegion (array, 0, size, reinterpret_cast<jbyte*>(imageBytes));
    delete imageBytes;

    return array;
  }

//This is the method called by Matt will be split 1??
JNIEXPORT jstring JNICALL Java_com_openalpr_jni_Alpr_native_1nextSplit
        (JNIEnv *env, jobject thisObj, jbyteArray jimageBytes, jobject rect)
{
    //original
    int len = env->GetArrayLength (jimageBytes);
    unsigned char* buf = new unsigned char[len];
    env->GetByteArrayRegion (jimageBytes, 0, len, reinterpret_cast<jbyte*>(buf));

    //std::vector<char> cvec(buf, buf+len);

    cv::Mat image = cv::Mat(720,1280,CV_8U,buf);

    // Set rect data
    // Get the class of the input object
    jclass clazz = env->GetObjectClass(rect);

    // Get Field references
    jfieldID xfid = env->GetFieldID(clazz, "x", "I");
    jfieldID yfid = env->GetFieldID(clazz, "y", "I");
    jfieldID heightfid = env->GetFieldID(clazz, "height", "I");
    jfieldID widthfid = env->GetFieldID(clazz, "width", "I");

    PlateRegion region;
    region.rect.x = env->GetIntField(rect, xfid);
    region.rect.y = env->GetIntField(rect, yfid);
    region.rect.height = env->GetIntField(rect, heightfid);
    region.rect.width = env->GetIntField(rect, widthfid);

    std::queue<PlateRegion> queue;
    queue.push(region);

    std::vector<PlateRegion> roi;
    roi.push_back(region);

    SplitReturn2 split2results = SplitReturn2(
            image,
            queue,
            roi
    );

    AlprFullDetails fullDetails = nativeImpl->split3impl(split2results);
    AlprResults results = fullDetails.results;
    std::string json = Alpr::toJson(results);

    return env->NewStringUTF(json.c_str());
}
JNIEXPORT jstring JNICALL Java_com_openalpr_jni_Alpr_native_1recognize__JIII
  (JNIEnv *env, jobject thisObj, jlong data, jint bytesPerPixel, jint width, jint height)
  {
    //printf("Recognize data pointer");

    SplitReturn splitresults = nativeImpl->recognize(
            reinterpret_cast<unsigned char*>(data),
            static_cast<int>(bytesPerPixel),
            static_cast<int>(width),
            static_cast<int>(height),
            std::vector<AlprRegionOfInterest>());

    SplitReturn2 split2return = nativeImpl-> split2impl(splitresults);
    //third split
    AlprFullDetails aFDResults = nativeImpl->split3impl(split2return);
    AlprResults results = aFDResults.results;
    std::string json = Alpr::toJson(results);

    return env->NewStringUTF(json.c_str());
  }


JNIEXPORT void JNICALL Java_com_openalpr_jni_Alpr_set_1default_1region
  (JNIEnv *env, jobject thisObj, jstring jdefault_region)
  {
    // Convert strings from java to C++ and release resources
    const char *cdefault_region = env->GetStringUTFChars(jdefault_region, NULL);
    std::string default_region(cdefault_region);
    env->ReleaseStringUTFChars(jdefault_region, cdefault_region);
    
    nativeImpl->setDefaultRegion(default_region);
  }

JNIEXPORT void JNICALL Java_com_openalpr_jni_Alpr_detect_1region
  (JNIEnv *env, jobject thisObj, jboolean detect_region)
  {
    nativeImpl->setDetectRegion(detect_region);
  }

JNIEXPORT void JNICALL Java_com_openalpr_jni_Alpr_set_1top_1n
  (JNIEnv *env, jobject thisObj, jint top_n)
  {
    nativeImpl->setTopN(top_n);
  }

JNIEXPORT jstring JNICALL Java_com_openalpr_jni_Alpr_get_1version
  (JNIEnv *env, jobject thisObj)
  {
    std::string version = nativeImpl->getVersion();

    return env->NewStringUTF(version.c_str());
  }
