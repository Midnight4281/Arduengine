import javax.swing.*;
import java.io.File;
import java.io.IOException;
import java.nio.file.FileAlreadyExistsException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.jar.JarEntry;

public class Main {
    public int currentNum = 1;

    public Screen screen;

    public int x = 20;
    public int y = 20;

    public int maxX = 20;
    public int maxY = 20;
    public List<Integer> list = new ArrayList<>();

    void main(String[] args) {
        init();
        screen = new Screen(this, x, y);
    }
    public void init() {
        for(int _x = 0; _x < x; _x++) {
            for(int _y = 0; _y < y; _y++) {
                list.add(0);
            }
        }
    }
    public int getTile(int i) {
        return list.get(i);
    }
    public int getTile(int _x, int _y) {
        return list.get(_x + x * _y);
    }
    public void setTile(int i, int v) {
        list.set(i, v);
    }
    public void save(String name) {
        //save the x and y size, then all of the data
        Path dataPath = Path.of(name + ".bd").toAbsolutePath();
        try {
            StringBuilder data = new StringBuilder();
            Files.createFile(dataPath);
            data.append(x);
            data.append(y);
            for(Integer i : list) {
                data.append(getTile(i));
            }
            Files.write(dataPath, data.toString().getBytes());
            System.out.println("File created successfully.");
        } catch (FileAlreadyExistsException e) {

        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
