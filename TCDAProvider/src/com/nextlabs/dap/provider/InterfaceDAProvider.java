package com.nextlabs.dap.provider;

import java.util.HashMap;

public interface InterfaceDAProvider {
	
	/**
	 * Interface for a Dynamic Attribute Provider implementation.
	 * Each instance is expected to be called by only one thread.
	 */
	public void init(ConnectionDetails connDetails) throws Exception;
	
	public ConnectionStatus testConnection();
	
	/**
	 * Reads the values for the key into a String array, for a user Id.
     * @param key (String) The name of the user's attribute to be 
     *    retrieved.
     * @param userId (String) The user Id.
     * @return (String[]) The values of the user's attribute. The function may  
     *    return empty array if there is no value. If the key was not found,
     *    the function may throw InvalidKeyException. If the user was not found,
     *    the function may throw InvalidUserIdException.
     */
	public String[] getValue(String key, String userId) throws Exception;
	
	/**
	 * Reads the values for multiple keys into a Hashmap, for a user Id. Not thread safe
     * @param keys (String[]) The name of the user's attributes to be 
     *    retrieved.
     * @param userId (String) The user Id.
     * @return (HashMap<String, String[]>) The values of the user's attribute. The function may  
     *    return empty array if there is no value. If the key was not found,
     *    removes the invalid key from Hashmap. If the user was not found, the
     *    function may throw InvalidUserIdException.
     */
	public HashMap<String, String[]> getValues(String[] keys, String userId) throws Exception;
	
	/**
	 * This method will be called by DAP server when shutting down or test connection failed.
	 * Can implement cleanup process like logout the session.
	 * @throws Exception
	 */
	public void destroy() throws Exception;

}
