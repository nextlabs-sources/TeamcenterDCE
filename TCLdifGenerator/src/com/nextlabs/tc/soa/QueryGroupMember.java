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
import com.teamcenter.soa.client.model.strong.GroupMember;
import com.teamcenter.soa.client.model.strong.ImanQuery;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

public class QueryGroupMember {
	private static final Logger logger                      = LogManager.getLogger("TCLDIFLOGGER");

	private QueryGroupMember() {}

	public static GroupMember[] queryGroupMemberList(String groupSearchPattern) throws ServiceException, TCQueryNotFoundException, InvalidPatternException {
		final String QUERY_NAME_GROUP       		= "Admin - Group/Role Membership";
		final String QUERY_NAME_GROUP_FIELD  		= "Group";
		final int    MAX_NUM_OBJECT_RETURN  		= 25;
		final String[] defaultGroupPropertyList 	= new String[] {"object_name","group","display_name","ga"};

		SavedQueryService queryService      = SavedQueryService.getService(AppXSession.getConnection());
		DataManagementService dmService     = DataManagementService.getService(AppXSession.getConnection());

		SavedQuery.FindSavedQueriesCriteriaInput[] queryCriteriaInputs = new SavedQuery.FindSavedQueriesCriteriaInput[1];
		queryCriteriaInputs[0] = new SavedQuery.FindSavedQueriesCriteriaInput();
		queryCriteriaInputs[0].queryNames = new String[] { QUERY_NAME_GROUP };

		SavedQuery.FindSavedQueriesResponse queryResponse = queryService.findSavedQueries(queryCriteriaInputs);
		if (queryResponse.savedQueries.length == 0) {
			throw new TCQueryNotFoundException(QUERY_NAME_GROUP);
		}

		ImanQuery query = queryResponse.savedQueries[0];
		logger.debug(() ->"Found " + QUERY_NAME_GROUP + " type in QueryBuilder list. Executing a query to find user list");

		QueryInput[] savedQueryInput = new QueryInput[1];
		savedQueryInput[0] = new QueryInput();
		savedQueryInput[0].query = query;
		savedQueryInput[0].maxNumToReturn = MAX_NUM_OBJECT_RETURN;
		savedQueryInput[0].limitList = new ModelObject[0];
		savedQueryInput[0].entries = new String[] { QUERY_NAME_GROUP_FIELD };
		savedQueryInput[0].values = new String[1];
		savedQueryInput[0].values[0] = groupSearchPattern;

		SavedQueriesResponse savedQueryResult = queryService.executeSavedQueries(savedQueryInput);
		QueryResults found = savedQueryResult.arrayOfResults[0];

		if (found.objectUIDS.length == 0) {
			throw new InvalidPatternException(groupSearchPattern);
		}

		logger.debug("Found list of Group object matched with given search pattern");
		logger.info(() -> "Found " + found.objectUIDS.length + " group-member relationships.");
		ServiceData sd = dmService.loadObjects(found.objectUIDS);

		GroupMember[] groupMemberList = new GroupMember[found.objectUIDS.length];

		for (int i = 0; i < found.objectUIDS.length; i += 1) {
			GroupMember groupMember = (GroupMember) sd.getPlainObject(i);
			dmService.refreshObjects(new ModelObject[] { groupMember });
			dmService.getProperties(new ModelObject[] { groupMember }, defaultGroupPropertyList);
			groupMemberList[i] = (GroupMember) sd.getPlainObject(i);
		}

		return groupMemberList;
	}
}
