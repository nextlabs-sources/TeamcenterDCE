package com.transcendata.userexits;

//ITI PROVIDES THIS PROGRAM AS IS AND WITH ALL FAULTS. ITI
//SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTY OF MERCHANTABILITY OR
//FITNESS FOR A PARTICULAR USE. ITI DOES NOT WARRANT THAT
//THE OPERATION OF THE PROGRAM WILL BE UNINTERRUPTED OR ERROR FREE.

//Copyright (C) 2015 International TechneGroup Incorporated
//5303 DuPont Circle
//Milford, Ohio 45150, U.S.A.  All Rights Reserved.

import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.Color;
import java.awt.event.FocusEvent;
import java.awt.event.FocusListener;
import java.awt.Container;
import java.awt.Insets;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.FileWriter;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import javax.swing.BorderFactory;
import javax.swing.ButtonGroup;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JTextField;
import java.awt.GridLayout;
import java.awt.Dimension;

import javax.swing.JFrame;
import javax.swing.JOptionPane;
import javax.swing.UIManager;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.apache.xml.serialize.OutputFormat;
import org.apache.xml.serialize.XMLSerializer;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.w3c.dom.Text;

import com.teamcenter.services.strong.core.DataManagementService;
import com.teamcenter.services.strong.core._2006_03.DataManagement.GenerateItemIdsAndInitialRevisionIdsProperties;
import com.teamcenter.services.strong.core._2006_03.DataManagement.GenerateItemIdsAndInitialRevisionIdsResponse;
import com.teamcenter.services.strong.core._2006_03.DataManagement.ItemIdsAndInitialRevisionIds;
import com.teamcenter.services.strong.core._2013_05.DataManagement.GenerateNextValuesIn;
import com.teamcenter.services.strong.core._2013_05.DataManagement.GenerateNextValuesResponse;
import com.teamcenter.services.strong.core._2013_05.DataManagement.GeneratedValue;
import com.teamcenter.services.strong.core._2013_05.DataManagement.GeneratedValuesOutput;
import com.teamcenter.soa.client.Connection;
import com.nextlabs.swrmx.nxlutils.NxlUtils;

public class SwimUserExits {

    /**
     * The String variable to hold the value provided by user in custom Replace Item ID dialog.
     */
    private static String _text = "";
    
    /**
     * The boolean flag to decide whether the value provided by the user has to be assigned as prefix or suffix to old
     * id.
     */
    private static boolean _prefix = true;
    
    /**
     * The boolean flag to decide whether the custom Replace Item ID dialog is closed on OK button click or cancel
     * button click. Set to true if the dialog closed on OK button click.
     */
    private static boolean _status = false;

    /**
     * The Constructor.
     */
    private SwimUserExits() {
    }
    
     /**
     * User exit to validate new <code>CheckInCollection</code> on save. For validation success returns true.
     * For invalid model alerts user with message and returns false.
     * 
     * @param fileName
     *            the String the xml file name in which the Save operation info has been dumped.
     * @return true if validation pass else false on failure.
     * @throws Exception
     */
        public static boolean saveValidation(String fileName) throws Exception {
        /*SolidWorks RMX Section Begin
        * Check if the user is authorized to perform the operation
        * Supported :    action: validation , new ,replace
        *				 opeation: Save, Save All,Save As,Clone,RMB->Save As,RMB->Save As Copy
        * Class : NxlUtils
        * Method : public static boolean nxlPerformAccessCheck(String fileName) throws Exception
        * @param fileName
        * 		the String the xml file name in which the Save operation info has been dumped
        * @return true if the user has the required permission to perform the action else false.
        * @throws Exception.
        */
        boolean authorizeStatus = NxlUtils.nxlPerformAccessCheck(fileName);
        //SolidWorks RMX Section End:
        return authorizeStatus;
    }
    	
