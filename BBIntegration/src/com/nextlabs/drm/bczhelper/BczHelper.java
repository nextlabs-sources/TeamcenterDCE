package com.nextlabs.drm.bczhelper;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.net.URI;
import java.nio.charset.StandardCharsets;
import java.nio.file.FileSystem;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.StandardCopyOption;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;


public class BczHelper {

	private static final Logger logger = LogManager.getLogger(BczHelper.class);
	private static final String LINE_SEPARATOR = System.getProperty("line.separator");
	private static final String OLD_NXL_HEADER = "NXLFMT!";
	private static final String NEW_NXL_HEADER = "NXLFMT@";

	public static void main(String[] args) {
		if (args.length != 1) {
			logger.error(() -> "Invalid number of arguments: " + args.length + ". Required exactly 1 arguments - The path to bcz file.");
		} else {
			String filePath = args[0];
			boolean result = RenameToNxl(filePath);
			if (result) {
				logger.info("Successfully renamed nxl file inside bcz.");
			} else {
				logger.info("Failed to renamed nxl file inside bcz.");
			}
		}
	}

	/*
	 * Add .NXL extension to all protected file inside a ZIP file (.bcz)
	 */
	public static boolean RenameToNxl(String bczFilePath) {

		boolean bRet = false;
		logger.info(LINE_SEPARATOR);
		logger.info(() -> "BczFile : " + bczFilePath);
		try {
			Map<String, String> env = new HashMap<>();
			env.put("create", "false");

			File fBczFile = new File(bczFilePath);
			URI uri = fBczFile.toURI();
			uri = new URI("jar:file", uri.getHost(), uri.getPath(), uri.getFragment());
			logger.info("URI : " + uri.toString());

			try (FileSystem zipFS = FileSystems.newFileSystem(uri, env)) {
				if (zipFS != null) {
					List<String> fileList = getAllProtectedFilesWithoutNxl(bczFilePath);

					for (String file : fileList) {
						logger.info(() -> "A protected file without \".nxl\" extension: " + file);
						Path filePath = zipFS.getPath(file);
						Path fileNewPath = zipFS.getPath(file + ".nxl");
						Files.move(filePath, fileNewPath, StandardCopyOption.REPLACE_EXISTING);
					}
					bRet = true;
				}
			}
		} catch (Exception ex) {
			logger.error("Exception when Rename files to NXL: ", ex);
		}
		return bRet;
	}

	private static List<String> getAllProtectedFilesWithoutNxl(String zipPath) {
		List<String> list = new ArrayList<>();

		try (ZipInputStream stream = new ZipInputStream(new FileInputStream(zipPath))){
			ZipEntry entry;
			while ((entry = stream.getNextEntry()) != null) {
				if (!entry.isDirectory() && !entry.getName().endsWith(".nxl")) {
					try {
						byte[] buffer = new byte[8];
						int len = stream.read(buffer);

						if (len >= 8) {
							String nxlHeader = new String(buffer, StandardCharsets.UTF_8);
							if (nxlHeader.startsWith(NEW_NXL_HEADER)) {
								list.add("/" + entry.getName());
							} else if (nxlHeader.startsWith(OLD_NXL_HEADER)) {
								logger.info("Old NXL format: " + entry.getName());
							}
						}
					} catch (IOException ioe) {
						logger.error(" StreamBuffer read error: ", ioe);
					}
				}
			}
		} catch (IOException ioe) {
			logger.error("ZipInputStream Error: ", ioe);
		}
		return list;
	}
}
