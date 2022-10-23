package com.nextlabs.tc.soa;

import com.teamcenter.clientx.AppXSession;
import com.teamcenter.schemas.soa._2006_03.exceptions.InvalidCredentialsException;

public class TeamcenterSOAConnection {
    private final  AppXSession session;

    public TeamcenterSOAConnection(String tcSoaUrl, String tcUserName, String tcPassword) throws InvalidCredentialsException {
        session = new AppXSession(tcSoaUrl);
        session.login(tcUserName, tcPassword);
    }

    public boolean testConnection() {
        return session.isConnectionAlive();
    }
}
