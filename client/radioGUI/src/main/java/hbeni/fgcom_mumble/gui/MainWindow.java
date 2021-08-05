/* 
 * This file is part of the FGCom-mumble distribution (https://github.com/hbeni/fgcom-mumble).
 * Copyright (c) 2020 Benedikt Hallinger
 * 
 * This program is free software: you can redistribute it and/or modify  
 * it under the terms of the GNU General Public License as published by  
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
package hbeni.fgcom_mumble.gui;

import hbeni.fgcom_mumble.MapWindow;
import hbeni.fgcom_mumble.Radio;
import hbeni.fgcom_mumble.State;
import hbeni.fgcom_mumble.radioGUI;
import java.awt.Color;
import java.awt.Component;
import java.awt.event.WindowEvent;
import java.net.URL;
import java.util.Iterator;
import javax.swing.BoxLayout;
import javax.swing.ImageIcon;
import javax.swing.JFrame;
import javax.swing.JInternalFrame;
import javax.swing.JPanel;
import javax.swing.JScrollBar;
import javax.swing.event.InternalFrameEvent;

/**
 *
 * @author beni
 */
public class MainWindow extends javax.swing.JFrame {

    private final About_Help helpWindow;
    private final OptionsWindow optionsWindow;
    private final LicenseWindow licenseWindow;
    
    protected State state;
    
    JPanel radioContainer; // Container holds the radio instances
    
    /**
     * Creates new form MainwWindow
     */
    public MainWindow(State st) {
        helpWindow    = new About_Help();
        optionsWindow = new OptionsWindow();
        licenseWindow = new LicenseWindow();
        state         = st;
        
        initComponents();
        URL iconURL = getClass().getResource("/fgcom_logo.png");
        ImageIcon icon = new ImageIcon(iconURL);
        this.setIconImage(icon.getImage());
        
        // Adjust the scroll panel
        radioContainer = new JPanel();
        radioContainer.setLayout(new BoxLayout(radioContainer, BoxLayout.PAGE_AXIS));
        
        // Update basic state
        updateFromState();
        
        // add radios that are already existing in the state (ie defaults)
        state.getRadios().forEach(r -> {
            radioContainer.add(new RadioInstance(r));
        });
        
        jScrollPanel_RadioPanel.setViewportView(radioContainer);
    }
    
    public void updateFromState() {
        jTextField_callsign.setText(state.getCallsign());
        jTextField_LAT.setText(Double.toString(state.getLatitutde()));
        jTextField_LON.setText(Double.toString(state.getLongitude()));
        jTextField_HGT.setText(Float.toString(state.getHeight()));

        // TODO: maybe we need to check the registered vs the displayed radios:
        //       add missing ones to the pane, remove obsolete ones
        for (int i=0; i < radioContainer.getComponentCount(); i++) {
            RadioInstance comp = (RadioInstance)radioContainer.getComponent(i);
            comp.updateFromState();
        }
    }
    
    public void setInputElemetsEditable(boolean p) {
        jTextField_callsign.setEnabled(p);
        jTextField_LAT.setEnabled(p);
        jTextField_LON.setEnabled(p);
        jTextField_HGT.setEnabled(p);
        jButton_pickLocation.setEnabled(p);
        
        for (int i=0; i < radioContainer.getComponentCount(); i++) {
            RadioInstance ri = (RadioInstance)radioContainer.getComponent(i);
            ri.setInputElemetsEditable(p);
        }
    }

    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jToggleButton_Connect = new javax.swing.JToggleButton();
        jLabel_ConnectionStatus = new javax.swing.JLabel();
        jSeparator2 = new javax.swing.JSeparator();
        jScrollPanel_RadioPanel = new javax.swing.JScrollPane();
        jLabel1 = new javax.swing.JLabel();
        jTextField_callsign = new javax.swing.JTextField();
        jLabel2 = new javax.swing.JLabel();
        jLabel3 = new javax.swing.JLabel();
        jLabel4 = new javax.swing.JLabel();
        jTextField_LAT = new javax.swing.JTextField();
        jTextField_LON = new javax.swing.JTextField();
        jTextField_HGT = new javax.swing.JTextField();
        jButton_pickLocation = new javax.swing.JButton();
        jScrollPane_Statusbar = new javax.swing.JScrollPane();
        jLabel_Statusbar = new javax.swing.JTextField();
        MainMenu = new javax.swing.JMenuBar();
        jMenu1 = new javax.swing.JMenu();
        jMenuItem_AddIdentity = new javax.swing.JMenuItem();
        jMenuItem_SimConnect = new javax.swing.JMenuItem();
        jSeparator1 = new javax.swing.JPopupMenu.Separator();
        jMenuItem_options = new javax.swing.JMenuItem();
        jMenuItem_quit = new javax.swing.JMenuItem();
        jMenu_Help = new javax.swing.JMenu();
        jMenuItem_Help_About = new javax.swing.JMenuItem();
        jMenuItem1 = new javax.swing.JMenuItem();

