package com.nextlabs.drm.transrightschecker.config;

import java.io.*;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.*;
import java.util.Map.Entry;

import com.nextlabs.drm.rmx.configuration.ConfigManager;
import com.nextlabs.drm.rmx.configuration.SecretPropertiesDTO;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.apache.openaz.pepapi.*;
import org.apache.openaz.pepapi.std.StdPepAgentFactory;
import org.apache.openaz.xacml.api.Decision;

import static com.nextlabs.openaz.utils.Constants.*;
import static com.nextlabs.drm.transrightschecker.config.NextLabsPolicyConstants.*;

public class NextLabsAuthorizationAgent {
    private static final Logger logger = LogManager.getLogger(NextLabsAuthorizationAgent.class);

    private static NextLabsAuthorizationAgent instance;
    private PepAgent pepAgent;
    private String hostname;
    private boolean bConnected = false;

    private static final String LONG_LINE_BREAK = "-----------------------------------------------------------------------";

    private NextLabsAuthorizationAgent() { }

    private synchronized void initialize() {
        findHostName();
        try {
            loadConfiguration();
        } catch (Exception ex) {
            logger.info("Failed to initialize NextLabsAuthorizationAgent : ", ex);
        }
    }

    private void findHostName() {
        try {
            // Get hostname
            InetAddress address = InetAddress.getLocalHost(); // Potentially UnknownHostException
            hostname = address.getHostName();
            bConnected = true;
        } catch (UnknownHostException hostEx) {
            logger.error("Failed to get hostname of this machine: ", hostEx);
            bConnected = false;
        }
    }

    // TRC module can't access Teamcenter preference hence need to load configuration from files
    // policy_config.properties store non-sensitive config variable
    // secret.properties store encrypted API secret from Control Center
    private void loadConfiguration() throws Exception {
        Properties agentProperties = new Properties();
        String propertyFilePath = System.getenv("RMX_ROOT") + File.separator + "TranslatorRightsChecker" + File.separator + "policy_config.properties";
        String secretConfigPass = System.getenv("RMX_ROOT") + File.separator + "Translator" + File.separator + "tonxlfile" + File.separator + "bin";

        try (InputStream input = new FileInputStream(propertyFilePath)) {
            agentProperties.load(input);
        } catch (FileNotFoundException fex) {
            logger.error("Properties file not found: ", fex);
        } catch (IOException ioe) {
            logger.error("Error when loading properties: ", ioe);
        }

        // Add Client Secret to agentProperties from secret.properties file
        ConfigManager secretConfig = new ConfigManager.Builder(new File(secretConfigPass)).buildSecret();
        SecretPropertiesDTO secretPropertiesDTO = secretConfig.getSecretPropertiesDTO();
        agentProperties.setProperty(PDP_REST_OAUTH2_CLIENT_SECRET, secretPropertiesDTO.getSecret());

        PepAgentFactory pepAgentFactory = new StdPepAgentFactory(agentProperties);
        pepAgent = pepAgentFactory.getPepAgent();
    }

    public static synchronized NextLabsAuthorizationAgent getInstance() {
        if (instance == null) {
            instance = new NextLabsAuthorizationAgent();
            instance.initialize();
        }
        return instance;
    }

    public boolean checkNxlPolicy(String userName, Map<String, String[]> metadata, String defaultAction, String defaultMessage) {
        final int CONN_RETRY = 2;
        int connRetryCount = CONN_RETRY;

        while(!bConnected) {
            synchronized (this) {
                if (!bConnected) {
                    initialize();
                }
            }
            connRetryCount --;

            if (!bConnected && connRetryCount < 0) {
                logger.info("Connection to PDP REST API failed after trying " + CONN_RETRY + " times, will return default action which is " + defaultAction);
                logger.info("Default Message : " + defaultMessage);

                return false;
            }
        }
        return checkPolicyAndObligation(userName, metadata);
    }

    public boolean checkPolicyAndObligation(String userName, Map<String, String[]> metadata) {
        Subject user = Subject.newInstance(userName);
        user.addAttribute("tc_username", userName);

        Action action = Action.newInstance(NXL_POLICY_ACTION);

        Resource resource = Resource.newInstance("TC_TranslatorRightsChecker");
        resource.addAttribute(ID_RESOURCE_RESOURCE_TYPE.stringValue(), RESOURCE_TYPE);
        resource.addAttribute(ATTRIBUTE_OPERATION_SOURCE, OPERATION_SOURCE);
        resource.addAttribute(ATTRIBUTE_HOST, hostname);
        resource.addAttribute(ATTRIBUTE_APPLICATION, APPLICATION);
        for (String key : metadata.keySet()) {
            resource.addAttribute(key, metadata.get(key));
        }

        Environment environment = Environment.newInstance();

        summarizePepRequest(user, action, resource, environment);
        PepResponse pepResponse = pepAgent.decide(user, action, resource, environment);

        boolean isPolicyAllow = checkPolicyDecision(pepResponse);
        boolean isRemoveProtectionObligationExist = checkRemoveProtectionObligation(pepResponse);

        return isPolicyAllow && isRemoveProtectionObligationExist;
    }

    private void summarizePepRequest(Subject user, Action action, Resource resource, Environment environment) {
        logger.info("");
        logger.info(LONG_LINE_BREAK);
        logger.info("|   Subject attributes:");
        logger.info("|           subject-id : " + user.getSubjectIdValue());
        for (Map.Entry<String, Object[]> entry : user.getAttributeMap().entrySet()) {
            if (entry.getKey().equals(Subject.SUBJECT_ID_KEY))
                continue;
            logger.info("|           " + entry.getKey() + " : " + entry.getValue()[0]);
        }
        logger.info("|");

        logger.info("|   Action : " + action.getActionIdValue());
        logger.info("|");

        logger.info("|   Resource attributes: ");
        logger.info("|           resource-id : " + resource.getResourceIdValue());
        for (Map.Entry<String, Object[]> entry : resource.getAttributeMap().entrySet()) {
            if (entry.getKey().equals(Resource.RESOURCE_ID_KEY))
                continue;
            logger.info("|           " + entry.getKey() + " : " + Arrays.toString(entry.getValue()));
        }
        logger.info("|");

        logger.info("|   Environment attributes: ");
        for (Map.Entry<String, Object[]> entry : environment.getAttributeMap().entrySet()) {
            logger.info("|           " + entry.getKey() + " : " + Arrays.toString(entry.getValue()));
        }
        logger.info(LONG_LINE_BREAK);
    }

    private boolean checkPolicyDecision(PepResponse pepResponse) {
        Decision decision = pepResponse.getWrappedResult().getDecision();
        boolean isAllow = decision == Decision.PERMIT;
        String effectValue = isAllow ? "Allow" : decision.toString();

        logger.info(LONG_LINE_BREAK);
        logger.info("|   Response : ");
        logger.info("|           Effect: " + effectValue + " ");

        return isAllow;
    }

    private boolean checkRemoveProtectionObligation(PepResponse pepResponse) {
        if (!pepResponse.getObligations().isEmpty()) {
            boolean haveCorrectObligation = false;
            logger.info("|           Obligations: ");

            for (Entry<String, Obligation> obligationEntry : pepResponse.getObligations().entrySet()) {
                String obligationName = obligationEntry.getKey();
                logger.info("|               " + obligationEntry.getKey());

                if (obligationName.equals(NextLabsPolicyConstants.OBLIGATION_REMOVEPROTECTION)) {
                    haveCorrectObligation = true;
                }
            }
            logger.info(LONG_LINE_BREAK);

            return haveCorrectObligation;
        }
        return false;
    }
}