	/**
     * The New Item ID user exit method. This method will take save validation xml as input and will evaluate the new
     * item IDs for the model. In return this method will write the new Item id and new model name information to the
     * xml. This xml will be created on the same location as that of the input xml.
     * 
     * @param fileName
     *            the String file name which has been dumped with all models which user wants to check in. must not be
     *            null or empty.
     * @return the string response xml name which has the new Item IDs and the new model name for the models.
     * @throws Exception
     */
	
    public static String saveNew(String fileName) throws Exception {
        String result = "";
		//Section Begin: SolidWorks RMX user authorization check for saveNew operation. 
        boolean authorizeStatus = NxlUtils.nxlPerformAccessCheck(fileName);
        if(!authorizeStatus) {
        	return fileName;
        }
		//Section End: 
        
        File file = new File(fileName);
        if (file.exists()) {
            String validationFileName = System.getProperty("java.io.tmpdir") + File.separator
                    + "txd_save_new.xml";
            File validationFile = new File(validationFileName);
            FileReader fr = null;
            FileWriter fw = null;
            try {
                fr = new FileReader(file);
                fw = new FileWriter(validationFile);
                int c = fr.read();
                while (c != -1) {
                    fw.write(c);
                    c = fr.read();
                }
            } finally {
                fr.close();
                fw.close();
            }
            String itemIDFile = file.getParent() + File.separator + "txd_save_new_response.xml";
            DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
            DocumentBuilder db = dbf.newDocumentBuilder();
            Document dom = db.parse(file);
            Document doc = db.newDocument();
            Element docEle = dom.getDocumentElement();
            NodeList nl = docEle.getElementsByTagName("model");
            if (nl != null && nl.getLength() > 0) {
                List<String> itemTypes = new ArrayList<String>();
                List<String> firstSaveAttributes = new ArrayList<String>();
                List<String> businessObjNames = new ArrayList<String>();
                List<String> namingRulePatterns = new ArrayList<String>();
                List<String> itemRevUids = new ArrayList<String>();
                for (int i = 0; i < nl.getLength(); i++) {
                    Element elem = (Element) nl.item(i);
                    String selectedNodeattr = elem.getAttribute("selected");
                    String firstSaveNodeAttr = elem.getAttribute("first_save");
                    boolean found = false;
                    if (selectedNodeattr.equalsIgnoreCase("yes")) {
                        found = true;
                    }
                    if (found) {
                        NodeList itemTypeNode = elem.getElementsByTagName("item_type");
                        String businessObjName = "";
                        if (itemTypeNode != null && itemTypeNode.getLength() > 0) {
                            Element itemTypeElem = (Element) itemTypeNode.item(0);
                            String itemType = itemTypeElem.getAttribute("value");
                            itemTypes.add(itemType);
                            businessObjName = itemType;
                        }
                        NodeList itemIdNode = elem.getElementsByTagName("item_id");
                        NodeList itemRevIdNode = elem.getElementsByTagName("item_revision_id");
                        if (itemIdNode != null & itemIdNode.getLength() > 0) {
                            String namingRulePattern = "";
                            String itemRevUid = "";
                            Element itemIdElem = (Element) itemIdNode.item(0);
                            NodeList namingRuleNodeList = itemIdElem.getElementsByTagName("naming_rule");
                            if (namingRuleNodeList != null && namingRuleNodeList.getLength() > 0) {
                                Element namingRuleElem = (Element) namingRuleNodeList.item(0);
                                businessObjName = namingRuleElem.getAttribute("business_obj_name");
                                namingRulePattern = namingRuleElem.getAttribute("naming_rule_pattern");
                            }
                            if (itemRevIdNode != null) {
                                Element revIdElem = (Element) itemRevIdNode.item(0);
                                itemRevUid = revIdElem.getAttribute("item_rev_uid");
                            }
                            firstSaveAttributes.add(firstSaveNodeAttr);
                            businessObjNames.add(businessObjName);
                            namingRulePatterns.add(namingRulePattern);
                            itemRevUids.add(itemRevUid);
                        }
                    }
                }

                // Get the new Item IDs from the Teamcenter.
                String[] itemTypesArray = new String[itemTypes.size()];
                itemTypesArray = itemTypes.toArray(itemTypesArray);
                // Get first save, business object, naming rule pattern and item revision UID information
                String[] firstSaveAttrArr = new String[itemTypes.size()];
                firstSaveAttrArr = firstSaveAttributes.toArray(firstSaveAttrArr);
                String[] businessObjNamesArr = new String[itemTypes.size()];
                businessObjNamesArr = businessObjNames.toArray(businessObjNamesArr);
                String[] itemIdNamingRulePatterArr = new String[itemTypes.size()];
                itemIdNamingRulePatterArr = namingRulePatterns.toArray(itemIdNamingRulePatterArr);
                String[] itemRevUidsArr = new String[itemTypes.size()];
                itemRevUidsArr = itemRevUids.toArray(itemRevUidsArr);
                // Get the new Item IDs from the Teamcenter with given connection object
                Connection conn = UserExitHelper.getSOAConnection();
                String[] newIDs = getNextPropValueUsingNamingRule(conn, "item_id", itemTypesArray, firstSaveAttrArr,
                        businessObjNamesArr, itemIdNamingRulePatterArr, itemRevUidsArr);
                
                // Write the new Item ID information to the response xml.
                int count = -1;
                Element resElem = doc.createElement("save_operation_response");
                resElem.setAttribute("action", "new");
                for (int i = 0; i < nl.getLength(); i++) {
                    Element elem = (Element) nl.item(i);
                    String modelName = elem.getAttribute("name");
                    String master = elem.getAttribute("master");
                    String type = elem.getAttribute("type");
                    String selectedNodeattr = elem.getAttribute("selected");
                    String firstSaveNodeAttr = elem.getAttribute("first_save");
                    String canRenameModelStr = elem.getAttribute("can_rename");
                    boolean canRenameModel = (new Boolean(canRenameModelStr)).booleanValue();
                    String itemType = ((Element)elem.getElementsByTagName("item_type").item(0)).getAttribute("value");
                    String itemID = ((Element)elem.getElementsByTagName("item_id").item(0)).getAttribute("value");
                    NodeList propList = elem.getElementsByTagName("properties");
                    boolean found = false;
                    if (selectedNodeattr.equalsIgnoreCase("yes")) {
                        found = true;
                    }
                    if (found) {
                        ++count;
                        Element modelElem = doc.createElement("model");
                        modelElem.setAttribute("name", modelName);
                        modelElem.setAttribute("master", master);
                        modelElem.setAttribute("type", type);
                        // Assigning same, empty or new Item IDs.
                        String valueToSet = newIDs[count];
                        modelElem.setAttribute("new_item_id", valueToSet);
                        if (canRenameModel) {
                            modelElem.setAttribute("new_model_name", valueToSet);
                        }
                        
                        Element properties = doc.createElement("properties");
                        if (propList != null) {
                            Element props = (Element) propList.item(0);
                            NodeList pdmProps = props.getElementsByTagName("pdm_property");
                            for (int j = 0; j < pdmProps.getLength(); j++) {
                                Element pdmProp = (Element) pdmProps.item(j);
                                Element pdmProperty = doc.createElement("pdm_property");
                                String pdmName = pdmProp.getAttribute("name");
                                pdmProperty.setAttribute("name", pdmName);
                                NodeList valuesList = pdmProp.getElementsByTagName("value");
                                if (valuesList != null) {
                                    for (int k = 0; k < valuesList.getLength(); k++) {
                                        String index = ((Element) valuesList.item(k)).getAttribute("index");
                                        String value = ((Element) valuesList.item(k)).getTextContent();
                                                                                
                                        Element propValue = doc.createElement("value");
                                        if (!index.equals("")) {
                                            propValue.setAttribute("index", index);
                                        }
                                        Text node = doc.createTextNode(value);
                                        propValue.appendChild(node);
                                        pdmProperty.appendChild(propValue);
                                        
                                    }
                                }
                                
                                properties.appendChild(pdmProperty);
                            }
                            
                            // Providing Item name
                            Element pdmProperty = doc.createElement("pdm_property");
                            pdmProperty.setAttribute("name", "Item.object_name");
                            // Providing valid/invalid Item Revision value.
                            pdmProperty = doc.createElement("pdm_property");
                            pdmProperty.setAttribute("name", "ItemRevision.item_revision_id");                        
                            modelElem.appendChild(properties);
                        }
                        
                        resElem.appendChild(modelElem);
                    }
                }
                doc.appendChild(resElem);
            }

            FileOutputStream fos = new FileOutputStream(itemIDFile);
            OutputFormat format = new OutputFormat(doc);
            format.setIndenting(true);
            format.setLineWidth(0);

            // Serialize the Document and write to the file.
            XMLSerializer serializer = new XMLSerializer(fos, format);
            Element docElem = doc.getDocumentElement();
            docElem.normalize();
            serializer.asDOMSerializer();
            serializer.serialize(docElem);
            fos.close();
            result = itemIDFile;
        }
        
        /*SolidWorks RMX Section Begin
         * Process the newly written response file with NxlUtils before returning it.
         * Class : NxlUtils
         * Method : public static boolean nxlProcessResponseFile(String fileName) throws Exception
         * @param fileName
         * 		the String the xml file name in which the Save operation info has been dumped
         * @return true if successfully processed else false. 
         * @throws Exception.
         */
        boolean status = NxlUtils.nxlProcessResponseFile(result);
        //Section End:
        return result;
    }
    	
	
	/**
     * Get the next property value from Teamcenter for given property with the help of the Connection object.
     * 
     * <p>
     * This method uses generateNextValues API that supports naming rule pattern to generate next values.
     * 
     * @param conn
     *            the Tamcenter connection Object.
     * @param itemTypesArray
     *            A string array of the Item Types.
     * @return the string array of new item Ids
     */
    private static String[] getNextPropValueUsingNamingRule(Connection conn, String propertyName,
            String[] itemTypesArray, String[] firstSaveAttrArr, String[] businessObjNamesArr,
            String[] itemNameRulePatternsArr, String[] itemRevUidsArr) {
        String[] itemIds = new String[itemTypesArray.length];
        ArrayList<GenerateNextValuesIn> nextValueInputList = new ArrayList<GenerateNextValuesIn>();
        for (int i = 0; i < itemTypesArray.length; i++) {
            GenerateNextValuesIn genNextItemIdValuesIn = new GenerateNextValuesIn();
            genNextItemIdValuesIn.clientId = Integer.toString(i);
            genNextItemIdValuesIn.operationType = 1;
            if (firstSaveAttrArr[i].equalsIgnoreCase("no")) {
                genNextItemIdValuesIn.operationType = 3;
            }
            genNextItemIdValuesIn.businessObjectName = businessObjNamesArr[i];
            genNextItemIdValuesIn.propertyNameWithSelectedPattern.put(propertyName, itemNameRulePatternsArr[i]);
            if (firstSaveAttrArr[i].equalsIgnoreCase("no") && itemRevUidsArr[i] != null) {
                genNextItemIdValuesIn.additionalInputParams.put("sourceObject", itemRevUidsArr[i]);
            }
            nextValueInputList.add(genNextItemIdValuesIn);
        }
        if (nextValueInputList.size() > 0) {
            GenerateNextValuesIn[] nextValueInputs = new GenerateNextValuesIn[nextValueInputList.size()];
            nextValueInputs = nextValueInputList.toArray(nextValueInputs);
            DataManagementService dmService = DataManagementService.getService(conn);
            GenerateNextValuesResponse resp = dmService.generateNextValues(nextValueInputs);
            GeneratedValuesOutput[] output = resp.generatedValues;

            for (int i = 0; i < output.length; i++) {
                Map<String, GeneratedValue> valuesMap = output[i].generatedValues;
                GeneratedValue value = valuesMap.get(propertyName);
                itemIds[i] = value.nextValue;
            }
        }
        return itemIds;
    }
	
