package com.nextlabs.dap.provider.rmxuser;

import com.nextlabs.dap.provider.exception.InvalidKeyException;
import com.nextlabs.dap.provider.exception.InvalidUserIdException;

import com.teamcenter.clientx.AppXSession;
import com.teamcenter.services.strong.administration.UserManagementService;
import com.teamcenter.services.strong.administration._2012_09.UserManagement.*;
import com.teamcenter.services.strong.core.DataManagementService;
import com.teamcenter.services.strong.query.SavedQueryService;
import com.teamcenter.services.strong.query._2007_09.SavedQuery.QueryResults;
import com.teamcenter.services.strong.query._2007_09.SavedQuery.SavedQueriesResponse;
import com.teamcenter.services.strong.query._2008_06.SavedQuery.QueryInput;
import com.teamcenter.services.strong.query._2010_04.SavedQuery.FindSavedQueriesCriteriaInput;
import com.teamcenter.services.strong.query._2010_04.SavedQuery.FindSavedQueriesResponse;
import com.teamcenter.soa.client.model.ModelObject;
import com.teamcenter.soa.client.model.Property;
import com.teamcenter.soa.client.model.ServiceData;
import com.teamcenter.soa.client.model.strong.ImanQuery;
import com.teamcenter.soa.client.model.strong.User;
import com.teamcenter.soa.client.model.strong.ADA_License;
import com.teamcenter.soa.exceptions.NotLoadedException;

import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.json.JSONObject;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.*;

public class QueryUser {
    Properties attributeMap;
    Properties queryAttribute;
    private static final int    MAX_NUM_OBJECT_RETURN       = 25;
    private static final String ADA_LICENSE_PROPERTY        = "ada_license";
    private static final String FIND_USER_QUERY_NAME        = "find_user_query_name";
    private static final String FIND_USER_QUERY_NAME_DEFAULT_VALUE      = "Admin - Employee Information";
    private static final String FIND_USER_QUERY_FIELDS      = "find_user_query_fields";
    private static final String FIND_USER_QUERY_FIELDS_VALUE            = "find_user_query_fields_value";
    private static final Logger logger                      = LogManager.getLogger("TCDAPLOGGER");
    private final SavedQueryService queryService            = SavedQueryService.getService(AppXSession.getConnection());
    private final DataManagementService dmService           = DataManagementService.getService(AppXSession.getConnection());

    public QueryUser(Properties attributeMap) {
        this.attributeMap = attributeMap;

        Properties defaultQueryAttr = new Properties();
        defaultQueryAttr.setProperty(FIND_USER_QUERY_NAME, FIND_USER_QUERY_NAME_DEFAULT_VALUE);
        defaultQueryAttr.setProperty(FIND_USER_QUERY_FIELDS, "");
        defaultQueryAttr.setProperty(FIND_USER_QUERY_FIELDS_VALUE, "");
        this.queryAttribute = defaultQueryAttr;
    }

    public void setQueryAttribute(Properties queryAttribute) {
        this.queryAttribute = queryAttribute;
    }

    public String[] queryKey(String key, String userId) throws Exception {
        // Retrieve User Object
        User user = getUserObject(userId);

        String tcKey;
        // Check if the key is in attribute mapping (To match lower case key name to correct camel case format for TC query)
        if (attributeMap.containsKey(key.toLowerCase())) {
            tcKey = attributeMap.getProperty(key.toLowerCase());
        } else {
            tcKey = key;
        }

        // The special case for license (ADA License) property since we will need a unique query
        if (tcKey.equalsIgnoreCase(ADA_LICENSE_PROPERTY)) {
            logger.debug("Querying " + ADA_LICENSE_PROPERTY + " key.");
            return queryLicenseKey(userId, user);
        }

        logger.debug(() -> "Query Teamcenter Attribute : " + tcKey);
        logger.debug("Refresh User object and its property");
        dmService.refreshObjects(new ModelObject[] { user });
        dmService.getProperties(new ModelObject[] { user }, new String[] { tcKey });
        try {
            Property keyValue = user.getPropertyObject(tcKey);
            logger.debug(() -> "Returning Key: " + key + " - Value: " + keyValue.getDisplayableValues());
            return keyValue.getDisplayableValues().toArray(new String[0]);
        } catch (IllegalArgumentException | NotLoadedException ex) {
            throw new InvalidKeyException(tcKey);
        }
    }

