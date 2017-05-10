import com.openalpr.jni.*;

import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.Files;

public class Main {

    public static void main(String[] args) throws Exception {

        String country = "", configfile = "", runtimeDataDir = "", licensePlate = "";
        if (args.length == 1) {
                country = "us";
                //configfile = args[1];
                //runtimeDataDir = args[make
            // 2];
                licensePlate = args[0];
        }
        else
        {
            System.err.println("Program requires 4 arguments: Country, Config File, runtime_data dir, and license plate image");
            System.exit(1);
        }

        Alpr alpr = new Alpr(country, configfile, runtimeDataDir);

        alpr.setTopN(1);

        // Read an image into a byte array and send it to OpenALPR .java
        Path path = Paths.get(licensePlate);
        byte[] imagedata = Files.readAllBytes(path);

        AlprRegionOfInterest results = alpr.recognize(imagedata);

		
       // System.out.println("OpenALPR Version: " + alpr.getVersion());
       // System.out.println("Image Size: " + results.getImgWidth() + "x" + results.getImgHeight());
//        System.out.println("Processing Time: " + results.getTotalProcessingTimeMs() + " ms");
//        System.out.println("Found " + results.getPlates().size() + " results");

//        System.out.format("  %-15s%-8s\n", "Plate Number", "Confidence");
//        for (AlprPlateResult result : results.getPlates())
//        {
//            for (AlprPlate plate : result.getTopNPlates()) {
//                if (plate.isMatchesTemplate())
//                    System.out.print("  * ");
//                else
//                    System.out.print("  - ");
//                System.out.format("%-15s%-8f\n", plate.getCharacters(), plate.getOverallConfidence());
//            }
//        }


        // Make sure to call this to release memory
        alpr.unload();
		//impl.unload();
    }
}