        setDefaultCloseOperation(javax.swing.WindowConstants.EXIT_ON_CLOSE);
        setTitle(java.util.ResourceBundle.getBundle("project").getString("product")+" v"+java.util.ResourceBundle.getBundle("project").getString("version"));
        setBounds(new java.awt.Rectangle(0, 0, 0, 0));

        jToggleButton_Connect.setText("Connect");
        jToggleButton_Connect.setToolTipText("Toggle sending of UDP packets to mumble plugin");

        jLabel_ConnectionStatus.setBackground(new java.awt.Color(255, 255, 0));
        jLabel_ConnectionStatus.setText(" ");
        jLabel_ConnectionStatus.setToolTipText("Green=All OK; Red=sending disabled or error occured");
        jLabel_ConnectionStatus.setBorder(new javax.swing.border.SoftBevelBorder(javax.swing.border.BevelBorder.RAISED));
        jLabel_ConnectionStatus.setOpaque(true);

        jSeparator2.setForeground(new java.awt.Color(153, 153, 153));
        jSeparator2.setMaximumSize(new java.awt.Dimension(3500, 1));

        jScrollPanel_RadioPanel.setAutoscrolls(true);
        jScrollPanel_RadioPanel.setMinimumSize(new java.awt.Dimension(24, 405));
        jScrollPanel_RadioPanel.setOpaque(false);

        jLabel1.setText("Callsign:");
        jLabel1.setToolTipText("How you are called on the radio");

