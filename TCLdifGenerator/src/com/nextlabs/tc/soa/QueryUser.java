package com.nextlabs.tc.soa;

import com.nextlabs.tc.exception.InvalidPatternException;
import com.nextlabs.tc.exception.TCQueryNotFoundException;
import com.teamcenter.clientx.AppXSession;
import com.teamcenter.schemas.soa._2006_03.exceptions.ServiceException;
import com.teamcenter.services.strong.core.DataManagementService;
import com.teamcenter.services.strong.query.SavedQueryService;
import com.teamcenter.services.strong.query._2007_09.SavedQuery.QueryResults;
import com.teamcenter.services.strong.query._2007_09.SavedQuery.SavedQueriesResponse;
import com.teamcenter.services.strong.query._2008_06.SavedQuery.QueryInput;
import com.teamcenter.services.strong.query._2010_04.SavedQuery;
import com.teamcenter.soa.client.model.ModelObject;
import com.teamcenter.soa.client.model.ServiceData;
import com.teamcenter.soa.client.model.strong.ImanQuery;
import com.teamcenter.soa.client.model.strong.User;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class QueryUser {
	private static final Logger logger                      = LogManager.getLogger("TCLDIFLOGGER");

	private QueryUser () {}

	public static User[] queryUserList(String userSearchPattern, String[] propertyList) throws ServiceException, TCQueryNotFoundException, InvalidPatternException {
		final String QUERY_NAME_USER        = "Admin - Employee Information";
		final String QUERY_NAME_USER_FIELD  = "Person Name";
		final int    MAX_NUM_OBJECT_RETURN  = 25;

		SavedQueryService queryService      = SavedQueryService.getService(AppXSession.getConnection());
		DataManagementService dmService     = DataManagementService.getService(AppXSession.getConnection());

		SavedQuery.FindSavedQueriesCriteriaInput[] queryCriteriaInputs = new SavedQuery.FindSavedQueriesCriteriaInput[1];
		queryCriteriaInputs[0] = new SavedQuery.FindSavedQueriesCriteriaInput();
		queryCriteriaInputs[0].queryNames = new String[] { QUERY_NAME_USER };

		SavedQuery.FindSavedQueriesResponse queryResponse = queryService.findSavedQueries(queryCriteriaInputs);
		if (queryResponse.savedQueries.length == 0) {
			throw new TCQueryNotFoundException(QUERY_NAME_USER);
		}

		ImanQuery query = queryResponse.savedQueries[0];
		logger.debug(() ->"Found " + QUERY_NAME_USER + " type in QueryBuilder list. Executing a query to find user list");

		QueryInput[] savedQueryInput = new QueryInput[1];
		savedQueryInput[0] = new QueryInput();
		savedQueryInput[0].query = query;
		savedQueryInput[0].maxNumToReturn = MAX_NUM_OBJECT_RETURN;
		savedQueryInput[0].limitList = new ModelObject[0];
		savedQueryInput[0].entries = new String[] { QUERY_NAME_USER_FIELD };
		savedQueryInput[0].values = new String[1];
		savedQueryInput[0].values[0] = userSearchPattern;

		SavedQueriesResponse savedQueryResult = queryService.executeSavedQueries(savedQueryInput);
		QueryResults found = savedQueryResult.arrayOfResults[0];

		if (found.objectUIDS.length == 0) {
			throw new InvalidPatternException(userSearchPattern);
		}

		logger.debug("Found list of User object matched with given search pattern");
		logger.info(() -> "Found " + found.objectUIDS.length + " users.");
		ServiceData sd = dmService.loadObjects(found.objectUIDS);

		User[] userList = new User[found.objectUIDS.length];

		for (int i = 0; i < found.objectUIDS.length; i += 1) {
			User user = (User) sd.getPlainObject(i);
			dmService.refreshObjects(new ModelObject[] { user });
			dmService.getProperties(new ModelObject[] { user }, propertyList);
			userList[i] = (User) sd.getPlainObject(i);
		}

		return userList;
	}
}
