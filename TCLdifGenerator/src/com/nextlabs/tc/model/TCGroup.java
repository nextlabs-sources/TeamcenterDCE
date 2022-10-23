package com.nextlabs.tc.model;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class TCGroup {

	private String name;
	private final Map<String, TCUser> members;
	private List<NameValuePair> attributes;
	private String dn;
	
	public TCGroup() {
		members = new HashMap<>();
	}

	public String getName(){
		return name;
	}

	public void setName(String name){
		this.name = name;
	}

	public String getDN() { return dn; }

	public void setDN(String dn) { this.dn = dn; }

	public void addUser(TCUser user){
		members.put(user.getName(), user);
	}

	public TCUser getUser(String name) {
		if (members.containsKey(name)) {
			return members.get(name);
		}
		return null;
	}
	
	public TCUser[] getUsers() {
		return members.values().toArray(new TCUser[0]);
	}
	
	public void addAttribute(String name,String value)
	{
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
