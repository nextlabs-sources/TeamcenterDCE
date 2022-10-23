package com.nextlabs.dap.provider.rmxuser;

import com.nextlabs.dap.provider.ConnectionDetails;
import com.nextlabs.dap.provider.exception.InvalidKeyException;
import com.nextlabs.dap.provider.exception.InvalidUserIdException;
import com.nextlabs.dap.provider.teamcenter.TCDAProvider;
import org.junit.BeforeClass;
import org.junit.Rule;
import org.junit.Test;
import org.junit.internal.TextListener;
import org.junit.rules.ExpectedException;
import org.junit.runner.JUnitCore;

import java.util.Arrays;
import java.util.HashMap;

import static org.junit.Assert.*;

public class QueryUserTest {
    private static final String PROXY_USER_ID = "dapproxy";
    private static final String PROXY_USER_PS = "123blue!";
    private static final String HOST_NAME = "tc06";
    private static final int    PORT_NUMBER = 8080;
    private static TCDAProvider dapProvider;

    @Rule
    public ExpectedException exceptionRule = ExpectedException.none();

    public static void main(String[] args) {
        JUnitCore junit = new JUnitCore();
        junit.addListener(new TextListener(System.out));
        junit.run(QueryUserTest.class);
    }

    @BeforeClass
    public static void beforeClass() throws Exception{
        dapProvider = new TCDAProvider();
        ConnectionDetails connectionDetails = new ConnectionDetails(PROXY_USER_ID, PROXY_USER_PS, HOST_NAME, PORT_NUMBER);
        dapProvider.init(connectionDetails);
    }

    @Test
    public void testDAPConnection() {
        assertTrue(dapProvider.testConnection().isSuccess());
    }

    @Test
    public void testQuerySingleKey1() throws Exception {
        String inputUser = "abraham.lincoln";
        String inputKey = "ip_clearance";

        String[] expectedOutput = new String[] { "secret" };
        String[] actualOutput = dapProvider.getValue(inputKey, inputUser);
        assertArrayEquals(actualOutput, expectedOutput);
    }

    @Test
    public void testQuerySingleKey2() throws Exception {
        String inputUser = "abraham.lincoln";
        String inputKey = "user_projects";

        String expectedProject1 = "AIR1-AirPlain Design 1";
        String expectedProject2 = "AIR2-AirPlain Design 2";
        String[] actualOutput = dapProvider.getValue(inputKey, inputUser);
        assertTrue(Arrays.asList(actualOutput).contains(expectedProject1));
        assertTrue(Arrays.asList(actualOutput).contains(expectedProject2));
    }

    @Test
    public void testQuerySingleKey3() throws Exception {
        String inputUser = "abraham.lincoln";
        String inputKey = "ada_license";

        String expectedLicense1 = "ENG3";
        String expectedLicense2 = "ENG2";
        String[] actualOutput = dapProvider.getValue(inputKey, inputUser);
        assertTrue(Arrays.asList(actualOutput).contains(expectedLicense1));
        assertTrue(Arrays.asList(actualOutput).contains(expectedLicense2));
    }

    @Test
    public void testQueryLicenseKey() throws Exception {
        String inputUser = "jimmy.carter";
        String inputKey = "ada_license";

        String expectedLicense1 = "ENG3";
        String expectedLicense2 = "ENG2";
        String expectedLicense3 = "ENG1";
        String expectedLicense4 = "ENG4";
        String[] actualOutput = dapProvider.getValue(inputKey, inputUser);
        assertTrue(Arrays.asList(actualOutput).contains(expectedLicense1));
        assertTrue(Arrays.asList(actualOutput).contains(expectedLicense2));
        assertTrue(Arrays.asList(actualOutput).contains(expectedLicense3));
        assertTrue(Arrays.asList(actualOutput).contains(expectedLicense4));
    }

    @Test(expected = InvalidKeyException.class)
    public void testQuerySingleInvalidKey() throws Exception {
        String inputUser = "abraham.lincoln";
        String inputKey = "ada_list";

        dapProvider.getValue(inputKey, inputUser);
    }

    @Test(expected = InvalidUserIdException.class)
    public void testQuerySingleKeyInvalidUser() throws Exception {
        String inputUser = "random_user";
        String inputKey = "whatever_property";

        dapProvider.getValue(inputKey, inputUser);
    }

    @Test
    public void testQueryMultipleKeyAllExist() throws Exception {
        String inputUser = "abraham.lincoln";
        String[] inputKeys = new String[] { "gov_clearance", "user_projects", "ada_license"};

        HashMap<String, String[]> actualOutput = dapProvider.getValues(inputKeys, inputUser);

        // All key appear in the output
        assertEquals(actualOutput.keySet().size(), inputKeys.length);
        assertTrue(Arrays.asList(actualOutput.get("gov_clearance")).contains("super-secret"));
        assertTrue(Arrays.asList(actualOutput.get("user_projects")).containsAll(Arrays.asList("AIR1-AirPlain Design 1", "AIR2-AirPlain Design 2")));
        assertTrue(Arrays.asList(actualOutput.get("ada_license")).containsAll(Arrays.asList("ENG2", "ENG3")));
    }

    @Test
    public void testQueryMultipleKeySomeExist() throws Exception {
        String inputUser = "abraham.lincoln";
        String[] inputKeys = new String[] { "ip_clearance", "projects_list", "ada_license"};

        HashMap<String, String[]> actualOutput = dapProvider.getValues(inputKeys, inputUser);

        // Keys length of output and input is different
        assertNotEquals(actualOutput.keySet().size(), inputKeys.length);

        assertTrue(actualOutput.containsKey("ip_clearance"));
        assertTrue(Arrays.asList(actualOutput.get("ip_clearance")).contains("secret"));

        assertTrue(actualOutput.containsKey("ada_license"));
        assertTrue(Arrays.asList(actualOutput.get("ada_license")).containsAll(Arrays.asList("ENG2", "ENG3")));
    }

    @Test
    public void testQueryMultipleKeyNonExist() throws Exception {
        String inputUser = "abraham.lincoln";
        String[] inputKeys = new String[] { "nationality_clearance", "address", "ada_list"};

        HashMap<String, String[]> actualOutput = dapProvider.getValues(inputKeys, inputUser);

        assertNotNull(actualOutput);

        assertEquals(actualOutput.keySet().size(), 0);
    }

    @Test(expected = InvalidUserIdException.class)
    public void testQueryMultipleKeyInvalidUser() throws Exception {
        String inputUser = "donald.trump";
        String[] inputKeys = new String[] { "address", "ada_list", "email"};

        dapProvider.getValues(inputKeys, inputUser);
    }
}
