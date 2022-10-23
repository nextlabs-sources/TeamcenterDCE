/*
 * Created on 28 Jan, 2021
 *
 * All sources, binaries and HTML pages (C) copyright 2021 by NextLabs Inc.,
 * San Mateo CA, Ownership remains with NextLabs Inc, All rights reserved
 * worldwide.
 */
package com.nextlabs.ipemrmx;

import java.io.File;
import java.io.IOException;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.nio.file.Paths;
import java.util.Collection;
import java.util.Iterator;
import java.util.Arrays;
import java.util.List;
import java.util.Map;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;

import com.nextlabs.ipemrmx.RMXLogHolder;
import com.nextlabs.ipemrmx.IpemRMXBase;
import com.nextlabs.ipemrmx.IpemRMXSession;

import com.transcendata.util.FileUtil;
import com.transcendata.util.ObjectUtil;
import com.transcendata.util.LogHolder;
import com.transcendata.cadpdm.CADFile;
import com.transcendata.cadpdm.CADIdentifier;
import com.transcendata.cadpdm.CADPDMException;
import com.transcendata.cadpdm.OperationCollection;
import com.transcendata.cadpdm.OperationElement;
import com.transcendata.cadpdm.ServiceManagerHolder;
import com.transcendata.cadpdm.pe.PEModelHelper;
import com.transcendata.cadpdm.OperationType;

import com.teamcenter.soa.client.model.strong.ImanFile;


/**
 * @author tcadmin
 *
 */
@Aspect
public class SOAFileExporterAspect extends IpemRMXBase {
	private String getPlainFileName(String nxlFileName) {
		String[] ne = FileUtil.pathAndExtension(nxlFileName);
		return ne[0];
	}
	
	private void fixNxlExtAfterDownload (OperationCollection coll, Map fileMap, Object dataInst) throws Throwable{
		IpemRMXSession rmxSess = getRMXSession();
		if (rmxSess == null) {
			return;
		}
	
		Field private_imanFiles = dataInst.getClass().getDeclaredField("imanFiles");
		private_imanFiles.setAccessible(true);
		List imanFiles = (List) private_imanFiles.get(dataInst);
		Iterator<ImanFile> imanFileIter = imanFiles.iterator();
		while (imanFileIter.hasNext()) {
			ImanFile obj = imanFileIter.next();
			CADIdentifier id = (CADIdentifier)fileMap.get(obj);
			if (id != null) {
				OperationElement elem = coll.get(id);
				if (elem != null) {
					CADFile cadFile = elem.getCADFile();
					if (cadFile != null && cadFile.getFile().exists() &&
							!cadFile.getPath().endsWith(".nxl")) {
						rmxSess.fixNxlExt(cadFile.getPath());
					}
				}	
			}
		}
	}
	