	/**
     * The Replace Item ID user exit method. This method will take save validation xml as input and will will launch the
     * custom replace item ID dialog. In return this method will write the new Item id and new model name information to
     * the xml with the help of input provided in custom dialog. This xml will be created on the same location as that
     * of the input xml.
     * 
     * @param fileName
     *            the String file name which has been dumped with all models which user wants to check in. must not be
     *            null or empty.
     * @return the string response xml name which has the new Item IDs and the new model name for the models.
     * @throws Exception
     */
   
    public static String saveReplace(String fileName) throws Exception {
        String result = "";
		//Section Begin: SolidWorks RMX user authorization check for saveReplace operation.
        boolean authorizeStatus = NxlUtils.nxlPerformAccessCheck(fileName);
        if(!authorizeStatus) {
        	return fileName;
        }
		//Section End: 
        
        File file = new File(fileName);
        if(file.exists()) {
            String validationFileName = System.getProperty("java.io.tmpdir") + File.separator
                    + "txd_save_replace.xml";
            File validationFile = new File(validationFileName);
            FileReader fr = null;
            FileWriter fw = null;
            try {
                fr = new FileReader(file);
                fw = new FileWriter(validationFile);
                int c = fr.read();
                while (c != -1) {
                    fw.write(c);
                    c = fr.read();
                }
            } finally {
                fr.close();
                fw.close();
            }
            
            // Create and display custom Replace Item ID dialog.
            displayReplaceIDDialog();
            // If user selects "OK" in the custom Replace Item ID dialog, then only go ahead for writing xml.
            if (_status) {
                String val = _text;
                String itemIDFile = file.getParent() + File.separator + "txd_save_replace_response.xml";
                DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
                DocumentBuilder db = dbf.newDocumentBuilder();
                Document dom = db.parse(file);
                Document doc = db.newDocument();
                Element docEle = dom.getDocumentElement();
                NodeList nl = docEle.getElementsByTagName("model");
                // Write the new Item ID information to the response xml.
                if (nl != null && nl.getLength() > 0) {
                    Element resElem = doc.createElement("save_operation_response");
                    resElem.setAttribute("action", "replace");
                    for (int i = 0; i < nl.getLength(); i++) {
                        Element elem = (Element) nl.item(i);
                        String selectedNodeattr = elem.getAttribute("selected");
                        NodeList propList = elem.getElementsByTagName("properties");
                        boolean found = false;
                        if (selectedNodeattr.equalsIgnoreCase("yes")) {
                            found = true;
                        }

                        if (found) {
                            String modelName = elem.getAttribute("name");
                            String master = elem.getAttribute("master");
                            String type = elem.getAttribute("type");
                            String canRenameModelStr = elem.getAttribute("can_rename");
                            Boolean canRenameModel = new Boolean(canRenameModelStr);
                            String oldItemID = "";
                            String itemType = ((Element)elem.getElementsByTagName("item_type").item(0)).getAttribute("value");
                            NodeList itemIDNode = elem.getElementsByTagName("item_id");
                            if (itemIDNode != null && itemIDNode.getLength() > 0) {
                                Element itemIDElem = (Element) itemIDNode.item(0);
                                oldItemID = itemIDElem.getAttribute("value");
                            }

                            Element modelElem = doc.createElement("model");
                            modelElem.setAttribute("name", modelName);
                            modelElem.setAttribute("master", master);
                            modelElem.setAttribute("type", type);
                            String newItemID = val + oldItemID;
                            if (!_prefix) {
                                newItemID = oldItemID + val;
                            }
                            
                            // Assigning same, empty or new Item IDs.
                            String valueToSet = newItemID;
                            modelElem.setAttribute("new_item_id", valueToSet);
                             if (canRenameModel) {
                                modelElem.setAttribute("new_model_name", valueToSet);
                            }
                            Element properties = doc.createElement("properties");
                            if (propList != null) {
                                Element props = (Element) propList.item(0);
                                NodeList pdmProps = props.getElementsByTagName("pdm_property");
                                for (int j = 0; j < pdmProps.getLength(); j++) {
                                    Element pdmProp = (Element) pdmProps.item(j);
                                    Element pdmProperty = doc.createElement("pdm_property");
                                    String pdmName = pdmProp.getAttribute("name");
                                    pdmProperty.setAttribute("name", pdmName);
                                    NodeList valuesList = pdmProp.getElementsByTagName("value");
                                    if (valuesList != null) {
                                        for (int k = 0; k < valuesList.getLength(); k++) {
                                            String index = ((Element) valuesList.item(k)).getAttribute("index");
                                            String value = ((Element) valuesList.item(k)).getTextContent();
                                            
                                            Element propValue = doc.createElement("value");
                                            if (!index.equals("")) {
                                                propValue.setAttribute("index", index);
                                            }
                                            Text node = doc.createTextNode(value);
                                            propValue.appendChild(node);
                                            pdmProperty.appendChild(propValue);
                                            
                                        }
                                    }
                                    
                                    properties.appendChild(pdmProperty);
                                } 
                                
                                // Providing Item name
                                Element pdmProperty = doc.createElement("pdm_property");
                                pdmProperty.setAttribute("name", "Item.object_name");
                            
                                // Providing valid/invalid Item Revision value.
                                pdmProperty = doc.createElement("pdm_property");
                                pdmProperty.setAttribute("name", "ItemRevision.item_revision_id");
                                modelElem.appendChild(properties);
                            }
                            resElem.appendChild(modelElem);
                        }
                    }
                    doc.appendChild(resElem);
                }

                FileOutputStream fos = new FileOutputStream(itemIDFile);
                OutputFormat format = new OutputFormat(doc);
                format.setIndenting(true);
                format.setLineWidth(0);

                // Serialize the Document and write to the file.
                XMLSerializer serializer = new XMLSerializer(fos, format);
                Element docElem = doc.getDocumentElement();
                docElem.normalize();
                serializer.asDOMSerializer();
                serializer.serialize(docElem);
                fos.close();
                result = itemIDFile;
            }
        }
        /*SolidWorks RMX Section Begin
         * Process the newly written response file with NxlUtils before returning it.
         * Class : NxlUtils
         * Method : public static boolean nxlProcessResponseFile(String fileName) throws Exception
         * @param fileName
         * 		the String the xml file name in which the Save operation info has been dumped
         * @return true if successfully processed else false. 
         * @throws Exception.
         */
        boolean status = NxlUtils.nxlProcessResponseFile(result);
      //Section End:
        return result;
    }
    
