package com.nextlabs.tc.exception;

public class InvalidPatternException extends Exception {
    public InvalidPatternException(String userSearchPattern) {
        super("User/Group Pattern " + userSearchPattern + " provided is invalid. Unable to find any results in Teamcenter system.");
    }
}
