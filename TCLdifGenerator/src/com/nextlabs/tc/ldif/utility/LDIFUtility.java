package com.nextlabs.tc.ldif.utility;

import com.nextlabs.tc.ActiveDirectory;
import com.nextlabs.tc.model.NameValuePair;
import com.nextlabs.tc.model.TCGroup;
import com.nextlabs.tc.model.TCUser;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import javax.naming.NamingException;
import javax.naming.directory.Attributes;
import javax.naming.directory.SearchResult;
import java.util.List;

public class LDIFUtility {
    private LDIFUtility() {}

    private static final Logger logger = LogManager.getLogger("TCLDIFLOGGER");

    public static byte[] getSid(SearchResult searchResult) {
        byte[] sid = null;
        if (searchResult == null)
            return new byte[0];
        try {
            Attributes attrs = searchResult.getAttributes();
            sid = (byte[])attrs.get("objectSID").get();
        } catch(NamingException exp) {
            logger.error("Failed to get Object SID: ", exp);
        }
        return sid;
    }

    public static String getDn(SearchResult searchResult) {
        String dn;
        if (searchResult == null)
            return null;
        dn = searchResult.getNameInNamespace();
        return dn;
    }

    public static void printTCUserInfo(TCUser[] users) {
        logger.info("All Teamcenter Users Information: ");
        int index = 1;
        for (TCUser user : users) {
            int finalIndex = index;
            logger.info(() -> "\t" + (finalIndex) + ": "+ user.getName());
            if (user.isLdapUser()) {
                logger.info(() -> "\t\tSID: " + ActiveDirectory.decodeSID(user.getSid()));
            }
            logger.info(() -> "\t\tIsLdapUser: " + user.isLdapUser());
            List<NameValuePair> userAttributes = user.getAttributes();
            for (NameValuePair pair : userAttributes) {
                logger.info(() -> "\t\t" + pair.getName() + ": \"" + pair.getValue() + "\"");
            }

            index ++;
        }
    }

    public static void printTCGroupInfo(TCGroup[] groups) {
        logger.info("All Teamcenter Groups Information: ");
        int index = 1;
        for (TCGroup group : groups) {
            int finalIndex = index;
            logger.info(() -> "\t" + (finalIndex) + ": " + group.getName());
            logger.info("\tDN: " + group.getDN());
            logger.info("\t\tMembers: ");
            TCUser[] members = group.getUsers();
            if (members == null || members.length <= 0)
                continue;

            for (TCUser member : members) {
                logger.info(() -> "\t\t\t" + member.getName());
            }

            List<NameValuePair> groupAttributes = group.getAttributes();
            if (groupAttributes == null) {
                logger.info("\t\tNo Group Attribute found");
            } else {
                for (NameValuePair pair : groupAttributes) {
                    logger.info(() -> "\t\t" + pair.getName() + ": " + pair.getValue());
                }
            }
            index ++;
        }
    }
}
