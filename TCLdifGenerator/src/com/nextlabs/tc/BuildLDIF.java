package com.nextlabs.tc;

import java.io.File;
import java.io.FileInputStream;
import java.io.InputStream;

import javax.naming.NamingEnumeration;
import javax.naming.directory.Attributes;
import javax.naming.directory.SearchResult;

import com.nextlabs.tc.ldif.LDIFGenerator;
import com.nextlabs.tc.ldif.utility.AttributeMapping;
import com.nextlabs.tc.ldif.utility.LDIFUtility;
import com.nextlabs.tc.ldif.utility.UserMapping;
import com.nextlabs.tc.model.TCGroup;
import com.nextlabs.tc.model.TCUser;
import com.nextlabs.tc.soa.QueryUser;
import com.nextlabs.tc.soa.QueryGroupMember;
import com.nextlabs.tc.soa.TeamcenterSOAConnection;

import com.teamcenter.schemas.soa._2006_03.exceptions.InvalidCredentialsException;
import com.teamcenter.soa.client.model.strong.GroupMember;
import com.teamcenter.soa.client.model.strong.User;

import java.util.*;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class BuildLDIF {
	private static final Logger logger = LogManager.getLogger("TCLDIFLOGGER");
	
	public static void main(String[] args) throws Exception {
		final String USER_DEFAULT_PROPERTIES_LIST = "nationality,fnd0citizenships,os_username";
		final String AD_DEFAULT_PROPERTIES_LIST = "sAMAccountName,givenName,cn,mail,objectSid,dn";

		// Load arguments
		String configFile;
		Properties programProperties = new Properties();
		if (args[0].equalsIgnoreCase("-config.file") || args[0].equalsIgnoreCase("-cp")) {
			configFile = args[1];
			if (!new File(configFile).exists()) {
				logger.info("Config File provided does not exist");
				System.exit(0);
			} else {
				logger.info("Load following config file: ");
				logger.info(configFile);
				InputStream inputStream = new FileInputStream(configFile);
				programProperties.load(inputStream);
				inputStream.close();
			}
		} else {
			logger.info("Please provide correct arguments: -config.file or -cp to the program");
			System.exit(0);
		}

		// Enrollment Scope of this run
		String enrollmentScope = programProperties.getProperty("enrollment.scope");
		boolean isTcScopeOnly = false;
		if (enrollmentScope.equalsIgnoreCase("ldap")) {
			logger.info("Enrollment Scope is ONLY from LDAP source. We recommend enroll directly from Control Center Enrollment Manager");
			System.exit(0);
		} else if (enrollmentScope.equalsIgnoreCase("teamcenter") || enrollmentScope.equalsIgnoreCase("tc")) {
			logger.info("Enrollment Scope is ONLY from Teamcenter source. There will be no AD/LDAP query.");
			isTcScopeOnly = true;
		} else {		// This is by default
			logger.info("Enrollment Scope is ALL (both Teamcenter and LDAP Source)");
		}

		logger.info("");
		// Start Teamcenter SOA Service
		String tcSoaUrl = programProperties.getProperty("teamcenter.soa.url");
		String tcUsername = programProperties.getProperty("teamcenter.username");
		logger.info(() -> "Using Teamcenter Proxy User " + tcUsername);
		logger.info("Please enter Teamcenter Proxy User Password: ");
		String tcPassword = new String(System.console().readPassword());
		String tcSiteName = programProperties.getProperty("teamcenter.site.name");
		try {
			TeamcenterSOAConnection soaConnection = new TeamcenterSOAConnection(tcSoaUrl, tcUsername, tcPassword);
			boolean validateLogin = soaConnection.testConnection();
			if (!validateLogin) {
				logger.info("Can't validate login with given credentials");
				System.exit(0);
			}
		} catch (InvalidCredentialsException ex) {
			logger.error("Invalid Credentials Provided: ", ex);
			System.exit(1);
		}

		logger.info("Start AD Connection");
		String ldapDomainFQDN = programProperties.getProperty("ldap.domain.fqdn");
		String ldapDomainServer = programProperties.getProperty("ldap.domain.server");
		String ldapUsername = programProperties.getProperty("ldap.username");
		logger.info(() -> "Using AD User: " + ldapUsername + " for connection.");
		logger.info("Please enter AD User Password: ");
		String ldapPassword = new String(System.console().readPassword());

		String ldapAttributes = programProperties.getProperty("ldap.attributes");
		String[] ldapExternalAttributeList = ldapAttributes.isEmpty() ? AD_DEFAULT_PROPERTIES_LIST.split(",") : ldapAttributes.split(",");

		String domainBase = ActiveDirectory.getDomainBase(ldapDomainFQDN);

		// Start AD Connection
		ActiveDirectory ad = new ActiveDirectory(ldapUsername, ldapPassword, ldapDomainFQDN, ldapDomainServer);
		ad.setReturnAttributes(ldapExternalAttributeList);
		String[] ldapAttributeList = ad.getReturnAttributes();

		String userMappingFile = programProperties.getProperty("enrollment.user.mapping");
		UserMapping userMapping = new UserMapping(userMappingFile);

		String attributesMappingFile = programProperties.getProperty("enrollment.attributes.mapping");
		AttributeMapping attrMapping = new AttributeMapping(attributesMappingFile);

		// Query User List
		String[] tcPropertyList;
		String tcProperties = programProperties.getProperty("teamcenter.properties");
		tcPropertyList = (tcProperties == null) ? USER_DEFAULT_PROPERTIES_LIST.split(",") : tcProperties.split(",");

		String tcUserPattern = programProperties.getProperty("teamcenter.user.pattern");
		tcUserPattern = tcUserPattern.isEmpty() ? "*" : tcUserPattern;

		User[] users = QueryUser.queryUserList(tcUserPattern, tcPropertyList);

		// Query Group List
		String tcGroupPattern = programProperties.getProperty("teamcenter.group.pattern");
		tcGroupPattern = tcGroupPattern.isEmpty() ? "*" : tcGroupPattern;

		GroupMember[] groupMembers = QueryGroupMember.queryGroupMemberList(tcGroupPattern);

		// Process a TCUser List for LDIF Output
		logger.info("");
		Map<String, TCUser> tcUsers = new HashMap<>();
		for (User user : users) {
			// Check User Mapping to find any entry of this username or else use tc username to search in AD
			String tcUserName = user.get_os_username();
			String adUserName = userMapping.get(tcUserName);

			logger.info(() -> "Teamcenter User : \"" + tcUserName + "\" matching with (LDAP Name=\"" + adUserName + "\")");

			// New Entry for TcUser. Name will follow Teamcenter source (primary source)
			TCUser tcUser = new TCUser();
			tcUser.setName(tcUserName);

			// TC Attributes process
			for (String prop : tcPropertyList) {
				String value = user.getPropertyDisplayableValue(prop);
				String attributeName = attrMapping.get(prop);
				tcUser.addAttribute(attributeName, value);
			}

			if (isTcScopeOnly) {
				tcUser.setSid(adUserName.getBytes());
				String dn = "CN=" + user.get_user_id() + ",OU=Users,OU=" + tcSiteName + "," + domainBase;
				tcUser.setDN(dn);
			} else {
				// Search adUserName in AD if Scope is "ALL" - both ldap and tc source
				SearchResult searchResult = ad.searchUser(adUserName);

				// Set SID base on search result. If unable to find, set as user_id
				byte[] sid = LDIFUtility.getSid(searchResult);
				sid = (sid == null) ? user.get_user_id().getBytes() : sid;
				tcUser.setSid(sid);

				// Set DN
				String dn = LDIFUtility.getDn(searchResult);
				if (dn == null || dn.isEmpty())
					dn = "CN=" + user.get_user_id() + ",OU=Users,OU=" + tcSiteName + "," + domainBase;
				tcUser.setDN(dn);

				if (searchResult == null) {
					logger.info(() -> "User \"" + adUserName + "\" is not existing in LDAP.");
				} else {
					logger.info(() -> "Found AD user " + adUserName);
					tcUser.setIsLdapUser();
					// AD attributes process
					Attributes internalAttributes = searchResult.getAttributes();
					NamingEnumeration<String> ids = internalAttributes.getIDs();
					while (ids.hasMore()) {
						String name = ids.next();
						if (ldapAttributeList != null && Arrays.asList(ldapAttributeList).contains(name)) {
							String value = null;
							try {
								value = (String)internalAttributes.get(name).get();
							}
							catch(ClassCastException ignored) {}

							if (value != null) {
								String attributeName = attrMapping.get(name);
								tcUser.addAttribute(attributeName, value);
							}
						}
					}
				}
			}
			tcUsers.put(tcUser.getName(), tcUser);
			logger.info("");
		}

		// Close AD Connection
		ad.closeLdapConnection();

		/*
		if(userMap!=null)
		{
			for (Map.Entry<Object, Object> e : userMap.entrySet()) 
			{
				String key = (String) e.getKey();
				String value = (String) e.getValue();
				TCUser srcUser=tcUsers.get(key);
				TCUser destUser=tcUsers.get(value);
				if(srcUser!=null&&destUser!=null)
				{
					destUser.setDuplicate(true);
				}
			}
		}
		*/

		LDIFUtility.printTCUserInfo((tcUsers.values().toArray(new TCUser[0])));

		Map<String, TCGroup> tcGroups = new HashMap<>();
		for (GroupMember groupMember : groupMembers) {
			String groupName = groupMember.get_group().get_name();
			String groupUserName = groupMember.get_user().get_user_id();
			boolean isGAUser = groupMember.get_ga();
			if (tcGroups.containsKey(groupName)) {
				TCGroup existingGroup = tcGroups.get(groupName);
				TCUser groupUser = tcUsers.get(groupUserName);

				existingGroup.addUser(groupUser);
				if(isGAUser)
					existingGroup.addAttribute("ga", groupMember.get_user().get_user_id());
			} else {
				TCGroup newGroup = new TCGroup();
				TCUser groupUser = tcUsers.get(groupUserName);

				newGroup.setName(groupName);
				newGroup.addUser(groupUser);
				String dn = "CN=" + groupName + ",OU=Groups,OU=" + tcSiteName + "," + domainBase;
				newGroup.setDN(dn);
				if(isGAUser)
					newGroup.addAttribute("ga", groupMember.get_user().get_user_id());

				tcGroups.put(groupName, newGroup);
			}
		}

		LDIFUtility.printTCGroupInfo((tcGroups.values().toArray(new TCGroup[0])));

		String outputFile = programProperties.getProperty("filepath.output");
		LDIFGenerator generator = new LDIFGenerator(tcUsers, tcGroups);
		generator.generate(outputFile);
	}
}
