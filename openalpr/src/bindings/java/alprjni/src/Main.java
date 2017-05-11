import com.openalpr.jni.Alpr;
import com.openalpr.jni.AlprPlate;
import com.openalpr.jni.AlprPlateResult;
import com.openalpr.jni.AlprResults;

import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.Files;

public class Main {

    public static void main(String[] args) throws Exception {

        String country = "us", configfile = "", runtimeDataDir = "", licensePlate = "/home/kane/csc450ALPR/openalpr/src/build/frames_025.png";

        Alpr alpr = new Alpr(country, configfile, runtimeDataDir);


        alpr.setTopN(1);

        // Read an image into a byte array and send it to OpenALPR .cpp or .java?
        Path path = Paths.get(licensePlate);
        byte[] imagedata = Files.readAllBytes(path);

        AlprResults results = alpr.recognize(imagedata);

		
        System.out.println("OpenALPR Version: " + alpr.getVersion());
        System.out.println("Image Size: " + results.getImgWidth() + "x" + results.getImgHeight());
        System.out.println("Processing Time: " + results.getTotalProcessingTimeMs() + " ms");
        System.out.println("Found " + results.getPlates().size() + " results");

        System.out.format("  %-15s%-8s\n", "Plate Number", "Confidence");
        for (AlprPlateResult result : results.getPlates())
        {
            for (AlprPlate plate : result.getTopNPlates()) {
                if (plate.isMatchesTemplate())
                    System.out.print("  * ");
                else
                    System.out.print("  - ");
                System.out.format("%-15s%-8f\n", plate.getCharacters(), plate.getOverallConfidence());
            }
        }


        // Make sure to call this to release memory
        alpr.unload();
		//impl.unload();
    }
}