        jTextField_callsign.setText("XYZ");
        jTextField_callsign.setToolTipText("How you are called on the radio");
        jTextField_callsign.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                jTextField_callsignKeyReleased(evt);
            }
        });

        jLabel2.setText("LAT:");
        jLabel2.setToolTipText("Latitude in decimal degrees (WGS84): \"12.345678\" (south of equator is given negative)");

        jLabel3.setText("LON:");
        jLabel3.setToolTipText("Longitude in decimal degrees (WGS84): \"12.345678\" (west of prime meridian is given negative)");

        jLabel4.setText("HGT:");
        jLabel4.setToolTipText("Height in feet above ground level");

        jTextField_LAT.setText("12.345678");
        jTextField_LAT.setToolTipText("Latitude in decimal degrees (WGS84): \"12.345678\" (south of equator is given negative)");
        jTextField_LAT.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                jTextField_LATKeyReleased(evt);
            }
        });

        jTextField_LON.setText("12.345678");
        jTextField_LON.setToolTipText("Longitude in decimal degrees (WGS84): \"12.345678\" (west of prime meridian is given negative)");
        jTextField_LON.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                jTextField_LONKeyReleased(evt);
            }
        });

        jTextField_HGT.setText("1234");
        jTextField_HGT.setToolTipText("Height in feet above ground level");
        jTextField_HGT.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyReleased(java.awt.event.KeyEvent evt) {
                jTextField_HGTKeyReleased(evt);
            }
        });

        jButton_pickLocation.setText("pick location");
        jButton_pickLocation.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jButton_pickLocationActionPerformed(evt);
            }
        });

        jScrollPane_Statusbar.setVerticalScrollBarPolicy(javax.swing.ScrollPaneConstants.VERTICAL_SCROLLBAR_NEVER);

        jLabel_Statusbar.setEditable(false);
        jLabel_Statusbar.setFont(new java.awt.Font("Monospaced", 0, 10)); // NOI18N
        jLabel_Statusbar.setText("initializing...");
        jLabel_Statusbar.setToolTipText("Shows the UDP sending status and its content");
        jLabel_Statusbar.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jLabel_StatusbarActionPerformed(evt);
            }
        });
        jScrollPane_Statusbar.setViewportView(jLabel_Statusbar);

        jMenu1.setText("RadioGUI");

        jMenuItem_AddIdentity.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_R, java.awt.event.InputEvent.CTRL_DOWN_MASK));
        jMenuItem_AddIdentity.setText("Add new Radio");
        jMenuItem_AddIdentity.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jMenuItem_AddIdentityActionPerformed(evt);
            }
        });
        jMenu1.add(jMenuItem_AddIdentity);

        jMenuItem_SimConnect.setText("Slave to SimConnect");
        jMenuItem_SimConnect.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jMenuItem_SimConnectActionPerformed(evt);
            }
        });
        jMenu1.add(jMenuItem_SimConnect);
        jMenu1.add(jSeparator1);

        jMenuItem_options.setText("Options");
        jMenuItem_options.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jMenuItem_optionsActionPerformed(evt);
            }
        });
        jMenu1.add(jMenuItem_options);

        jMenuItem_quit.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_Q, java.awt.event.InputEvent.CTRL_DOWN_MASK));
        jMenuItem_quit.setText("Quit");
        jMenuItem_quit.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jMenuItem_quitActionPerformed(evt);
            }
        });
        jMenu1.add(jMenuItem_quit);

        MainMenu.add(jMenu1);

        jMenu_Help.setText("Help/About");

        jMenuItem_Help_About.setText("Help/About");
        jMenuItem_Help_About.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jMenuItem_Help_AboutActionPerformed(evt);
            }
        });
        jMenu_Help.add(jMenuItem_Help_About);

        jMenuItem1.setText("License");
        jMenuItem1.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                jMenuItem1ActionPerformed(evt);
            }
        });
        jMenu_Help.add(jMenuItem1);

        MainMenu.add(jMenu_Help);

        setJMenuBar(MainMenu);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jSeparator2, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jScrollPanel_RadioPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(layout.createSequentialGroup()
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addGroup(layout.createSequentialGroup()
                                .addComponent(jLabel1)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(jTextField_callsign, javax.swing.GroupLayout.PREFERRED_SIZE, 142, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addGroup(layout.createSequentialGroup()
                                .addComponent(jLabel2)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addComponent(jTextField_LAT, javax.swing.GroupLayout.PREFERRED_SIZE, 142, javax.swing.GroupLayout.PREFERRED_SIZE)))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(layout.createSequentialGroup()
                                .addComponent(jLabel3)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(jTextField_LON, javax.swing.GroupLayout.PREFERRED_SIZE, 144, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(jLabel4)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(jTextField_HGT, javax.swing.GroupLayout.PREFERRED_SIZE, 86, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addComponent(jButton_pickLocation))
                        .addGap(0, 102, Short.MAX_VALUE))
                    .addGroup(layout.createSequentialGroup()
                        .addComponent(jToggleButton_Connect)
                        .addGap(2, 2, 2)
                        .addComponent(jLabel_ConnectionStatus, javax.swing.GroupLayout.PREFERRED_SIZE, 13, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(3, 3, 3)
                        .addComponent(jScrollPane_Statusbar)))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addGap(3, 3, 3)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel1)
                    .addComponent(jTextField_callsign, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(jButton_pickLocation))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel2)
                    .addComponent(jTextField_LAT, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(jLabel3)
                    .addComponent(jTextField_LON, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(jLabel4)
                    .addComponent(jTextField_HGT, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jScrollPanel_RadioPanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(jSeparator2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(jToggleButton_Connect)
                        .addComponent(jLabel_ConnectionStatus, javax.swing.GroupLayout.PREFERRED_SIZE, 13, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addComponent(jScrollPane_Statusbar, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void jMenuItem_quitActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jMenuItem_quitActionPerformed
        this.dispose();
    }//GEN-LAST:event_jMenuItem_quitActionPerformed

    
    private void jMenuItem_optionsActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jMenuItem_optionsActionPerformed
        optionsWindow.setVisible(true);
    }//GEN-LAST:event_jMenuItem_optionsActionPerformed

    private void jMenuItem_Help_AboutActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jMenuItem_Help_AboutActionPerformed
        helpWindow.setVisible(true);
    }//GEN-LAST:event_jMenuItem_Help_AboutActionPerformed

    private void jMenuItem1ActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jMenuItem1ActionPerformed
        licenseWindow.setVisible(true);
    }//GEN-LAST:event_jMenuItem1ActionPerformed

    private void jMenuItem_AddIdentityActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jMenuItem_AddIdentityActionPerformed
        Radio r = new Radio();
        state.getRadios().add(r);
        radioContainer.add(new RadioInstance(r));
    }//GEN-LAST:event_jMenuItem_AddIdentityActionPerformed

    private void jTextField_callsignKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_jTextField_callsignKeyReleased
       state.setCallsign(jTextField_callsign.getText());
    }//GEN-LAST:event_jTextField_callsignKeyReleased

    private void jTextField_LATKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_jTextField_LATKeyReleased
        try {
            state.setLatitude(Double.valueOf(jTextField_LAT.getText()));
        } catch( java.lang.NumberFormatException e) {
            jTextField_LAT.setText(Double.toString(state.getLatitutde()));
        }
    }//GEN-LAST:event_jTextField_LATKeyReleased

    private void jTextField_LONKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_jTextField_LONKeyReleased
        try {
            state.setLongitude(Double.valueOf(jTextField_LON.getText()));
        } catch( java.lang.NumberFormatException e) {
            jTextField_LON.setText(Double.toString(state.getLongitude()));
        }
    }//GEN-LAST:event_jTextField_LONKeyReleased

    private void jTextField_HGTKeyReleased(java.awt.event.KeyEvent evt) {//GEN-FIRST:event_jTextField_HGTKeyReleased
        try {
            state.setHeight(Float.valueOf(jTextField_HGT.getText()));
        } catch( java.lang.NumberFormatException e) {
            jTextField_HGT.setText(Float.toString(state.getHeight()));
        }
    }//GEN-LAST:event_jTextField_HGTKeyReleased

    private void jLabel_StatusbarActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jLabel_StatusbarActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_jLabel_StatusbarActionPerformed

    private void jButton_pickLocationActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jButton_pickLocationActionPerformed
        MapWindow locationSelector = new MapWindow(state, this);
    }//GEN-LAST:event_jButton_pickLocationActionPerformed

    private void jMenuItem_SimConnectActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_jMenuItem_SimConnectActionPerformed
        radioGUI.enableSimConnect();
    }//GEN-LAST:event_jMenuItem_SimConnectActionPerformed

    
    /**
     * Prepares GUI state for slaving trough SimConnect
     */
    public void prepareSimConnect() {
        jToggleButton_Connect.setSelected(false); // disable sending
        
        // clean radio Instances from the JPane scrollview
        for (int i=0; i < radioContainer.getComponentCount(); i++) {
            RadioInstance ri = (RadioInstance) radioContainer.getComponent(i);
            ri.setVisible(false);
            ri.dispose();
        }
        
        radioContainer.repaint();
        jScrollPanel_RadioPanel.repaint();
       
        
        /* Prepare exactly two fresh radios for updates by SimConnect */
        state.getRadios().forEach(r -> {
            RadioInstance ri = new RadioInstance(r);
            ri.setClosable(false);
            radioContainer.add(ri);
        });
        radioContainer.repaint();
        
        
        // update main window
        updateFromState();
        
        // lock input elements
        setInputElemetsEditable(false);
        radioGUI.mainWindow.jMenuItem_AddIdentity.setEnabled(false); // prevent adding radios
        radioGUI.mainWindow.jMenuItem_SimConnect.setEnabled(false);  // prevent reestablishing
        //radioGUI.mainWindow.jToggleButton_Connect.setEnabled(false); // prevent "connect"
        optionsWindow.prepareSimConnect();
    
    }
    
    /**
     * Get state of connection button (should we send?)
     */
    public boolean getConnectionActivation() {
        return this.jToggleButton_Connect.isSelected();
    }
    
    /**
     * Change state of connection button
     */
    public void setConnectionActivation(boolean p) {
        this.jToggleButton_Connect.setSelected(p);
    }
    
    /**
     * Update the connection state
     */
    public void setConnectionState(boolean state) {
        if (state) {
            this.jLabel_ConnectionStatus.setBackground(Color.green);
        } else {
            this.jLabel_ConnectionStatus.setBackground(Color.red);
        }
    }
    
    /**
     * Update the message status box
     */
    public void setStatusText(String txt) {
        this.jLabel_Statusbar.setText(txt);
    }

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JMenuBar MainMenu;
    private javax.swing.JButton jButton_pickLocation;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel_ConnectionStatus;
    private javax.swing.JTextField jLabel_Statusbar;
    private javax.swing.JMenu jMenu1;
    private javax.swing.JMenuItem jMenuItem1;
    private javax.swing.JMenuItem jMenuItem_AddIdentity;
    private javax.swing.JMenuItem jMenuItem_Help_About;
    private javax.swing.JMenuItem jMenuItem_SimConnect;
    private javax.swing.JMenuItem jMenuItem_options;
    private javax.swing.JMenuItem jMenuItem_quit;
    private javax.swing.JMenu jMenu_Help;
    private javax.swing.JScrollPane jScrollPane_Statusbar;
    private javax.swing.JScrollPane jScrollPanel_RadioPanel;
    private javax.swing.JPopupMenu.Separator jSeparator1;
    private javax.swing.JSeparator jSeparator2;
    private javax.swing.JTextField jTextField_HGT;
    private javax.swing.JTextField jTextField_LAT;
    private javax.swing.JTextField jTextField_LON;
    private javax.swing.JTextField jTextField_callsign;
    private javax.swing.JToggleButton jToggleButton_Connect;
    // End of variables declaration//GEN-END:variables

    private void setFrameIcon(ImageIcon imageIcon) {
        throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
    }
}
