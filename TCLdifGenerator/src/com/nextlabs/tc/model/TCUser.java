package com.nextlabs.tc.model;

import java.util.ArrayList;
import java.util.List;

public class TCUser {
	private String 					name;
	private byte[] 					sid;
	private boolean 				duplicate;
	private String 					dn;
	private boolean 				ldapUser;
	private List<NameValuePair> 	attributes;

	public TCUser() {
		this.ldapUser = false;
		this.duplicate = false;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public byte[] getSid() {
		return this.sid;
	}

	public void setSid(byte[] sid) {
		this.sid = sid;
	}

	public boolean getDuplicate() {
		return duplicate;
	}

	public void setDuplicate(boolean duplicate) {
		this.duplicate = duplicate;
	}

	public String getDN() {
		return dn;
	}

	public void setDN(String dn) {
		this.dn = dn;
	}

	public boolean isLdapUser() {
		return ldapUser;
	}

	public void setIsLdapUser() {
		this.ldapUser = true;
	}

	public void addAttribute(String name, String value) {
		if (attributes == null) {
			attributes = new ArrayList<>();
		}
		NameValuePair pair = new NameValuePair(name,value);
		attributes.add(pair);
	}

	public String[] getAttribute(String name) {
		ArrayList<String> values = new ArrayList<>();
		if (attributes == null)
			return new String[0];
		for (NameValuePair pair : attributes) {
			if (pair.getName().equalsIgnoreCase(name)) {
				values.add(pair.getValue());
			}
		}
		return values.toArray(new String[0]);
	}

	public List<NameValuePair> getAttributes() {
		return attributes;
	}
}
