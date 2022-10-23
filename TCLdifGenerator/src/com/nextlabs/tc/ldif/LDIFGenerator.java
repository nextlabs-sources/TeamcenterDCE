package com.nextlabs.tc.ldif;

import com.nextlabs.tc.ActiveDirectory;
import com.nextlabs.tc.model.NameValuePair;
import com.nextlabs.tc.model.TCGroup;
import com.nextlabs.tc.model.TCUser;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.io.*;
import java.util.List;
import java.util.Map;
import java.util.UUID;

public class LDIFGenerator {
    final Logger logger = LogManager.getLogger("TCLDIFLOGGER");
    final String lineSeparator = System.getProperty("line.separator");
    static final String OBJECT_GUID_FIELD = "objectGUID: ";
    static final String OBJECT_SID_FIELD = "objectSid: ";

    Map<String, TCGroup> groupInformation;
    Map<String, TCUser> userInformation;

    public LDIFGenerator(Map<String, TCUser> userInformation, Map<String, TCGroup> groupInformation) {
        this.groupInformation = groupInformation;
        this.userInformation = userInformation;
    }

    public void generate(String fileOutputPath) throws FileNotFoundException {
        TCGroup[] groupList = (groupInformation.values().toArray(new TCGroup[0]));
        TCUser[] userList = (userInformation.values().toArray(new TCUser[0]));

        File output = new File(fileOutputPath);
        FileOutputStream fos = new FileOutputStream(output);
        OutputStreamWriter osw = new OutputStreamWriter(fos);
        try (Writer w = new BufferedWriter(osw)) {
            for (TCUser user : userList) {
                w.write(userLDIFEntry(user));
                w.write(lineSeparator);
            }
            for (TCGroup group : groupList) {
                w.write(groupLDIFEntry(group));
                w.write(lineSeparator);
            }
        } catch (IOException ex) {
            logger.error("Unable to write to output file : ", ex);
        }
    }

    private String userLDIFEntry(TCUser user) {
        StringBuilder entry = new StringBuilder();
        entry.append("dn: ").append(user.getDN()).append(lineSeparator);
        entry.append("changetype: add").append(lineSeparator);
        entry.append("objectClass: top").append(lineSeparator);
        entry.append("objectClass: person").append(lineSeparator);
        entry.append("objectClass: organizationPerson").append(lineSeparator);
        entry.append("objectClass: user").append(lineSeparator);
        entry.append("cn: ").append(user.getName()).append(lineSeparator);
        entry.append("displayName: ").append(user.getName()).append(lineSeparator);
        entry.append("userPrincipalName: ").append(user.getName()).append(lineSeparator);

        if (user.isLdapUser()) {
            entry.append(OBJECT_GUID_FIELD).append(ActiveDirectory.decodeSID(user.getSid())).append(lineSeparator);
            entry.append(OBJECT_SID_FIELD).append(ActiveDirectory.decodeSID(user.getSid())).append(lineSeparator);
        } else {
            // Generate GUID for non AD user
            UUID guid = UUID.randomUUID();
            entry.append(OBJECT_GUID_FIELD).append(guid).append(lineSeparator);
            entry.append(OBJECT_SID_FIELD).append(guid).append(lineSeparator);
        }

        List<NameValuePair> attributes = user.getAttributes();
        for (NameValuePair pair : attributes) {
            if (pair.getValue() != null && !pair.getValue().isEmpty()) {
                entry.append(pair.getName()).append(": ").append(pair.getValue()).append(lineSeparator);
            }
        }

        TCGroup[] groupList = (groupInformation.values().toArray(new TCGroup[0]));
        for(TCGroup group : groupList) {
            if (group.getUser(user.getName()) != null) {
                String groupDN = group.getDN();
                entry.append("memberOf: ").append(groupDN).append(lineSeparator);
            }
        }

        return entry.toString();
    }

    private String groupLDIFEntry(TCGroup group) {
        StringBuilder entry = new StringBuilder();
        entry.append("dn: ").append(group.getDN()).append(lineSeparator);
        entry.append("changetype: add").append(lineSeparator);
        entry.append("objectClass: top").append(lineSeparator);
        entry.append("objectClass: group").append(lineSeparator);
        entry.append("cn: ").append(group.getName()).append(lineSeparator);
        entry.append("name: ").append(group.getName()).append(lineSeparator);
        entry.append(OBJECT_GUID_FIELD).append(group.getName()).append(lineSeparator);
        entry.append(OBJECT_SID_FIELD).append(group.getName()).append(lineSeparator);

        for (TCUser member : group.getUsers()) {
            entry.append("member: ").append(member.getDN()).append(lineSeparator);
        }

        List<NameValuePair> attributes = group.getAttributes();
        if (attributes != null) {
            for (NameValuePair pair : attributes) {
                if (pair.getValue() != null && !pair.getValue().isEmpty())
                    entry.append(pair.getName()).append(": ").append(pair.getValue()).append(lineSeparator);
            }
        }

        return entry.toString();
    }
}