	 /**
     * Display the custom Replace Item ID dialog and get the input provided by the user.
     * 
     * @throws Exception
     */
    private static void displayReplaceIDDialog() throws Exception {
        final JFrame parent = new JFrame();
        // Force user exit dialogs/alerts on top of integration dialogs
        parent.setAlwaysOnTop(true);
        parent.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);

        final JDialog dialog = new JDialog(parent, true);
        dialog.setSize(400, 200);
        dialog.setTitle("Replace Item ID");
        dialog.setDefaultCloseOperation(JDialog.DISPOSE_ON_CLOSE);
        // Force user exit dialogs/alerts on top of integration dialogs
        dialog.setAlwaysOnTop(true);

        JPanel radioPanel = new JPanel(new GridLayout());
        radioPanel.setBorder(BorderFactory.createTitledBorder("Replace Item ID"));
        GridBagConstraints radiogbc = new GridBagConstraints();
        radiogbc.insets = new Insets(0, 10, 0, 10);

        final JRadioButton prefBtn = new JRadioButton("Add Prefix");
        final JRadioButton suffBtn = new JRadioButton("Add Suffix");
        ButtonGroup grp = new ButtonGroup();
        grp.add(prefBtn);
        grp.add(suffBtn);
        prefBtn.setSelected(true);
        radiogbc.gridx = 0;
        radiogbc.gridy = 0;
        radiogbc.anchor = GridBagConstraints.NORTHWEST;
        radioPanel.add(prefBtn, radiogbc);