	// public boolean export(Collection<CADIdentifier> completed) throws CADPDMException
	@Around("execution(private boolean com.transcendata.cadpdm.soa.SOAFileExporter.downloadFiles(..)) "
			+ "&& !within(com.nextlabs.ipemrmx.SOAFileExporterAspect)")
	public boolean around_downloadFiles(ProceedingJoinPoint jp) throws CADPDMException, Throwable {
		RMXLogHolder.enteringAspect("around_SOAFileExporter.downloadFiles");
	
		boolean result = (boolean) jp.proceed();
		IpemRMXSession rmxSess = getRMXSession();
		if (rmxSess == null) {
			RMXLogHolder.error("Ignore to handle export. IpemRMX not initialized");
			RMXLogHolder.exitAspect("around_SOAFileExporter.downloadFiles");
			return result;
		}

		Object instance = jp.getThis();
		Field private_coll = instance.getClass().getDeclaredField("_collection");
		private_coll.setAccessible(true);
		OperationCollection _collection = (OperationCollection) private_coll.get(instance);
		if (_collection == null) {
			RMXLogHolder.exitAspect("around_SOAFileExporter.downloadFiles");
			return result;
		}

		Field private_fileMap = instance.getClass().getDeclaredField("_fileMap");
		private_fileMap.setAccessible(true);
		Map _fileMap = (Map) private_fileMap.get(instance);
		if (_fileMap == null) {
			RMXLogHolder.exitAspect("around_SOAFileExporter.downloadFiles");
			return result;
		}

		Object[] objs = jp.getArgs();
		if (objs == null || objs.length < 1) {
			RMXLogHolder.exitAspect("around_SOAFileExporter.downloadFiles");
			return result;
		}
		
		if (!rmxSess.isJTDispEnabled()) {
			RMXLogHolder.debug("Skip. IPEMRMX_JTDISPATCH not set to 1");
			RMXLogHolder.exitAspect("around_SOAFileExporter.downloadFiles");
			//
			// Append .nxl extension in case it's lost
			//
			fixNxlExtAfterDownload(_collection, _fileMap, objs[0]);
			
			return result;
		}	
		
		if (!ServiceManagerHolder.get().getOperationType().equals(OperationType.ETS) ||
				ServiceManagerHolder.get().isGraphical()) {
			RMXLogHolder.exitAspect("around_SOAFileExporter.downloadFiles");
			return result;
		}

		String rmxDir = System.getenv("CADRMX_DIR");
		if (rmxDir == null || rmxDir.length() ==0) {
			RMXLogHolder.error("CADRMX_DIR env var not defined");
			RMXLogHolder.exitAspect("around_SOAFileExporter.downloadFiles");
			return result;
		}	
		File scriptFile = Paths.get(rmxDir, "transdecrypt.bat").toFile();
		
		boolean isDone = false;
		
		Object dataInst = objs[0];
		Field private_imanFiles = dataInst.getClass().getDeclaredField("imanFiles");
		private_imanFiles.setAccessible(true);
		List imanFiles = (List) private_imanFiles.get(dataInst);
		Iterator<ImanFile> imanFileIter = imanFiles.iterator();
		while (imanFileIter.hasNext()) {
			ImanFile obj = imanFileIter.next();
			CADIdentifier id = (CADIdentifier)_fileMap.get(obj);
			if (id != null) {
				OperationElement elem = _collection.get(id);
				if (elem != null) {
					CADFile cadFile = elem.getCADFile();
					if (cadFile != null && cadFile.getFile().exists()) {
						File inDir = Paths.get(cadFile.getParent(), "nxlfiles").toFile();
						if (inDir.exists()) {
							isDone = true;
							RMXLogHolder.debug("Files already decrypted. Ignore");
						} else if (cadFile.getPath().endsWith(".nxl")) {
							isDone = true;
							// Decrypt only if file is in UNC folder					
							if (rmxSess.isShareFolder(cadFile.getFile())) {
								String decryptedFilePath = getPlainFileName(cadFile.getPath());
								File decryptedFile = new File(decryptedFilePath);
								
								if (!scriptFile.exists()) {
									RMXLogHolder.error(scriptFile.getAbsolutePath() + " not found");
									RMXLogHolder.exitAspect("around_SOAFileExporter.downloadFiles");
									return result;
								}
								String cmd = "\"" + scriptFile.getAbsolutePath() + "\" \"" + cadFile.getParent() + "\"";
								
								try {
									// create a new process
									RMXLogHolder.debug("Executing decrypt command: " + cmd);
									Process p = Runtime.getRuntime().exec(cmd);
									// cause this process to stop until process p is terminated
									p.waitFor();
									
									// when you manually close notepad.exe program will continue here
									RMXLogHolder.debug("Decrypt command completed.");
									
								} catch (IOException e) {
									RMXLogHolder.error("Exception occurs when executing command", e);
								}

							} else {
								RMXLogHolder.debug("File not in UNC folder or network drive. Ignore to handle");
							}

						}
					}
				}
			}

		if (isDone) {
			break; 
		}
	} 

	RMXLogHolder.exitAspect("around_SOAFileExporter.downloadFiles");

	return result;
}
// public boolean export(Collection<CADIdentifier> completed) throws
// CADPDMException
/*@Around("execution(public boolean com.transcendata.cadpdm.soa.SOAFileExporter.export(..)) "
			+ "&& !within(com.nextlabs.ipemrmx.SOAFileExporterAspect)")
	public boolean around_export(ProceedingJoinPoint jp) throws CADPDMException, Throwable {
		RMXLogHolder.enteringAspect("around_SOAFileExporter.export");

		IpemRMXSession rmxSess = getRMXSession();
		if (rmxSess == null) {
			RMXLogHolder.error("Ignore to handle export. IpemRMX not initialized");
		}

		boolean result = (boolean) jp.proceed();
		Object[] objs = jp.getArgs();

		Collection<CADIdentifier> completed = (Collection<CADIdentifier>) objs[0];
		Object instance = jp.getThis();
		Field private_coll = instance.getClass().getDeclaredField("_collection");
		private_coll.setAccessible(true);
		OperationCollection _collection = (OperationCollection) private_coll.get(instance);
		String rmxDir = System.getenv("CADRMX_DIR");
		String scriptFile = Paths.get(rmxDir, "decrypt.bat").toString();

		OperationElement elem = _collection.get((CADIdentifier)completed.toArray()[0]);

		CADFile cadFile = elem.getCADFile();
		File inDir = Paths.get(cadFile.getParent(), "nxlfiles").toFile();
		if (inDir.exists()) {
			RMXLogHolder.debug("Files already decrypted. Ignore");
		} else {
			//RMXLogHolder.info("Exported file id: " + id.toString() + " , filename: " + cadFile.getPath());

			if (!cadFile.getFile().exists()) {
				RMXLogHolder.debug(cadFile.getFile() + " not existed. Ignore.");
			} else if (cadFile.getPath().endsWith(".nxl")) {
				// Decrypt only if file is in UNC folder
				if (rmxSess.isShareFolder(cadFile.getFile())) {
					String decryptedFilePath = getPlainFileName(cadFile.getPath());
					File decryptedFile = new File(decryptedFilePath);

					String cmd = "\"" + scriptFile + "\" \"" + cadFile.getParent() + "\"";

					try {

						// create a new process
						RMXLogHolder.debug("Executing decrypt command: " + cmd);
						Process p = Runtime.getRuntime().exec(cmd);

						// cause this process to stop until process p is terminated
						p.waitFor();

						// when you manually close notepad.exe program will continue here
						RMXLogHolder.debug("Decrypt command completed. List files in the folder:");
						cmd = "\"" + Paths.get(rmxDir, "fixname.bat").toString() + "\" \"" + cadFile.getParent() + "\"";

						RMXLogHolder.debug("Executing fixname command: " + cmd);
						Process p2 = Runtime.getRuntime().exec(cmd);

						// cause this process to stop until process p is terminated
						p2.waitFor();

						// when you manually close notepad.exe program will continue here
						RMXLogHolder.debug("fixname command completed. List files in the folder:");

						String[] files = cadFile.getParentFile().list();
						for(String fileName : files){
							RMXLogHolder.debug("-" + fileName);
						}

					} catch (IOException e) {
						RMXLogHolder.error("Exception occurs when executing command", e);
					}

				} else {
					RMXLogHolder.debug("File not in UNC folder or network drive. Ignore to handle");
				}

			}
		}
		RMXLogHolder.exitAspect("around_SOAFileExporter.export");

		return result;
	}*/
}
