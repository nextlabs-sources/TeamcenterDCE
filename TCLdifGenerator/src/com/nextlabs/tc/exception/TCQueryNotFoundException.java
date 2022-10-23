package com.nextlabs.tc.exception;

public class TCQueryNotFoundException extends Exception {
    public TCQueryNotFoundException(String queryName) {
        super("Unable to find related query. " + queryName + " query must be in QueryBuilder list.");
    }
}