    public HashMap<String, String[]> queryKeys(String[] keys, String userId) throws Exception {
        HashMap<String, String[]> queryResult = new HashMap<>();

        // Retrieve User Object
        User user = getUserObject(userId);

        String[] tcKeys = new String[keys.length];
        // Check if the key is in attribute mapping (To match lower case key name to correct camel case format for TC query)
        for (int i=0; i < keys.length; i++) {
            if (attributeMap.containsKey(keys[i].toLowerCase())) {
                tcKeys[i] = attributeMap.getProperty(keys[i].toLowerCase());
            } else {
                tcKeys[i] = keys[i];
            }
        }

        logger.debug("Refresh User object and its property list");
        dmService.refreshObjects(new ModelObject[] { user });
        dmService.getProperties(new ModelObject[] { user }, tcKeys);

        for (int i=0; i < tcKeys.length; i++) {
            String key = tcKeys[i];
            try {
                if (key.equalsIgnoreCase(ADA_LICENSE_PROPERTY)) {
                    queryResult.put(keys[i], queryLicenseKey(userId, user));
                } else {
                    Property keyValue = user.getPropertyObject(key);

                    queryResult.put(keys[i], keyValue.getDisplayableValues().toArray(new String[0]));
                }
            } catch (Exception e) {
                // KeyNotFoundException: Ignore when query multiple keys
            }
        }
        logger.debug("Returning");
        for (String key: queryResult.keySet()) {
            String[] values = queryResult.get(key);
            logger.debug(() -> key + " " + Arrays.toString(values));
        }
        return queryResult;
    }

    private User getUserObject(String userId) throws Exception {
        String queryName = queryAttribute.getProperty(FIND_USER_QUERY_NAME);
        String queryFields = queryAttribute.getProperty(FIND_USER_QUERY_FIELDS);
        if (queryFields.equals("")) {
            queryFields = "User ID";
        } else {
            queryFields = "User ID," + queryFields;
        }
        String queryFieldsValue = queryAttribute.getProperty(FIND_USER_QUERY_FIELDS_VALUE);
        if (queryFieldsValue.equals("")) {
            queryFieldsValue = userId;
        } else {
            queryFieldsValue = userId + "," + queryFieldsValue;
        }

        FindSavedQueriesCriteriaInput[] queryCriteriaInputs = new FindSavedQueriesCriteriaInput[1];
        queryCriteriaInputs[0] = new FindSavedQueriesCriteriaInput();
        queryCriteriaInputs[0].queryNames = new String[] { queryName };

        FindSavedQueriesResponse queryResponse = queryService.findSavedQueries(queryCriteriaInputs);
        if (queryResponse.savedQueries.length == 0) {
            throw new Exception("Unable to find User related query. " + queryName + " query must be in QueryBuilder list.");
        }

        ImanQuery query = queryResponse.savedQueries[0];
        logger.debug(() -> "Found " + queryName + " type in QueryBuilder list. Executing a query to find user");

        QueryInput[] savedQueryInput = new QueryInput[1];
        savedQueryInput[0] = new QueryInput();
        savedQueryInput[0].query = query;
        savedQueryInput[0].maxNumToReturn = MAX_NUM_OBJECT_RETURN;
        savedQueryInput[0].limitList = new ModelObject[0];
        // savedQueryInput[0].entries = new String[] { QUERY_NAME_USERID_FIELD };
        savedQueryInput[0].entries = queryFields.split(",");
        savedQueryInput[0].values = queryFieldsValue.split(",");
        // savedQueryInput[0].values = new String[1]; savedQueryInput[0].values[0] = userId;

        SavedQueriesResponse savedQueryResult = queryService.executeSavedQueries(savedQueryInput);
        QueryResults found = savedQueryResult.arrayOfResults[0];

        if (found.objectUIDS.length == 0) {
            throw new InvalidUserIdException(userId);
        }

        logger.debug("Found object matched with given UserID");
        ServiceData sd = dmService.loadObjects(found.objectUIDS);
        User user = (User) sd.getPlainObject(0);
        logger.debug(() -> "User : " + userId);

        return user;
    }

    private String[] queryLicenseKey(String userId, User user) throws Exception {
        String[] userRelatedLicense = queryUserADALicenseKey(userId);
        String[] userGroups = getUserGroupList(user);

        Set<String> licenseSet = new HashSet<>(Arrays.asList(userRelatedLicense));
        for (String groupId : userGroups) {
            String[] groupRelatedLicense = queryGroupADALicenseKey(groupId);
            licenseSet.addAll(Arrays.asList(groupRelatedLicense));
        }

        return licenseSet.toArray(new String[0]);
    }

