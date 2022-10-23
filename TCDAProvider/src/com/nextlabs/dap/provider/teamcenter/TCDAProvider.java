package com.nextlabs.dap.provider.teamcenter;

import com.nextlabs.dap.provider.ConnectionDetails;
import com.nextlabs.dap.provider.ConnectionStatus;
import com.nextlabs.dap.provider.InterfaceDAProvider;
import com.teamcenter.clientx.AppXSession;
import com.nextlabs.dap.provider.rmxuser.QueryUser;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

import java.io.IOException;
import java.util.HashMap;
import java.util.Properties;

public class TCDAProvider implements InterfaceDAProvider {
    private AppXSession session;
    private static Properties attributeMap;
    private static Properties queryAttribute;
    private static final String PROTOCOL = "http://";
    private static final String APPLICATION_PATH = "/tc";
    private static final Logger logger = LogManager.getLogger("TCDAPLOGGER");

    public TCDAProvider(){ }

    @Override
    public void init(ConnectionDetails connDetails) throws Exception {
        String serverHost = PROTOCOL + connDetails.getHostname() + ":" + connDetails.getPortNum() + APPLICATION_PATH;

        loadAttributeMapping();
        loadQueryAttribute();

        session = new AppXSession(serverHost);
        session.login(connDetails.getUserId(), connDetails.getPassword());
    }

    public static void loadAttributeMapping() {
        attributeMap = new Properties();
        try {
            attributeMap.load(TCDAProvider.class.getResourceAsStream("/user_attr_camelcase.properties"));
        } catch (IOException e) {
            logger.error("Failed to load internal attribute mappings: ", e);
        }
    }

    public static void loadQueryAttribute() {
        queryAttribute = new Properties();
        try {
            queryAttribute.load(TCDAProvider.class.getResourceAsStream("/query_attribute.properties"));
        } catch (IOException e) {
            logger.error("Failed to load internal query attribute mappings: ", e);
        }
    }

    @Override
    public ConnectionStatus testConnection() {
        return session.connectionStatus();
    }

    @Override
    public String[] getValue(String key, String userId) throws Exception {
        QueryUser queryUser = new QueryUser(attributeMap);
        queryUser.setQueryAttribute(queryAttribute);
        return queryUser.queryKey(key, userId);
    }

    @Override
    public HashMap<String, String[]> getValues(String[] keys, String userId) throws Exception {
        QueryUser queryUser = new QueryUser(attributeMap);
        queryUser.setQueryAttribute(queryAttribute);
        return queryUser.queryKeys(keys, userId);
    }

    public void destroy() throws Exception { session.logout(); }
}
