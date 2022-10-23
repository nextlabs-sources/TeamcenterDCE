package com.nextlabs.dap.provider.rmxuser;

import com.nextlabs.dap.provider.ConnectionDetails;
import com.nextlabs.dap.provider.teamcenter.TCDAProvider;

import java.util.Arrays;
import java.util.HashMap;

public class Query {

    private static final String PROXY_USER_ID = "infodba";
    private static final String PROXY_USER_PS = "infodba";
    private static final String HOST_NAME = "tc06";
    private static final int    PORT_NUMBER = 8080;
    private static TCDAProvider dapProvider;

    public static void main(String[] args) throws Exception{
        dapProvider = new TCDAProvider();
        ConnectionDetails connectionDetails = new ConnectionDetails(PROXY_USER_ID, PROXY_USER_PS, HOST_NAME, PORT_NUMBER);
        dapProvider.init(connectionDetails);
        System.out.println(dapProvider.testConnection().isSuccess());


        getValue("geography", "abraham.lincoln");
        getValue("userid", "infodba");
        getValue("geography", "jimmy.carter");
        getValue("ada_license", "jimmy.carter");
        getValue("fnd0userdeclaredgeography", "jimmy.carter");
        getValue("fnd0licenseserver", "jimmy.carter");

        String[] inputKeys = new String[] { "gov_clearance", "fnd0userdeclaredgeography", "ada_license"};
        getValues(inputKeys, "jimmy.carter");

        try
        {
            Thread.sleep(3000);
        }
        catch(InterruptedException ex)
        {
            Thread.currentThread().interrupt();
        }


        // Terminate the session with the Teamcenter server
        dapProvider.destroy();
    }

    public static void getValue(String key, String userId) {
        System.out.println("Querying following key: " + key + " of user : " + userId);

        try {
            String[] keyValues = dapProvider.getValue(key, userId);
            System.out.println("Number of items: " + keyValues.length);
            System.out.println(Arrays.toString(keyValues));
        } catch (Exception ex) {
            System.out.println(ex.getMessage());
        }
    }

    public static HashMap<String, String[]> getValues(String[] keys, String userId) {
        System.out.println("Querying following key: " + Arrays.toString(keys) + " of user : " + userId);

        try {
            HashMap<String, String[]> keyValues = dapProvider.getValues(keys, userId);

            for (String key: keyValues.keySet()) {
                String[] values = keyValues.get(key);
                System.out.println(key + " " + Arrays.toString(values));
            }
            return keyValues;
        } catch (Exception ex) {
            System.out.println(ex.getMessage());
        }
        return new HashMap<>();
    }
}