    private String[] queryUserADALicenseKey(String userId) throws Exception {
        final String QUERY_NAME_LICENSE          = "FindLicenseByUser";
        final String QUERY_NAME_LICENSE_FIELD    = "Users";

        FindSavedQueriesCriteriaInput[] queryCriteriaInputs = new FindSavedQueriesCriteriaInput[1];
        queryCriteriaInputs[0] = new FindSavedQueriesCriteriaInput();
        queryCriteriaInputs[0].queryNames = new String[] { QUERY_NAME_LICENSE };

        FindSavedQueriesResponse queryResponse = queryService.findSavedQueries(queryCriteriaInputs);
        if (queryResponse.savedQueries.length == 0) {
            throw new Exception("Unable to find ADA License related query. " + QUERY_NAME_LICENSE + " query must be in QueryBuilder list.");
        }

        ImanQuery query = queryResponse.savedQueries[0];
        logger.debug(() -> "Found " + QUERY_NAME_LICENSE + " type in QueryBuilder list. Executing a query to find user/license pair");

        QueryInput[] savedQueryInput = new QueryInput[1];
        savedQueryInput[0] = new QueryInput();
        savedQueryInput[0].query = query;
        savedQueryInput[0].maxNumToReturn = MAX_NUM_OBJECT_RETURN;
        savedQueryInput[0].limitList = new ModelObject[0];
        savedQueryInput[0].entries = new String[] { QUERY_NAME_LICENSE_FIELD };
        savedQueryInput[0].values = new String[1];
        savedQueryInput[0].values[0] = userId;

        SavedQueriesResponse savedQueryResult = queryService.executeSavedQueries(savedQueryInput);
        QueryResults found = savedQueryResult.arrayOfResults[0];

        ServiceData sd = dmService.loadObjects(found.objectUIDS);
        ModelObject[] foundObjects = new ModelObject[sd.sizeOfPlainObjects()];
        String[] licenseList = new String[sd.sizeOfPlainObjects()];

        for (int k = 0; k < sd.sizeOfPlainObjects(); k++) {
            foundObjects[k] = sd.getPlainObject(k);
        }

        logger.debug("Load ID property for ADA_License found");
        dmService.refreshObjects(foundObjects);
        dmService.getProperties(foundObjects, new String[] { "id", "lock_date", "expiry_date", "fnd0license_category", "fnd0user_citizenships" });

        try {
            for (int k = 0; k < foundObjects.length; k++) {
                ADA_License license = (ADA_License) foundObjects[k];

                String licenseInfo = summarizeADALicenseInfo(license);
                logger.info(licenseInfo);
                licenseList[k] = licenseInfo;
            }
            logger.info(() -> "Returning User's related ADA_License: " + Arrays.toString(licenseList));

            return licenseList;
        } catch (NotLoadedException ex) {
            throw new NotLoadedException("Property Id not loaded for all ADA License found");
        }
    }

    private String[] getUserGroupList(User user) throws Exception {
        final String INPUT_CLIENT_ID          = "TcDAP";

        List<String> groupListOutput = new ArrayList<>();

        UserManagementService userManagementService = UserManagementService.getService(AppXSession.getConnection());

        GetUserGroupMembersInputData[] inputs = new GetUserGroupMembersInputData[1];
        inputs[0] = new GetUserGroupMembersInputData();
        inputs[0].clientId = INPUT_CLIENT_ID;
        inputs[0].includeInactive = false;
        inputs[0].includeUser = true;
        inputs[0].user = user;

        GetUserGroupMembersResponse response = userManagementService.getUserGroupMembers(inputs);
        GetUserGroupMembersOutput[] groupDataOutputList = response.outputs;

        for (GetUserGroupMembersOutput groupData : groupDataOutputList) {
            UserGroupMemberData[] data = groupData.memebrs;
            for (UserGroupMemberData i : data) {
                dmService.getProperties(new ModelObject[] { i.group }, new String[] { "name" });
                logger.debug("    Belong to group: " + i.group.get_name());
                groupListOutput.add(i.group.get_name());
            }
        }

        String[] result = groupListOutput.toArray(new String[0]);
        logger.info(() -> "Belong to group(s): " + Arrays.toString(result));
        return result;
    }

