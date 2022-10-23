package com.nextlabs.tc;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Properties;
import java.util.UUID;

import javax.naming.Context;
import javax.naming.NamingEnumeration;
import javax.naming.NamingException;
import javax.naming.directory.DirContext;
import javax.naming.directory.InitialDirContext;
import javax.naming.directory.SearchControls;
import javax.naming.directory.SearchResult;


/**
 * Query Active Directory using Java
 */
public class ActiveDirectory {
    private static final Logger logger                      = LogManager.getLogger("TCLDIFLOGGER");

    private DirContext dirContext;
    private final SearchControls searchControls;
	private String[] returnAttributes = { "sAMAccountName", "givenName", "cn", "mail","objectSid","dn" };
    private final String domainBase;

    public ActiveDirectory(String username, String password, String domainController,String domainServerUrl) {
        Properties properties = new Properties();

        properties.put(Context.INITIAL_CONTEXT_FACTORY, "com.sun.jndi.ldap.LdapCtxFactory");
        
        logger.info(() -> "Provider Url: " + domainServerUrl);
        properties.put(Context.PROVIDER_URL, domainServerUrl);

        logger.info(() -> "SECURITY_PRINCIPAL: " + username + "@" + domainController);
        properties.put(Context.SECURITY_PRINCIPAL, username + "@" + domainController);

        properties.put(Context.SECURITY_CREDENTIALS, password);
        properties.put(Context.REFERRAL, "follow");
        properties.put("java.naming.ldap.attributes.binary", "objectGUID objectSID msExchMailboxGuid userCertificate");

        try {
			dirContext = new InitialDirContext(properties);
		} catch (NamingException e) {
			logger.error("Error on initializing Active Directory", e);
		}

        domainBase = getDomainBase(domainController);
        
        //initializing search controls
        searchControls = new SearchControls();
        searchControls.setSearchScope(SearchControls.SUBTREE_SCOPE);
    }

    public String[] getReturnAttributes() { return returnAttributes; }

    public void setReturnAttributes(String[] attrs) {
        if (attrs == null)
            return;

        List<String> both = new ArrayList<>(returnAttributes.length + attrs.length);
        Collections.addAll(both, returnAttributes);
        Collections.addAll(both, attrs);
        returnAttributes = both.toArray(new String[0]);
        searchControls.setReturningAttributes(returnAttributes);
    }
    
    /**
     * search the Active directory by username/email id for given search base
     * 
     * @param searchValue a {@link java.lang.String} object - search value used for AD search for eg. username or email
     * @param searchBy a {@link java.lang.String} object - scope of search by username or by email id
     * @param searchBase a {@link java.lang.String} object - search base value for scope tree for eg. DC=myjeeva,DC=com
     * @return search result a {@link javax.naming.NamingEnumeration} object - active directory search result
     * @throws NamingException
     */
    public NamingEnumeration<SearchResult> searchUser(String searchValue, String searchBy, String searchBase) throws NamingException {
    	String filter = getFilter(searchValue, searchBy); 
    	String base = (null == searchBase) ? domainBase : getDomainBase(searchBase);
    	
		return this.dirContext.search(base, filter, this.searchControls);
    }

    public SearchResult searchUser(String searchValue) {
    	return searchUser(searchValue,"username");
    }
    
    public SearchResult searchUser(String searchValue,String searchBy) {
    	SearchResult rs = null;

    	try {
			NamingEnumeration<SearchResult> result = this.searchUser(searchValue, searchBy, null);
			if (result != null && result.hasMore()) {
				rs = result.next();
			}
    	} catch(Exception exp) {
            return null;
		}
		return rs;
    }

    /**
     * closes the LDAP connection with Domain controller
     */
    public void closeLdapConnection() {
        try {
            if(dirContext != null)
                dirContext.close();
        } catch (NamingException e) {
        	logger.error(e.getMessage());
        }
    }
    
    public static String getGuidFromByteArray(byte[] bytes)
    {
        UUID uuid = UUID.nameUUIDFromBytes(bytes);
        return uuid.toString();
    }
    public static String decodeSID(byte[] sid) {
        
        final StringBuilder strSid = new StringBuilder("S-");

        // get version
        final int revision = sid[0];
        strSid.append(Integer.toString(revision));
        
        //next byte is the count of sub-authorities
        final int countSubAuths = sid[1] & 0xFF;
        
        //get the authority
        long authority = 0;
        for(int i = 2; i <= 7; i++) {
           authority |= ((long)sid[i]) << (8 * (5 - (i - 2)));
        }
        strSid.append("-");
        strSid.append(Long.toHexString(authority));
        
        //iterate all the sub-auths
        int offset = 8;
        int size = 4; //4 bytes for each sub auth
        for(int j = 0; j < countSubAuths; j++) {
            long subAuthority = 0;
            for(int k = 0; k < size; k++) {
                subAuthority |= (long)(sid[offset + k] & 0xFF) << (8 * k);
            }
            
            strSid.append("-");
            strSid.append(subAuthority);
            
            offset += size;
        }
        
        return strSid.toString();    
    }
    /**
     * active directory filter string value
     * 
     * @param searchValue a {@link java.lang.String} object - search value of username/email id for active directory
     * @param searchBy a {@link java.lang.String} object - scope of search by username or email id
     * @return a {@link java.lang.String} object - filter string
     */
    private String getFilter(String searchValue, String searchBy) {
    	String filter = "(&((&(objectCategory=Person)(objectClass=User)))";
    	if(searchBy.equals("email")) {
    		filter += "(mail=" + searchValue + "))";
    	} else if(searchBy.equals("username")) {
    		filter += "(samaccountname=" + searchValue + "))";
    	}
		return filter;
	}
    
    /**
     * creating a domain base value from domain controller name
     * 
     * @param base a {@link java.lang.String} object - name of the domain controller
     * @return a {@link java.lang.String} object - base name for eg. DC=myjeeva,DC=com
     */
	public static String getDomainBase(String base) {
		char[] namePair = base.toUpperCase().toCharArray();
		StringBuilder dn = new StringBuilder("DC=");
		for (int i = 0; i < namePair.length; i++) {
			if (namePair[i] == '.') {
				dn.append(",DC=").append(namePair[++i]);
			} else {
				dn.append(namePair[i]);
			}
		}
		return dn.toString();
	}
}