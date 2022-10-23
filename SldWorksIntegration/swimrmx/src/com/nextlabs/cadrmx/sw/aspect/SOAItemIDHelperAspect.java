/*
 * Created on September 17, 2020
 *
 * All sources, binaries and HTML pages (C) copyright 2020 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */
package com.nextlabs.cadrmx.sw.aspect;

import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Map;
import java.util.HashMap;
import java.awt.Component;
import java.io.File;
import java.io.FileOutputStream;
import java.io.FileWriter;

import java.nio.file.Path;
import java.nio.file.Paths;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.apache.xml.serialize.OutputFormat;
import org.apache.xml.serialize.XMLSerializer;
import org.w3c.dom.Attr;
import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;
import org.w3c.dom.Text;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.reflect.MethodSignature;

import com.transcendata.cadpdm.CADFile;
import com.transcendata.cadpdm.CADIdentifier;
import com.transcendata.cadpdm.CADOperationHandler;
import com.transcendata.cadpdm.OperationElement;
import com.transcendata.cadpdm.ServiceManager;
import com.transcendata.util.StringUtil;
import com.transcendata.util.UserCancelException;

import com.nextlabs.cadrmx.sw.SwimRMXSession;
import com.nextlabs.cadrmx.sw.SwRMXResult;
import com.nextlabs.cadrmx.sw.SwRMXStatus;
import com.nextlabs.cadrmx.util.RMXLogHolder;

@Aspect
public class SOAItemIDHelperAspect {
	
	private static final String ERR_MSG_SAVEAS_DENY = "Operation Denied. You do not have 'Save As/Edit' permission to save as the following model(s). \nTo continue, return to the dialog and unselect them:\n";
	
	//protected void updateIDs(OperationElement[] elems, String[] newIDs, String[] newRevIDs, String[] newModelNames, Component cpt)
	@Around("execution(protected void com.transcendata.cadpdm.soa.gui.SOAItemIDHelper.updateIDs(..)) " +
	"&& !within(com.nextlabs.cadrmx.sw.aspect.SOAItemIDHelperAspect)")
	public void around_updateIDs(ProceedingJoinPoint jp) throws Throwable, UserCancelException{
		RMXLogHolder.enteringAspect("around_SOAItemIDHelper.updateIDs");

		if (!SwimRMXSession.isRMXRunning()) {		
			RMXLogHolder.error("Ignore to check right. SwimRMX not initialized");
			jp.proceed();
			return;
		}
		Map<String, String> newModels = new HashMap<>();
		OperationElement[] elems = null;
		String[] newNames = null;
		String[] newIDs = null;
		ArrayList<String> denyModels = new ArrayList();
		
		String[] argNames = ((MethodSignature) jp.getSignature()).getParameterNames();
		Object[] values = jp.getArgs();
		
        if (argNames.length != 0) {
            for (int i = 0; i < argNames.length; i++) {
                if (argNames[i].equals("elems")) {
                	elems = (OperationElement[])values[i];
                } else if (argNames[i].equals("newIDs")) {
                	newIDs = (String[])values[i];
                } else if (argNames[i].equals("newModelNames")) {
                	newNames = (String[])values[i];
                }
            }
        }
	
        Document doc = null;
		if (elems != null ) {
			try {
				DocumentBuilderFactory dbf = DocumentBuilderFactory.newInstance();
				DocumentBuilder db = dbf.newDocumentBuilder();
				doc = db.newDocument();
				Element resElem = doc.createElement("saveas_operation_response");
				resElem.setAttribute("action", "new");
			
				for(int i = 0; i < elems.length; i++) {
					OperationElement elem = elems[i];
					CADFile cadFile = elem.getCADFile();
					SwRMXResult ret = SwimRMXSession.checkSaveAsRight(cadFile.getPath());
					if (!ret.getOperationStatus()) {
						denyModels.add(elem.getCADIdentifier().getCADIdForMessage());		
					}
					
					String newName = null;
					if (newNames != null && newNames[i] != null) {
						newName = newNames[i];
					} else if (newIDs != null && newIDs[i] != null) {
						newName = newIDs[i];
					}
					
					CADIdentifier cadID = elem.getCADIdentifier();
					RMXLogHolder.debug("Source: " + cadID.getCADIdForMessage() + ", new_model_name" + newName);
					Element modelElem = doc.createElement("model");
					modelElem.setAttribute("name", cadID.getCADName());
					modelElem.setAttribute("type", cadID.getCADType().toString());
					modelElem.setAttribute("new_model_name", newName);
					resElem.appendChild(modelElem);
				}
				
				doc.appendChild(resElem);
				
			} catch (Exception ex) {
				RMXLogHolder.error("Exception occurs during generating saveasfile.xml", ex);
			}
		}

		if (denyModels.size() == 0) {
			RMXLogHolder.debug("Proceeding SOAItemIDHelper.updateIDs...");
			
			if (doc != null) {
				String filename = StringUtil.appendCurrentDateTime("saveasfiles", "_", false) + ".xml";
				String itemIDFile = Paths.get(SwimRMXSession.getRMXTempDir(), filename).toString();
				try {
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
					
					RMXLogHolder.info(itemIDFile + " generated");
					SwimRMXSession.sendSaveAsResponse(itemIDFile);
				}
				catch (Exception ex) {
					RMXLogHolder.error("Exception occurs during generating saveasfile.xml", ex);
				}
			}
			
			jp.proceed();
			
			RMXLogHolder.exitAspect("around_SOAItemIDHelper.updateIDs");
		}
		else {
			String errMsg = ERR_MSG_SAVEAS_DENY + denyModels;	
			RMXLogHolder.error("Ignore to proceed SOAItemIDHelper.updateIDs( error: " +  errMsg + " )");
			SwimRMXSession.promptAlertDialog(errMsg);
			throw new UserCancelException();
		}
	}
}