    private String[] queryGroupADALicenseKey(String groupId) throws Exception {
        final String QUERY_NAME_LICENSE          = "FindLicenseByGroup";
        final String QUERY_GROUP_LICENSE_FIELD   = "Groups";

        FindSavedQueriesCriteriaInput[] queryCriteriaInputs = new FindSavedQueriesCriteriaInput[1];
        queryCriteriaInputs[0] = new FindSavedQueriesCriteriaInput();
        queryCriteriaInputs[0].queryNames = new String[] { QUERY_NAME_LICENSE };

        FindSavedQueriesResponse queryResponse = queryService.findSavedQueries(queryCriteriaInputs);
        if (queryResponse.savedQueries.length == 0) {
            throw new Exception("Unable to find ADA License related query. " + QUERY_NAME_LICENSE + " query must be in QueryBuilder list.");
        }

        ImanQuery query = queryResponse.savedQueries[0];
        logger.debug(() -> "Found " + QUERY_NAME_LICENSE + " type in QueryBuilder list. Executing a query to find group/license pair");

        QueryInput[] savedQueryInput = new QueryInput[1];
        savedQueryInput[0] = new QueryInput();
        savedQueryInput[0].query = query;
        savedQueryInput[0].maxNumToReturn = MAX_NUM_OBJECT_RETURN;
        savedQueryInput[0].limitList = new ModelObject[0];
        savedQueryInput[0].entries = new String[] { QUERY_GROUP_LICENSE_FIELD };
        savedQueryInput[0].values = new String[1];
        savedQueryInput[0].values[0] = groupId;

        SavedQueriesResponse savedQueryResult = queryService.executeSavedQueries(savedQueryInput);
        QueryResults found = savedQueryResult.arrayOfResults[0];

        ServiceData sd = dmService.loadObjects(found.objectUIDS);
        ModelObject[] foundObjects = new ModelObject[sd.sizeOfPlainObjects()];
        String[] licenseList = new String[sd.sizeOfPlainObjects()];

        for (int k = 0; k < sd.sizeOfPlainObjects(); k++) {
            foundObjects[k] = sd.getPlainObject(k);
        }

        logger.debug("Load ID property for ADA_License found");
        dmService.refreshObjects(foundObjects);
        dmService.getProperties(foundObjects, new String[] { "id", "lock_date", "expiry_date", "fnd0license_category", "fnd0user_citizenships" });

        try {
            for (int k = 0; k < foundObjects.length; k++) {
                ADA_License license = (ADA_License) foundObjects[k];

                String licenseInfo = summarizeADALicenseInfo(license);
                logger.info(licenseInfo);
                licenseList[k] = licenseInfo;
            }
            logger.info(() -> "Group " + groupId + "'s related ADA_License: " + Arrays.toString(licenseList));

            return licenseList;
        } catch (NotLoadedException ex) {
            throw new NotLoadedException("Property Id not loaded for all ADA License found");
        }
    }

    private String summarizeADALicenseInfo(ADA_License license) throws NotLoadedException {
        String licenseId = license.get_id();
        String licenseType = license.get_object_type();
        Calendar lockDate = license.get_lock_date();
        Calendar expiryDate = license.get_expiry();
        String category = license.get_fnd0license_category();
        String[] citizenshipList = license.get_fnd0user_citizenships();

        JSONObject licenseObject = new JSONObject();
        // License ID and Type are mandatory fields for ADA License
        licenseObject.put("id", licenseId);
        logger.info("ID : " + licenseId);
        licenseObject.put("type", licenseType);
        logger.info("Type: " + licenseType);

        if (category == null) {
            logger.info("No category specified for this license");
        } else {
            licenseObject.put("fnd0license_category", category);
            logger.info("Category: " + category);
        }

        if (citizenshipList == null) {
            logger.info("No Citizenship List specified for this license");
        } else {
            licenseObject.put("fnd0user_citizenships", Arrays.asList(citizenshipList).toString());
            logger.info("Citizenship List: " + Arrays.asList(citizenshipList));
        }

        DateFormat dateFormat = new SimpleDateFormat("yyyy/MM/dd HH:mm:ss");
        if (lockDate == null) {
            logger.info("No Lock Date specified for this license");
        } else {
            String lockDateVal = dateFormat.format(lockDate.getTime());
            licenseObject.put("lock_date", lockDateVal);
            logger.info("Lock Date: " + lockDateVal);
        }

        if (expiryDate == null) {
            logger.info("No Expiry Date specified for this license");
        } else {
            String expiryDateVal = dateFormat.format(expiryDate.getTime());
            licenseObject.put("expiry_date", expiryDateVal);
            logger.info("Expiry Date: " + expiryDateVal);
        }

        return licenseObject.toString();
    }
}