        radiogbc.gridy = 1;
        radiogbc.weightx = 1.0;
        radioPanel.add(suffBtn, radiogbc);

        JPanel idPanel = new JPanel(new GridLayout());
        idPanel.setBorder(BorderFactory.createTitledBorder("Input Information"));
        GridBagConstraints idgbc = new GridBagConstraints();
        idgbc.insets = new Insets(0, 4, 0, 4);

        final JLabel label = new JLabel("Add Prefix");
        idgbc.gridx = 0;
        idgbc.gridy = 0;
        idgbc.anchor = GridBagConstraints.NORTHWEST;
        idPanel.add(label, idgbc);

        final JTextField textfield = new JTextField();
        textfield.setEditable(true);
        textfield.setText("");
        textfield.setForeground(Color.GRAY);
        textfield.setHorizontalAlignment(JTextField.LEFT);
        textfield.setColumns(5);
        textfield.setMinimumSize(new Dimension(50, textfield.getPreferredSize().height));
        idgbc.gridy = 1;
        idgbc.fill = GridBagConstraints.HORIZONTAL;
        idgbc.weightx = 1.0;
        idgbc.weighty = 1.0;
        idPanel.add(textfield, idgbc);

        JPanel btnPanel = new JPanel(new GridBagLayout());
        GridBagConstraints btngbc = new GridBagConstraints();
        btngbc.insets = new Insets(0, 5, 0, 5);

