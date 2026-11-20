import javax.swing.*;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import java.awt.*;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

public class Screen {
    public Main main;

    public JFrame frame;

    public JPanel drawPanel;
    public JPanel toolPanel;

    public JButton saveButton;

    public JTextField ySizeField;
    public JTextField xSizeField;

    public JTextField currentNumField;
    public JTextField nameField;

    public Screen(Main main, int width, int height) {
        this.main = main;
        frame = new JFrame("Arduino Tile Editor");
        frame.setSize(1000, 800);
        frame.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        frame.setLayout(new BorderLayout());

        drawPanel = new JPanel();
        drawPanel.setLayout(new GridLayout(width,height));

        toolPanel = new JPanel();

        toolPanel.setPreferredSize(new Dimension(200, 800));
        drawPanel.setPreferredSize(new Dimension(800, 800));

        frame.add(drawPanel, BorderLayout.CENTER);

        frame.add(toolPanel, BorderLayout.WEST);

        reloadScreen();


        frame.setVisible(true);
    }
    public void reloadScreen() {
        drawPanel.removeAll();
        for(int i = 0; i < main.x * main.y; i++) {
            int num = i;
            JButton button = new JButton();
            button.setBorder(BorderFactory.createLineBorder(Color.black));
            button.setBackground(Color.white);
            button.setName(num + "");
            button.setSize(drawPanel.getWidth() / main.x, drawPanel.getHeight() / main.y);
            button.setText("" + main.getTile(num));
            button.addMouseListener(new MouseAdapter() {
                @Override
                public void mouseClicked(MouseEvent e) {
                    if(SwingUtilities.isLeftMouseButton(e)) {
                        main.setTile(num, main.currentNum);
                        button.setText("" + main.getTile(num));
                        System.out.println(num);
                    }
                }
            });
            System.out.println("Adding button: " + num);
            drawPanel.add(button);
        }
        toolPanel.removeAll();
        currentNumField = new JTextField();
        currentNumField.setPreferredSize(new Dimension(100, 20));
        currentNumField.getDocument().addDocumentListener(new DocumentListener() {
            @Override
            public void insertUpdate(DocumentEvent e) {
                handleValueChange();
            }

            @Override
            public void removeUpdate(DocumentEvent e) {
                handleValueChange();
            }

            @Override
            public void changedUpdate(DocumentEvent e) {
                handleValueChange();
            }

            // Helper method to retrieve the text
            private void handleValueChange() {
                String updatedText = currentNumField.getText();
                System.out.println("Current value: " + updatedText);
                if(!updatedText.isEmpty()) {
                    main.currentNum = Integer.parseInt(updatedText);
                }
            }
        });
        toolPanel.add(currentNumField);
        nameField = new JTextField();
        nameField.setPreferredSize(new Dimension(100, 20));
        toolPanel.add(nameField);

        saveButton = new JButton("Save");
        saveButton.setPreferredSize(new Dimension(100, 20));
        saveButton.addMouseListener(new MouseAdapter() {
            @Override
            public void mouseClicked(MouseEvent e) {
                if(SwingUtilities.isLeftMouseButton(e)) {
                    main.save(nameField.getText());
                }
            }
        });
        saveButton.setEnabled(true);
        toolPanel.add(saveButton);

        frame.revalidate();
        frame.repaint();
    }
}
