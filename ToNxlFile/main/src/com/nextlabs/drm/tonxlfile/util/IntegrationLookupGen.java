package com.nextlabs.drm.tonxlfile.util;

/**
 * 
 * All sources, binaries and HTML pages (C) copyright 2004-2019 by NextLabs, Inc.,
 * San Mateo CA, Ownership remains with NextLabs, Inc., All rights reserved
 * worldwide.
 */

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.util.HashMap;
import java.util.Map;

import com.nextlabs.drm.tonxlfile.dto.IntegrationDatasetDTO;

public class IntegrationLookupGen {
	
	public static void main(String[] args) {
		IntegrationDatasetDTO allTypes = new IntegrationDatasetDTO.Builder("AllTypes")
		.addNamedReference("UGPART")
		.addNamedReference("ALSG")
		.addNamedReference("Text")
		.addNamedReference("JTPART")
		.addNamedReference("PDF_Reference")
		.addNamedReference("word")
		.addNamedReference("excel")
		.addNamedReference("powerpoint")
		.addNamedReference("Image")
		.addNamedReference("JPEG_Reference")
		.addNamedReference("TIF_Reference")
		.addNamedReference("AsmFile")
		.addNamedReference("DrwFile")
		.addNamedReference("PrtFile")
		.build();

		IntegrationDatasetDTO swAsm = new IntegrationDatasetDTO.Builder("swasm", true)
			.addNamedReference("PNG")
			.addNamedReference("JPEG")
			.build();
		IntegrationDatasetDTO swDrw = new IntegrationDatasetDTO.Builder("swdrw", true)
			.addNamedReference("PNG")
			.addNamedReference("JPEG")
			.build();
		IntegrationDatasetDTO swPrt = new IntegrationDatasetDTO.Builder("swprt", true)
			.addNamedReference("PNG")
			.addNamedReference("JPEG")
			.build();		
		IntegrationDatasetDTO proAsm = new IntegrationDatasetDTO.Builder("proasm", true)
			.addNamedReference("PNG")
			.addNamedReference("JPEG")
			.build();
		IntegrationDatasetDTO proDrw = new IntegrationDatasetDTO.Builder("prodrw", true)
			.addNamedReference("PNG")
			.addNamedReference("JPEG")
			.build();
		IntegrationDatasetDTO proPrt = new IntegrationDatasetDTO.Builder("proprt", true)
			.addNamedReference("PNG")
			.addNamedReference("JPEG")
			.build();
		
		HashMap<String, IntegrationDatasetDTO> integrationLookup = new HashMap<>();
		integrationLookup.put(allTypes.getDatasetType(), allTypes);
		integrationLookup.put(swAsm.getDatasetType(), swAsm);
		integrationLookup.put(swDrw.getDatasetType(), swDrw);
		integrationLookup.put(swPrt.getDatasetType(), swPrt);
		integrationLookup.put(proAsm.getDatasetType(), proAsm);
		integrationLookup.put(proDrw.getDatasetType(), proDrw);
		integrationLookup.put(proPrt.getDatasetType(), proPrt);
		
		try (FileOutputStream fos = new FileOutputStream(new File("tonxlfile.bin"));
			ObjectOutputStream oos = new ObjectOutputStream(fos)
				) {
			oos.writeObject(integrationLookup);
		} catch (IOException ioe) {
			
		}
		
		try (FileInputStream fis = new FileInputStream(new File("tonxlfile.bin"));
			ObjectInputStream ois = new ObjectInputStream(fis)
				) {
			@SuppressWarnings("unchecked")
			Map<String, IntegrationDatasetDTO> integrationLookup1 = (HashMap<String, IntegrationDatasetDTO>) ois.readObject();

			for (String name: integrationLookup1.keySet()) {
				IntegrationDatasetDTO data =  integrationLookup1.get(name);
				System.out.println(data.toString());
			}
		} catch (IOException ioe) {
			
		} catch (ClassNotFoundException e) {
			
		}
	
	}

}