        final JButton okBtn = new JButton("OK");
        btngbc.gridx = 0;
        btngbc.gridy = 0;
        btngbc.anchor = GridBagConstraints.CENTER;
        btngbc.insets = new Insets(0, 2, 0, 2);
        btnPanel.add(okBtn, btngbc);

        final JButton cancelBtn = new JButton("Cancel");
        btngbc.gridx = 1;
        btnPanel.add(cancelBtn, btngbc);

        textfield.addFocusListener(new FocusListener() {
            public void focusLost(FocusEvent e) {}

            public void focusGained(FocusEvent e) {
                textfield.setForeground(Color.BLACK);
                textfield.setText("");
            }
        });

        prefBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                _prefix = true;
                textfield.setForeground(Color.GRAY);
                label.setText("Add Prefix");
            }
        });

        suffBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                _prefix = false;
                textfield.setForeground(Color.GRAY);
                label.setText("Add Suffix");
            }
        });

        okBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                dialog.setVisible(false);
                if (prefBtn.isSelected()) {
                    _prefix = true;
                } else {
                    _prefix = false;
                }
                _status = true;
                _text = textfield.getText();
                dialog.dispose();
                parent.dispose();
            }
        });

        cancelBtn.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent e) {
                dialog.setVisible(false);
                _status = false;
                dialog.dispose();
                parent.dispose();
            }
        });

        dialog.addWindowListener(new WindowAdapter() {
            public void windowClosing(WindowEvent windowEvent) {
                _status = false;
                dialog.dispose();
                parent.dispose();
            }
        });

        Container cp = dialog.getContentPane();
        cp.setLayout(new GridBagLayout());
        GridBagConstraints gbc = new GridBagConstraints();

        // Add radio Panel
        gbc.gridx = 0;
        gbc.gridy = 0;
        gbc.anchor = GridBagConstraints.CENTER;
        gbc.fill = GridBagConstraints.HORIZONTAL;
        gbc.weightx = 1.0;
        gbc.insets = new Insets(5, 0, 5, 0);
        cp.add(radioPanel, gbc);

        // Add Item ID Panel
        gbc.gridy = 1;
        cp.add(idPanel, gbc);

        // Add Button Panel
        gbc.gridy = 2;
        gbc.fill = GridBagConstraints.NONE;
        gbc.weightx = 0.0;
        gbc.insets = new Insets(5, 0, 0, 0);
        cp.add(btnPanel, gbc);
        dialog.setVisible(true);
    }
    
}
