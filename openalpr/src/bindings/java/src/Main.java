import com.openalpr.jni.*;

import javax.imageio.ImageIO;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.ByteArrayInputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.nio.file.Files;

public class Main {

    public static void main(String[] args) throws Exception {

        String country = "us", configfile = "", runtimeDataDir = "", licensePlate = "/home/matt/Pictures/plates/2fps/frames_048.png";

        Alpr alpr = new Alpr(country, configfile, runtimeDataDir);

        alpr.setTopN(10);
        //alpr.setDefaultRegion("wa");

        // Read an image into a byte array and send it to OpenALPR .cpp or .java?
        Path path = Paths.get(licensePlate);
        byte[] imagedata = Files.readAllBytes(path);

        SplitReturn split1Return;
        AlprResults results;

        try {
            split1Return = alpr.firstSplit(imagedata);
            // Call next split with results as image data;
            // ROI will be setup for frame 26@2fps
            results = alpr.nextSplit(split1Return.img, split1Return.rect);

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
        } catch (Exception e) {
            e.printStackTrace();
        } finally {
            alpr.unload();
        }
    }
}
