import javax.imageio.ImageIO;
import java.awt.*;
import java.awt.image.BufferedImage;
import java.io.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.Scanner;

public class Main {
    public int channelSteps = 1;
    void main(String[] args) {
        Scanner input = new Scanner(System.in);
        Scanner scan = new Scanner(System.in);
        System.out.println("Enter channel steps: (recommended: 3)");
        channelSteps = input.nextInt();
        System.out.println("Enter the filepath of a PNG image");
        String text = scan.nextLine();
        String r = text.replace("\"", "");
        System.out.println(r);
        Path p = Paths.get(r);
        try {
            // 1. Load the PNG file
            File file = new File(p.toString());
            BufferedImage image = ImageIO.read(file);

            // 2. Get image dimensions
            int width = image.getWidth();
            int height = image.getHeight();

            // 2. Initialize a Buffered Image with RGB color profiles
            BufferedImage bufferedImage = new BufferedImage(width, height, BufferedImage.TYPE_INT_RGB);

            // 3. Loop through every pixel row by column
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {

                    // Get the packed 32-bit ARGB color value
                    int pixel = image.getRGB(x, y);

                    // 4. Extract individual channels using bitwise shifting and masking
                    int alpha = (pixel >> 24) & 0xff;

                    int red   = (pixel >> 16) & 0xff;
                    red = limitChannel(red);
                    int green = (pixel >> 8)  & 0xff;
                    green = limitChannel(green);
                    int blue  =  pixel        & 0xff;
                    blue = limitChannel(blue);

                    int f = (red << 16) | (green << 8) | blue;
                    // Example usage: Print the coordinates and ARGB values
                    //System.out.printf("Pixel at (%d,%d) -> A:%d, R:%d, G:%d, B:%d%n",
                    //        x, y, alpha, red, green, blue);
                    if (alpha != 255) {//draw anything with alpha < full as rgb (1,1,1) [not drawn by Arduino]
                        f = 0x00010101;
                    }
                    bufferedImage.setRGB(x, y, f);
                }
            }
            try {
                String fileName = file.getName();
                String result = fileName.replaceFirst("(?i)\\.png$", "");
                result = result.replaceFirst("(?i)\\.bmp$", "");
                File outputFile = new File(result + ".bmp");

                System.out.println("enter the x scale for the image");
                int xScale = input.nextInt();
                System.out.println("enter the y scale for the image");
                int yScale = input.nextInt();
                int xSize = xScale * image.getWidth();
                int ySize = yScale * image.getHeight();
                String metaPath = (result + ".sc");
                ByteBuffer buffer = ByteBuffer.allocate(8);
                buffer.order(ByteOrder.LITTLE_ENDIAN);

                buffer.putShort((short) xScale);
                buffer.putShort((short) yScale);
                buffer.putShort((short) xSize);
                buffer.putShort((short) ySize);

                try {
                    Files.write(Path.of(metaPath), buffer.array());
                    System.out.println("File created successfully.");
                } catch (IOException e) {
                    e.printStackTrace();
                }

                boolean success = ImageIO.write(bufferedImage, "BMP", outputFile);
                if (success) {
                    System.out.println("Custom BMP file successfully generated: " + outputFile.getAbsolutePath());
                } else {
                    System.out.println("Error: BMP writer format driver not found.");
                }
            } catch (IOException e) {
                System.err.println("Failed to write the image file to disk.");
                e.printStackTrace();
            }

        } catch (IOException e) {
            System.err.println("Error reading the image file: " + e.getMessage());
        } catch (Exception e) {
            throw new RuntimeException(e);
        }

    }
    public int limitChannel(int value) {
        int div = Math.clamp(1, channelSteps, 255);
        int clamped = (int) Math.clamp(value, 0, 255);

        long rounded = Math.round(clamped / (255.0 / div));

        int t = (int) Math.round(rounded * (255.0 / div));

        if(t < 128 && t > 3) {
            t = 128;
        }

        return t;
    }

}
